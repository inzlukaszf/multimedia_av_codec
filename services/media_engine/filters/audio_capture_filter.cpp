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

#include "audio_capture_filter.h"
#include "common/log.h"
#include "filter/filter_factory.h"
#include "source/audio_capture/audio_capture_module.h"

namespace OHOS {
namespace Media {
namespace Pipeline {
constexpr uint32_t TIME_OUT_MS = 0;
static AutoRegisterFilter<AudioCaptureFilter> g_registerAudioCaptureFilter("builtin.recorder.audiocapture",
    FilterType::AUDIO_CAPTURE,
    [](const std::string& name, const FilterType type) {
        return std::make_shared<AudioCaptureFilter>(name, FilterType::AUDIO_CAPTURE);
    });

/// End of Stream Buffer Flag
constexpr uint32_t BUFFER_FLAG_EOS = 0x00000001;
class AudioCaptureFilterLinkCallback : public FilterLinkCallback {
public:
    explicit AudioCaptureFilterLinkCallback(std::shared_ptr<AudioCaptureFilter> audioCaptureFilter)
        : audioCaptureFilter_(std::move(audioCaptureFilter))
    {
    }

    void OnLinkedResult(const sptr<AVBufferQueueProducer> &queue, std::shared_ptr<Meta> &meta) override
    {
        if (auto captureFilter = audioCaptureFilter_.lock()) {
            captureFilter->OnLinkedResult(queue, meta);
        } else {
            MEDIA_LOG_I("invalid captureFilter");
        }
    }

    void OnUnlinkedResult(std::shared_ptr<Meta> &meta) override
    {
        if (auto captureFilter = audioCaptureFilter_.lock()) {
            captureFilter->OnUnlinkedResult(meta);
        } else {
            MEDIA_LOG_I("invalid captureFilter");
        }
    }

    void OnUpdatedResult(std::shared_ptr<Meta> &meta) override
    {
        if (auto captureFilter = audioCaptureFilter_.lock()) {
            captureFilter->OnUpdatedResult(meta);
        } else {
            MEDIA_LOG_I("invalid captureFilter");
        }
    }

private:
    std::weak_ptr<AudioCaptureFilter> audioCaptureFilter_;
};

class AudioCaptureModuleCallbackImpl : public AudioCaptureModule::AudioCaptureModuleCallback {
public:
    explicit AudioCaptureModuleCallbackImpl(std::shared_ptr<EventReceiver> receiver)
        : receiver_(receiver)
    {
    }
    void OnInterrupt(const std::string &interruptInfo) override
    {
        MEDIA_LOG_I("AudioCaptureModuleCallback interrupt: " PUBLIC_LOG_S, interruptInfo.c_str());
        receiver_->OnEvent({"audio_capture_filter", EventType::EVENT_ERROR, Status::ERROR_AUDIO_INTERRUPT});
    }
private:
    std::shared_ptr<EventReceiver> receiver_;
};

AudioCaptureFilter::AudioCaptureFilter(std::string name, FilterType type): Filter(name, type)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "audio capture filter create", logTag_.c_str());
}

AudioCaptureFilter::~AudioCaptureFilter()
{
    MEDIA_LOG_I(PUBLIC_LOG_S "audio capture filter destroy", logTag_.c_str());
}

void AudioCaptureFilter::Init(const std::shared_ptr<EventReceiver> &receiver,
    const std::shared_ptr<FilterCallback> &callback)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "Init", logTag_.c_str());
    receiver_ = receiver;
    callback_ = callback;
    audioCaptureModule_ = std::make_shared<AudioCaptureModule::AudioCaptureModule>();
    audioCaptureModule_->SetLogTag(logTag_);
    std::shared_ptr<AudioCaptureModule::AudioCaptureModuleCallback> cb =
        std::make_shared<AudioCaptureModuleCallbackImpl>(receiver_);
    Status cbError = audioCaptureModule_->SetAudioInterruptListener(cb);
    if (cbError != Status::OK) {
        MEDIA_LOG_E(PUBLIC_LOG_S "audioCaptureModule_ SetAudioInterruptListener failed.", logTag_.c_str());
    }
    if (audioCaptureModule_) {
        audioCaptureModule_->SetAudioSource(sourceType_);
        audioCaptureModule_->SetParameter(audioCaptureConfig_);
    }
    Status err = audioCaptureModule_->Init();
    if (err != Status::OK) {
        MEDIA_LOG_E(PUBLIC_LOG_S "Init audioCaptureModule fail", logTag_.c_str());
    } else {
        state_ = FilterState::INITIALIZED;
    }
}

