/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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
#define HST_LOG_TAG "HlsPlayListDownloader"
#include <mutex>
#include "plugin/plugin_time.h"
#include "hls_playlist_downloader.h"
#include <unistd.h>

namespace OHOS {
namespace Media {
namespace Plugins {
namespace HttpPlugin {
void HlsPlayListDownloader::PlayListUpdateLoop()
{
    OSAL::SleepFor(5000); // 5000 how often is playlist updated
    UpdateManifest();
}

// StateMachine thread: call plugin SetSource -> call Open
// StateMachine thread: call plugin GetSeekable -> call GetSeekable
// PlayListDownload thread: call ParseManifest
// First call Open, then start PlayListDownload thread, it seems no lock is required.
// [In future] StateMachine thread: call plugin GetDuration -> call GetDuration
void HlsPlayListDownloader::Open(const std::string& url)
{
    url_ = url;
    master_ = nullptr;
    DoOpen(url);
}

void HlsPlayListDownloader::UpdateManifest()
{
    if (currentVariant_ && currentVariant_->m3u8_ && !currentVariant_->m3u8_->uri_.empty()) {
        DoOpen(currentVariant_->m3u8_->uri_);
    } else {
        MEDIA_LOG_E("UpdateManifest currentVariant_ not ready.");
    }
}

void HlsPlayListDownloader::SetPlayListCallback(PlayListChangeCallback* callback)
{
    callback_ = callback;
}

int64_t HlsPlayListDownloader::GetDuration() const
{
    if (!master_) {
        return 0;
    }
    return master_->bLive_ ? -1 : ((int64_t)(master_->duration_ * HST_SECOND) / HST_NSECOND); // -1
}

Seekable HlsPlayListDownloader::GetSeekable() const
{
    // need wait master_ not null
    while (true) {
        if (master_ && master_->isSimple_) {
            break;
        }
        OSAL::SleepFor(1);
    }
    if (master_->bLive_) {
        updateTask_->Start();
    }
    return master_->bLive_ ? Seekable::UNSEEKABLE : Seekable::SEEKABLE;
}

void HlsPlayListDownloader::NotifyListChange()
{
    auto files = currentVariant_->m3u8_->files_;
    auto playList = std::vector<PlayInfo>();
    if (currentVariant_->m3u8_->isDecryptAble_) {
        while (!currentVariant_->m3u8_->isDecryptKeyReady_) {
            OSAL::SleepFor(10); // 10
        }
        callback_->OnSourceKeyChange(currentVariant_->m3u8_->key_, currentVariant_->m3u8_->keyLen_,
            currentVariant_->m3u8_->iv_);
    } else {
        MEDIA_LOG_E("Decrypkey is not needed.");
        callback_->OnSourceKeyChange(nullptr, 0, nullptr);
    }
    playList.reserve(files.size());
    for (const auto &file: files) {
        PlayInfo palyInfo;
        palyInfo.url_ = file->uri_;
        palyInfo.duration_ = file->duration_;
        playList.push_back(palyInfo);
    }
    if (!currentVariant_->m3u8_->localDrmInfos_.empty()) {
        callback_->OnDrmInfoChanged(currentVariant_->m3u8_->localDrmInfos_);
    }
    callback_->OnPlayListChanged(playList);
}

void HlsPlayListDownloader::ParseManifest()
{
    if (!master_) {
        master_ = std::make_shared<M3U8MasterPlaylist>(playList_, url_);
        currentVariant_ = master_->defaultVariant_;
        if (!master_->isSimple_) {
            UpdateManifest();
        } else {
            // need notify , avoid delay 5s
            NotifyListChange();
        }
    } else {
        if (master_->isSimple_) {
            bool ret = currentVariant_->m3u8_->Update(playList_);
            if (ret) {
                NotifyListChange();
            }
        } else {
            currentVariant_ = master_->defaultVariant_;
            bool ret = currentVariant_->m3u8_->Update(playList_);
            if (ret) {
                master_->isSimple_ = true;
                master_->duration_ = currentVariant_->m3u8_->GetDuration();
                NotifyListChange();
            }
        }
    }
}

void HlsPlayListDownloader::SelectBitRate(uint32_t bitRate)
{
    currentVariant_ = newVariant_;
    MEDIA_LOG_I("SelectBitRate currentVariant_ " PUBLIC_LOG_U64, currentVariant_->bandWidth_);
}

bool HlsPlayListDownloader::IsBitrateSame(uint32_t bitRate)
{
    uint32_t maxGap = 0;
    bool isFirstSelect = true;
    for (const auto &item : master_->variants_) {
        uint32_t tempGap = (item->bandWidth_ > bitRate) ? (item->bandWidth_ - bitRate) : (bitRate - item->bandWidth_);
        if (isFirstSelect || (tempGap < maxGap)) {
            isFirstSelect = false;
            maxGap = tempGap;
            newVariant_ = item;
        }
    }
    if (newVariant_->bandWidth_ == currentVariant_->bandWidth_) {
        return true;
    }
    return false;
}

std::vector<uint32_t> HlsPlayListDownloader::GetBitRates()
{
    std::vector<uint32_t> bitRates;
    for (const auto &item : master_->variants_) {
        if (item->bandWidth_) {
            bitRates.push_back(item->bandWidth_);
        }
    }
    return bitRates;
}

bool HlsPlayListDownloader::IsLive() const
{
    MEDIA_LOG_I("HlsPlayListDownloader IsLive enter.");
    if (!master_) {
        return false;
    }
    return master_->bLive_;
}
}
}
}
}