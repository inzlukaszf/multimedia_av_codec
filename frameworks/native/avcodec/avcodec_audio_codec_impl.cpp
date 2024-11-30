/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "avcodec_audio_codec_impl.h"
#include "i_avcodec_service.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "avcodec_trace.h"
#include "avcodec_codec_name.h"
#include "codec_server.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecAudioCodecImpl"};
constexpr int32_t DEFAULT_BUFFER_NUM = 4;
constexpr const char *INPUT_BUFFER_QUEUE_NAME = "AVCodecAudioCodecImpl";
const std::string_view ASYNC_HANDLE_INPUT = "OS_ACodecIn";
const std::string_view ASYNC_OUTPUT_FRAME = "OS_ACodecOut";
constexpr uint8_t LOGD_FREQUENCY = 5;
constexpr uint8_t TIME_OUT_MS = 5;
constexpr uint32_t DEFAULT_TRY_DECODE_TIME = 1;
constexpr int64_t MILLISECONDS = 100;
} // namespace

namespace OHOS {
namespace MediaAVCodec {

AudioCodecConsumerListener::AudioCodecConsumerListener(AVCodecAudioCodecImpl *impl)
{
    impl_ = impl;
}

void AudioCodecConsumerListener::OnBufferAvailable()
{
    impl_->Notify();
}

int32_t AVCodecAudioCodecImpl::Init(AVCodecType type, bool isMimeType, const std::string &name)
{
    AVCODEC_SYNC_TRACE;
    codecService_ = CodecServer::Create();
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_UNKNOWN, "failed to create codec service");

    implBufferQueue_ = Media::AVBufferQueue::Create(DEFAULT_BUFFER_NUM, Media::MemoryType::SHARED_MEMORY,
        INPUT_BUFFER_QUEUE_NAME);

    inputTask_ = std::make_unique<TaskThread>(ASYNC_HANDLE_INPUT);
    outputTask_ = std::make_unique<TaskThread>(ASYNC_OUTPUT_FRAME);

    return codecService_->Init(type, isMimeType, name, API_VERSION::API_VERSION_11);
}

AVCodecAudioCodecImpl::AVCodecAudioCodecImpl()
{
    AVCODEC_LOGD("AVCodecAudioCodecImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecAudioCodecImpl::~AVCodecAudioCodecImpl()
{
    codecService_ = nullptr;
    AVCODEC_LOGD("AVCodecAudioCodecImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVCodecAudioCodecImpl::Configure(const Format &format)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_STATE, "service died");
    auto meta = const_cast<Format &>(format).GetMeta();

    return codecService_->Configure(meta);
}

int32_t AVCodecAudioCodecImpl::Prepare()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_STATE, "service died");

    implProducer_ = implBufferQueue_->GetProducer();
    codecService_->SetOutputBufferQueue(implProducer_);
    int32_t ret = codecService_->Prepare();

    CHECK_AND_RETURN_RET_LOG(ret == 0, AVCS_ERR_INVALID_STATE, "prepare fail, ret:%{public}d", ret);

    implConsumer_ = implBufferQueue_->GetConsumer();
    mediaCodecProducer_ = codecService_->GetInputBufferQueue();
    CHECK_AND_RETURN_RET_LOG(mediaCodecProducer_ != nullptr, AVCS_ERR_INVALID_VAL, "mediaCodecProducer_ is nullptr");

    sptr<Media::IConsumerListener> comsumerListener = new AudioCodecConsumerListener(this);
    implConsumer_->SetBufferAvailableListener(comsumerListener);

    outputTask_->RegisterHandler([this] { ConsumerOutputBuffer(); });
    return AVCS_ERR_OK;
}

int32_t AVCodecAudioCodecImpl::Start()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(Prepare() == AVCS_ERR_OK, AVCS_ERR_INVALID_STATE, "Prepare failed");
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_STATE, "service died");
    int32_t ret = codecService_->Start();
    isRunning_ = true;
    indexInput_ = 0;
    indexOutput_ = 0;
    if (inputTask_) {
        inputTask_->RegisterHandler([this] { ProduceInputBuffer(); });
        inputTask_->Start();
    } else {
        AVCODEC_LOGE("Start failed, inputTask_ is nullptr, please check the inputTask_.");
        ret = AVCS_ERR_UNKNOWN;
    }
    if (outputTask_) {
        outputTask_->Start();
    } else {
        AVCODEC_LOGE("Start failed, inputTask_ is nullptr, please check the inputTask_.");
        ret = AVCS_ERR_UNKNOWN;
    }
    AVCODEC_LOGI("Start,ret = %{public}d", ret);
    return ret;
}