void AudioCaptureFilter::SetLogTag(std::string logTag)
{
    logTag_ = std::move(logTag);
}

Status AudioCaptureFilter::PrepareAudioCapture()
{
    MEDIA_LOG_I(PUBLIC_LOG_S "PrepareAudioCapture", logTag_.c_str());
    FALSE_RETURN_V_MSG_W(state_ == FilterState::INITIALIZED, Status::ERROR_INVALID_OPERATION,
        PUBLIC_LOG_S "filter is not in init state", logTag_.c_str());
    if (!taskPtr_) {
        taskPtr_ = std::make_shared<Task>("DataReader");
        taskPtr_->RegisterJob([this] { ReadLoop(); });
    }

    Status err = audioCaptureModule_->Prepare();
    if (err != Status::OK) {
        MEDIA_LOG_E(PUBLIC_LOG_S "audioCaptureModule prepare fail", logTag_.c_str());
    } else {
        state_ = FilterState::PREPARING;
    }
    return err;
}

Status AudioCaptureFilter::SetAudioCaptureChangeCallback(
    const std::shared_ptr<AudioStandard::AudioCapturerInfoChangeCallback> &callback)
{
    if (audioCaptureModule_ == nullptr) {
        return Status::ERROR_WRONG_STATE;
    }
    return audioCaptureModule_->SetAudioCapturerInfoChangeCallback(callback);
}

Status AudioCaptureFilter::Prepare()
{
    MEDIA_LOG_I(PUBLIC_LOG_S "Prepare", logTag_.c_str());
    if (callback_ == nullptr) {
        MEDIA_LOG_E(PUBLIC_LOG_S "callback is nullptr", logTag_.c_str());
        return Status::ERROR_NULL_POINTER;
    }
    callback_->OnCallback(shared_from_this(), FilterCallBackCommand::NEXT_FILTER_NEEDED,
        StreamType::STREAMTYPE_RAW_AUDIO);
    return Status::OK;
}

Status AudioCaptureFilter::Start()
{
    MEDIA_LOG_I(PUBLIC_LOG_S "Start", logTag_.c_str());
    nextFilter_->Start();
    eos_ = false;
    auto res = Status::ERROR_INVALID_OPERATION;
    // start audioCaptureModule firstly
    if (audioCaptureModule_) {
        res = audioCaptureModule_->Start();
    }
    FALSE_RETURN_V_MSG_E(res == Status::OK, res, "start audioCaptureModule failed");
    // start task secondly
    if (taskPtr_) {
        taskPtr_->Start();
    }
    state_ = FilterState::RUNNING;
    return res;
}

Status AudioCaptureFilter::Pause()
{
    MEDIA_LOG_I(PUBLIC_LOG_S "Pause", logTag_.c_str());
    if (taskPtr_) {
        taskPtr_->Pause();
    }
    Status ret = Status::ERROR_INVALID_OPERATION;
    if (audioCaptureModule_) {
        ret = audioCaptureModule_->Stop();
    }
    if (ret == Status::OK) {
        state_ = FilterState::PAUSED;
    } else {
        MEDIA_LOG_I(PUBLIC_LOG_S "audioCaptureModule stop fail", logTag_.c_str());
    }
    return ret;
}

Status AudioCaptureFilter::Resume()
{
    MEDIA_LOG_I(PUBLIC_LOG_S "Resume", logTag_.c_str());
    if (taskPtr_) {
        taskPtr_->Start();
    }
    Status ret = Status::ERROR_INVALID_OPERATION;
    if (audioCaptureModule_) {
        ret = audioCaptureModule_->Start();
    }
    if (ret == Status::OK) {
        state_ = FilterState::RUNNING;
    } else {
        MEDIA_LOG_E(PUBLIC_LOG_S "audioCaptureModule start fail", logTag_.c_str());
    }
    return ret;
}

