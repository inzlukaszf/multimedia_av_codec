/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define HST_LOG_TAG "HttpMediaDownloader"

#include "http/http_media_downloader.h"

namespace OHOS {
namespace Media {
namespace Plugins {
namespace HttpPlugin {
namespace {
#ifdef OHOS_LITE
constexpr int RING_BUFFER_SIZE = 5 * 48 * 1024;
constexpr int WATER_LINE = RING_BUFFER_SIZE / 30; // 30 WATER_LINE:8192
#else
constexpr int RING_BUFFER_SIZE = 5 * 1024 * 1024;
constexpr int WATER_LINE = 8192; //  WATER_LINE:8192
#endif
}

HttpMediaDownloader::HttpMediaDownloader() noexcept
{
    buffer_ = std::make_shared<RingBuffer>(RING_BUFFER_SIZE);
    buffer_->Init();

    downloader_ = std::make_shared<Downloader>("http");
}

HttpMediaDownloader::~HttpMediaDownloader()
{
    MEDIA_LOG_I("~HttpMediaDownloader dtor");
    Close(false);
}

bool HttpMediaDownloader::Open(const std::string& url)
{
    MEDIA_LOG_I("Open download " PUBLIC_LOG_S, url.c_str());
    auto saveData =  [this] (uint8_t*&& data, uint32_t&& len) {
        return SaveData(std::forward<decltype(data)>(data), std::forward<decltype(len)>(len));
    };
    FALSE_RETURN_V(statusCallback_ != nullptr, false);
    auto realStatusCallback = [this] (DownloadStatus&& status, std::shared_ptr<Downloader>& downloader,
                                  std::shared_ptr<DownloadRequest>& request) {
        statusCallback_(status, downloader_, std::forward<decltype(request)>(request));
    };
    downloadRequest_ = std::make_shared<DownloadRequest>(url, saveData, realStatusCallback);
    downloader_->Download(downloadRequest_, -1); // -1
    buffer_->SetMediaOffset(0);
    downloader_->Start();
    return true;
}

void HttpMediaDownloader::Close(bool isAsync)
{
    buffer_->SetActive(false);
    downloader_->Stop(isAsync);
    cvReadWrite_.NotifyOne();
}

void HttpMediaDownloader::Pause()
{
    bool cleanData = GetSeekable() != Seekable::SEEKABLE;
    buffer_->SetActive(false, cleanData);
    downloader_->Pause();
    cvReadWrite_.NotifyOne();
}

void HttpMediaDownloader::Resume()
{
    buffer_->SetActive(true);
    downloader_->Resume();
    cvReadWrite_.NotifyOne();
}

bool HttpMediaDownloader::Read(unsigned char* buff, unsigned int wantReadLength,
                               unsigned int& realReadLength, bool& isEos)
{
    FALSE_RETURN_V(buffer_ != nullptr, false);
    isEos = false;
    while (buffer_->GetSize() == 0) {
        isEos = downloadRequest_->IsEos();
        bool isClosed = downloadRequest_->IsClosed();
        if (isEos || isClosed) {
            MEDIA_LOG_D("HttpMediaDownloader read return, isEos: " PUBLIC_LOG_D32 ", isClosed: "
                PUBLIC_LOG_D32, isEos, isClosed);
            realReadLength = 0;
            return false;
        }
        OSAL::SleepFor(5); // 5
    }
    if (buffer_->GetMediaOffset() + wantReadLength <= downloadRequest_->GetFileContentLength() &&
        (buffer_->GetSize() < wantReadLength)) {
        MEDIA_LOG_D("wantReadLength larger than available, wait for write");
        AutoLock lock(mutex_);
        cvReadWrite_.Wait(lock, [this, wantReadLength] {
            return buffer_->GetSize() >= wantReadLength || downloadRequest_->IsEos() || downloadRequest_->IsClosed();
        });
    }
    realReadLength = buffer_->ReadBuffer(buff, wantReadLength, 2); // wait 2 times
    MEDIA_LOG_D("Read: wantReadLength " PUBLIC_LOG_D32 ", realReadLength " PUBLIC_LOG_D32 ", isEos "
                PUBLIC_LOG_D32, wantReadLength, realReadLength, isEos);
    return true;
}

bool HttpMediaDownloader::SeekToPos(int64_t offset)
{
    FALSE_RETURN_V(buffer_ != nullptr, false);
    MEDIA_LOG_I("Seek: buffer size " PUBLIC_LOG_ZU ", offset " PUBLIC_LOG_D64, buffer_->GetSize(), offset);
    if (buffer_->Seek(offset)) {
        MEDIA_LOG_I("buffer_ seek success.");
        return true;
    }
    buffer_->SetActive(false); // First clear buffer, avoid no available buffer then task pause never exit.
    downloader_->Pause();
    buffer_->Clear();
    buffer_->SetActive(true);
    bool result = downloader_->Seek(offset);
    if (result) {
        buffer_->SetMediaOffset(offset);
    }
    downloader_->Resume();
    return result;
}

size_t HttpMediaDownloader::GetContentLength() const
{
    if (downloadRequest_->IsClosed()) {
        return 0; // 0
    }
    return downloadRequest_->GetFileContentLength();
}

int64_t HttpMediaDownloader::GetDuration() const
{
    return 0;
}

Seekable HttpMediaDownloader::GetSeekable() const
{
    return downloadRequest_->IsChunked() ? Seekable::UNSEEKABLE : Seekable::SEEKABLE;
}

void HttpMediaDownloader::SetCallback(Callback* cb)
{
    callback_ = cb;
}

void HttpMediaDownloader::SetStatusCallback(StatusCallbackFunc cb)
{
    statusCallback_ = cb;
}

bool HttpMediaDownloader::GetStartedStatus()
{
    return startedPlayStatus_;
}

void HttpMediaDownloader::SetReadBlockingFlag(bool isReadBlockingAllowed)
{
    FALSE_RETURN(buffer_ != nullptr);
    buffer_->SetReadBlocking(isReadBlockingAllowed);
}

bool HttpMediaDownloader::SaveData(uint8_t* data, uint32_t len)
{
    FALSE_RETURN_V(buffer_->WriteBuffer(data, len), false);
    cvReadWrite_.NotifyOne();
    size_t bufferSize = buffer_->GetSize();
    double ratio = (static_cast<double>(bufferSize)) / RING_BUFFER_SIZE;
    if ((bufferSize >= WATER_LINE ||
        bufferSize >= downloadRequest_->GetFileContentLength() / 2) && !aboveWaterline_) { // 2
        aboveWaterline_ = true;
        MEDIA_LOG_I("Send http aboveWaterline event, ringbuffer ratio " PUBLIC_LOG_F, ratio);
        if (callback_ != nullptr) {
            callback_->OnEvent({PluginEventType::ABOVE_LOW_WATERLINE, {ratio}, "http"});
        }
        startedPlayStatus_ = true;
    } else if (bufferSize < WATER_LINE && aboveWaterline_) {
        aboveWaterline_ = false;
        MEDIA_LOG_I("Send http belowWaterline event, ringbuffer ratio " PUBLIC_LOG_F, ratio);
        if (callback_ != nullptr) {
            callback_->OnEvent({PluginEventType::BELOW_LOW_WATERLINE, {ratio}, "http"});
        }
    }
    return true;
}
}
}
}
}