int32_t AVCodecAudioCodecImpl::Stop()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_STATE, "service died");
    StopTask();
    int32_t ret = codecService_->Stop();
    ClearCache();
    return ret;
}

int32_t AVCodecAudioCodecImpl::Flush()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_STATE, "service died");
    PauseTask();
    int32_t ret = codecService_->Flush();
    ClearCache();
    return ret;
}

int32_t AVCodecAudioCodecImpl::Reset()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_STATE, "service died");
    StopTask();
    int32_t ret = codecService_->Reset();
    ClearCache();
    return ret;
}

int32_t AVCodecAudioCodecImpl::Release()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_STATE, "service died");
    StopTask();
    int32_t ret = codecService_->Release();
    ClearCache();
    return ret;
}

int32_t AVCodecAudioCodecImpl::QueueInputBuffer(uint32_t index)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_STATE, "service died");
    CHECK_AND_RETURN_RET_LOG(mediaCodecProducer_ != nullptr, AVCS_ERR_INVALID_STATE,
                             "mediaCodecProducer_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(codecService_->GetStatus(), AVCS_ERR_INVALID_STATE,
                             "GetStatus is not running");
              
    std::shared_ptr<AVBuffer> buffer;
    {
        std::unique_lock lock(inputMutex_);
        auto it = inputBufferObjMap_.find(index);
        CHECK_AND_RETURN_RET_LOG(it != inputBufferObjMap_.end(), AVCS_ERR_INVALID_VAL,
            "Index does not exsist");
        buffer = it->second;
        inputBufferObjMap_.erase(index);
    }
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, AVCS_ERR_INVALID_STATE, "buffer not found");
    Media::Status ret = mediaCodecProducer_->PushBuffer(buffer, true);
    return StatusToAVCodecServiceErrCode(ret);
}

int32_t AVCodecAudioCodecImpl::GetOutputFormat(Format &format)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_STATE, "service died");
    std::shared_ptr<Media::Meta> parameter = std::make_shared<Media::Meta>();
    int32_t ret = codecService_->GetOutputFormat(parameter);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "GetOutputFormat fail, ret:%{public}d", ret);
    format.SetMeta(parameter);
    return ret;
}

int32_t AVCodecAudioCodecImpl::ReleaseOutputBuffer(uint32_t index)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_STATE, "service died");
    std::shared_ptr<AVBuffer> buffer;
    {
        std::unique_lock lock(outputMutex_);
        auto it = outputBufferObjMap_.find(index);
        CHECK_AND_RETURN_RET_LOG(it != outputBufferObjMap_.end(), AVCS_ERR_INVALID_VAL,
            "Index does not exsist");
        buffer = it->second;
        outputBufferObjMap_.erase(index);
    }
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, AVCS_ERR_INVALID_STATE, "buffer is nullptr");
    if (buffer->flag_ == AVCODEC_BUFFER_FLAG_EOS) {
        AVCODEC_LOGI("EOS detected, QueueInputBuffer set eos status.");
        codecService_->NotifyEos();
    }
    
    Media::Status ret = implConsumer_->ReleaseBuffer(buffer);
    return StatusToAVCodecServiceErrCode(ret);
}

int32_t AVCodecAudioCodecImpl::SetParameter(const Format &format)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_STATE, "service died");
    auto meta = const_cast<Format &>(format).GetMeta();
    return codecService_->SetParameter(meta);
}

int32_t AVCodecAudioCodecImpl::SetCallback(const std::shared_ptr<MediaCodecCallback> &callback)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_STATE, "service died");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AVCS_ERR_INVALID_VAL, "callback is nullptr");
    callback_ = callback;
    return AVCS_ERR_OK;
}

void AVCodecAudioCodecImpl::Notify()
{
    bufferConsumerAvailableCount_++;
	// The medicCodec has filled the buffer with the output data in this producer.
	// Notify the ProduceInputBuffer thread that it can continue fetching data from the user.
    inputCondition_.notify_one();
    outputCondition_.notify_one();
}