Status AudioCaptureFilter::Stop()
{
    MEDIA_LOG_I(PUBLIC_LOG_S "Stop", logTag_.c_str());
    // stop task firstly
    if (taskPtr_) {
        taskPtr_->StopAsync();
    }
    // stop audioCaptureModule secondly
    Status ret = Status::ERROR_INVALID_OPERATION;
    if (audioCaptureModule_) {
        ret = audioCaptureModule_->Stop();
    }
    if (ret == Status::OK) {
        state_ = FilterState::INITIALIZED;
    } else {
        MEDIA_LOG_E(PUBLIC_LOG_S "audioCaptureModule stop fail", logTag_.c_str());
    }
    if (nextFilter_) {
        nextFilter_->Stop();
    }
    return ret;
}

Status AudioCaptureFilter::Flush()
{
    MEDIA_LOG_I(PUBLIC_LOG_S "Flush", logTag_.c_str());
    return Status::OK;
}

Status AudioCaptureFilter::Release()
{
    MEDIA_LOG_I(PUBLIC_LOG_S "Release", logTag_.c_str());
    if (taskPtr_) {
        taskPtr_->Stop();
    }
    if (audioCaptureModule_) {
        audioCaptureModule_->Deinit();
    }
    audioCaptureModule_ = nullptr;
    taskPtr_ = nullptr;
    return Status::OK;
}

void AudioCaptureFilter::SetParameter(const std::shared_ptr<Meta> &meta)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "SetParameter", logTag_.c_str());
    audioCaptureConfig_ = meta;
}

void AudioCaptureFilter::GetParameter(std::shared_ptr<Meta> &meta)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "GetParameter", logTag_.c_str());
    audioCaptureModule_->GetParameter(meta);
}

Status AudioCaptureFilter::LinkNext(const std::shared_ptr<Filter> &nextFilter, StreamType outType)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "LinkNext", logTag_.c_str());
    auto meta = std::make_shared<Meta>();
    GetParameter(meta);
    nextFilter_ = nextFilter;
    std::shared_ptr<FilterLinkCallback> filterLinkCallback =
        std::make_shared<AudioCaptureFilterLinkCallback>(shared_from_this());
    nextFilter->OnLinked(outType, meta, filterLinkCallback);
    nextFilter->Prepare();
    return Status::OK;
}

FilterType AudioCaptureFilter::GetFilterType()
{
    MEDIA_LOG_I(PUBLIC_LOG_S "GetFilterType", logTag_.c_str());
    return FilterType::AUDIO_CAPTURE;
}

void AudioCaptureFilter::SetAudioSource(int32_t source)
{
    if (source == 1) {
        sourceType_ = AudioStandard::SourceType::SOURCE_TYPE_MIC;
    } else {
        sourceType_ = static_cast<AudioStandard::SourceType>(source);
    }
}

Status AudioCaptureFilter::SendEos()
{
    MEDIA_LOG_I(PUBLIC_LOG_S "SendEos", logTag_.c_str());
    Status ret = Status::OK;
    if (outputBufferQueue_) {
        std::shared_ptr<AVBuffer> buffer;
        AVBufferConfig avBufferConfig;
        ret = outputBufferQueue_->RequestBuffer(buffer, avBufferConfig, TIME_OUT_MS);
        if (ret != Status::OK) {
            MEDIA_LOG_I("RequestBuffer fail, ret" PUBLIC_LOG_D32, ret);
            return ret;
        }
        buffer->flag_ |= BUFFER_FLAG_EOS;
        outputBufferQueue_->PushBuffer(buffer, false);
    }
    eos_ = true;
    return ret;
}