int32_t AVCodecAudioCodecImpl::GetInputBufferSize()
{
    int32_t capacity = 0;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, capacity, "codecService_ is nullptr");
    std::shared_ptr<Media::Meta> bufferConfig = std::make_shared<Media::Meta>();
    CHECK_AND_RETURN_RET_LOG(bufferConfig != nullptr, capacity, "bufferConfig is nullptr");
    int32_t ret = codecService_->GetOutputFormat(bufferConfig);
    CHECK_AND_RETURN_RET_LOG(ret == 0, capacity, "GetOutputFormat fail");
    CHECK_AND_RETURN_RET_LOG(bufferConfig->Get<Media::Tag::AUDIO_MAX_INPUT_SIZE>(capacity), capacity,
                             "get max input buffer size fail");
    return capacity;
}

void AVCodecAudioCodecImpl::ProduceInputBuffer()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "produceInputBuffer enter");
    if (!isRunning_) {
        usleep(DEFAULT_TRY_DECODE_TIME);
        AVCODEC_LOGE("ProduceInputBuffer isRunning_ false");
        return;
    }
    Media::Status ret = Media::Status::OK;
    Media::AVBufferConfig avBufferConfig;
    avBufferConfig.size = GetInputBufferSize();
    std::unique_lock lock2(inputMutex2_);
    while (isRunning_) {
        std::shared_ptr<AVBuffer> emptyBuffer = nullptr;
        CHECK_AND_CONTINUE_LOG(mediaCodecProducer_ != nullptr, "mediaCodecProducer_ is nullptr");
        ret = mediaCodecProducer_->RequestBuffer(emptyBuffer, avBufferConfig, TIME_OUT_MS);
        if (ret != Media::Status::OK) {
            AVCODEC_LOGW("produceInputBuffer RequestBuffer fail, ret=%{public}d", ret);
            break;
        }
        CHECK_AND_CONTINUE_LOG(emptyBuffer != nullptr, "buffer is nullptr");
        {
            std::unique_lock lock1(inputMutex_);
            inputBufferObjMap_[indexInput_] = emptyBuffer;
        }
        CHECK_AND_CONTINUE_LOG(callback_ != nullptr, "callback is nullptr");
        callback_->OnInputBufferAvailable(indexInput_, emptyBuffer);
        indexInput_++;
    }
    inputCondition_.wait_for(lock2, std::chrono::milliseconds(MILLISECONDS),
                             [this] { return ((bufferConsumerAvailableCount_ > 0) || !isRunning_); });
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "produceInputBuffer exit");
}

void AVCodecAudioCodecImpl::ConsumerOutputBuffer()
{
    AVCODEC_SYNC_TRACE;
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "ConsumerOutputBuffer enter");
    if (!isRunning_) {
        usleep(DEFAULT_TRY_DECODE_TIME);
        AVCODEC_LOGE("Consumer isRunning_ false");
        return;
    }
    Media::Status ret = Media::Status::OK;
    std::unique_lock lock2(outputMutex_2);
    while (isRunning_ && (bufferConsumerAvailableCount_ > 0)) {
        std::shared_ptr<AVBuffer> filledInputBuffer;
        ret = implConsumer_->AcquireBuffer(filledInputBuffer);
        if (ret != Media::Status::OK) {
            AVCODEC_LOGE("Consumer AcquireBuffer fail,ret=%{public}d", ret);
            break;
        }
        {
            std::unique_lock lock(outputMutex_);
            outputBufferObjMap_[indexOutput_] = filledInputBuffer;
        }
        callback_->OnOutputBufferAvailable(indexOutput_, filledInputBuffer);
        indexOutput_++;
        bufferConsumerAvailableCount_--;
    }
    outputCondition_.wait_for(lock2, std::chrono::milliseconds(MILLISECONDS),
                              [this] { return ((bufferConsumerAvailableCount_ > 0) || !isRunning_); });
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "ConsumerOutputBuffer exit");
}

void AVCodecAudioCodecImpl::ClearCache()
{
    for (auto iter = outputBufferObjMap_.begin(); iter != outputBufferObjMap_.end();) {
        std::shared_ptr<AVBuffer> buffer = iter->second;
        iter = outputBufferObjMap_.erase(iter);
        implConsumer_->ReleaseBuffer(buffer);
    }
    inputBufferObjMap_.clear();
}

void AVCodecAudioCodecImpl::StopTask()
{
    isRunning_ = false;
    if (inputTask_) {
        inputTask_->Stop();
    }
    if (outputTask_) {
        outputTask_->Stop();
    }
}

void AVCodecAudioCodecImpl::PauseTask()
{
    isRunning_ = false;
    if (inputTask_) {
        inputTask_->Pause();
    }
    if (outputTask_) {
        outputTask_->Pause();
    }
}
} // namespace MediaAVCodec
} // namespace OHOS