void AudioCaptureFilter::ReadLoop()
{
    MEDIA_LOG_I(PUBLIC_LOG_S "ReadLoop", logTag_.c_str());
    if (eos_.load()) {
        return;
    }
    uint64_t bufferSize = 0;
    auto ret = audioCaptureModule_->GetSize(bufferSize);
    if (ret != Status::OK) {
        MEDIA_LOG_E(PUBLIC_LOG_S "Get audioCaptureModule buffer size fail", logTag_.c_str());
        return;
    }
    std::shared_ptr<AVBuffer> buffer;
    AVBufferConfig avBufferConfig;
    avBufferConfig.size = bufferSize;
    avBufferConfig.memoryFlag = MemoryFlag::MEMORY_READ_WRITE;
    ret = outputBufferQueue_->RequestBuffer(buffer, avBufferConfig, TIME_OUT_MS);
    if (ret != Status::OK) {
        MEDIA_LOG_E(PUBLIC_LOG_S "RequestBuffer fail", logTag_.c_str());
        return;
    }
    ret = audioCaptureModule_->Read(buffer, bufferSize);
    if (ret == Status::ERROR_AGAIN) {
        MEDIA_LOG_E(PUBLIC_LOG_S "audioCaptureModule read return again", logTag_.c_str());
        outputBufferQueue_->PushBuffer(buffer, false);
        return;
    }
    if (ret != Status::OK) {
        MEDIA_LOG_E(PUBLIC_LOG_S "RequestBuffer fail", logTag_.c_str());
        outputBufferQueue_->PushBuffer(buffer, false);
        return;
    }
    buffer->memory_->SetSize(bufferSize);

    Status status = outputBufferQueue_->PushBuffer(buffer, true);
    if (status != Status::OK) {
        MEDIA_LOG_E(PUBLIC_LOG_S "PushBuffer fail", logTag_.c_str());
    }
}

Status AudioCaptureFilter::GetCurrentCapturerChangeInfo(AudioStandard::AudioCapturerChangeInfo &changeInfo)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "GetCurrentCapturerChangeInfo", logTag_.c_str());
    if (audioCaptureModule_ == nullptr) {
        MEDIA_LOG_E(PUBLIC_LOG_S "audioCaptureModule_ is nullptr, cannot get audio capturer change info",
            logTag_.c_str());
        return Status::ERROR_INVALID_OPERATION;
    }
    audioCaptureModule_->GetCurrentCapturerChangeInfo(changeInfo);
    return Status::OK;
}

int32_t AudioCaptureFilter::GetMaxAmplitude()
{
    MEDIA_LOG_I(PUBLIC_LOG_S "GetMaxAmplitude", logTag_.c_str());
    if (audioCaptureModule_ == nullptr) {
        MEDIA_LOG_E(PUBLIC_LOG_S "audioCaptureModule_ is nullptr, cannot get audio capturer change info ",
            logTag_.c_str());
        return (int32_t)Status::ERROR_INVALID_OPERATION;
    }
    return audioCaptureModule_->GetMaxAmplitude();
}

void AudioCaptureFilter::OnLinkedResult(const sptr<AVBufferQueueProducer> &queue, std::shared_ptr<Meta> &meta)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "OnLinkedResult", logTag_.c_str());
    outputBufferQueue_ = queue;
    PrepareAudioCapture();
}

Status AudioCaptureFilter::UpdateNext(const std::shared_ptr<Filter> &nextFilter, StreamType outType)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "UpdateNext", logTag_.c_str());
    return Status::OK;
}

Status AudioCaptureFilter::UnLinkNext(const std::shared_ptr<Filter> &nextFilter, StreamType outType)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "UnLinkNext", logTag_.c_str());
    return Status::OK;
}

Status AudioCaptureFilter::OnLinked(StreamType inType, const std::shared_ptr<Meta> &meta,
    const std::shared_ptr<FilterLinkCallback> &callback)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "OnLinked", logTag_.c_str());
    return Status::OK;
}

Status AudioCaptureFilter::OnUpdated(StreamType inType, const std::shared_ptr<Meta> &meta,
    const std::shared_ptr<FilterLinkCallback> &callback)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "OnUpdated", logTag_.c_str());
    return Status::OK;
}

Status AudioCaptureFilter::OnUnLinked(StreamType inType, const std::shared_ptr<FilterLinkCallback> &callback)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "OnUnLinked", logTag_.c_str());
    return Status::OK;
}

void AudioCaptureFilter::OnUnlinkedResult(const std::shared_ptr<Meta> &meta)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "OnUnlinkedResult", logTag_.c_str());
    (void) meta;
}

void AudioCaptureFilter::OnUpdatedResult(const std::shared_ptr<Meta> &meta)
{
    MEDIA_LOG_I(PUBLIC_LOG_S "OnUpdatedResult", logTag_.c_str());
    (void) meta;
}

} // namespace Pipeline
} // namespace Media
} // namespace OHOS