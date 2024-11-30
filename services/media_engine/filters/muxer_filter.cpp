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

#include "muxer_filter.h"
#include <sys/timeb.h>
#include <unordered_map>
#include "common/log.h"
#include "filter/filter_factory.h"
#include "muxer/media_muxer.h"
#include "avcodec_trace.h"
#include "avcodec_sysevent.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_DOMAIN_SYSTEM_PLAYER, "MuxerFilter" };
static const std::unordered_map<OHOS::Media::Plugins::OutputFormat, std::string> FORMAT_TABLE = {
    {OHOS::Media::Plugins::OutputFormat::DEFAULT, OHOS::Media::Plugins::MimeType::MEDIA_MP4},
    {OHOS::Media::Plugins::OutputFormat::MPEG_4, OHOS::Media::Plugins::MimeType::MEDIA_MP4},
    {OHOS::Media::Plugins::OutputFormat::M4A, OHOS::Media::Plugins::MimeType::MEDIA_M4A},
    {OHOS::Media::Plugins::OutputFormat::AMR, OHOS::Media::Plugins::MimeType::MEDIA_AMR},
    {OHOS::Media::Plugins::OutputFormat::MP3, OHOS::Media::Plugins::MimeType::MEDIA_MP3},
    {OHOS::Media::Plugins::OutputFormat::WAV, OHOS::Media::Plugins::MimeType::MEDIA_WAV},
};
}

namespace OHOS {
namespace Media {
namespace Pipeline {
using namespace OHOS::MediaAVCodec;
constexpr int64_t WAIT_TIME_OUT_NS = 3000000000;
constexpr int64_t US_TO_MS = 1000;
constexpr uint32_t BUFFER_IS_EOS = 1;
static AutoRegisterFilter<MuxerFilter> g_registerMuxerFilter("builtin.recorder.muxer", FilterType::FILTERTYPE_MUXER,
    [](const std::string& name, const FilterType type) {
        return std::make_shared<MuxerFilter>(name, FilterType::FILTERTYPE_MUXER);
    });

class MuxerBrokerListener : public IBrokerListener {
public:
    MuxerBrokerListener(std::shared_ptr<MuxerFilter> muxerFilter, int32_t trackIndex,
        StreamType streamType, sptr<AVBufferQueueProducer> inputBufferQueue)
        : muxerFilter_(std::move(muxerFilter)), trackIndex_(trackIndex), streamType_(streamType),
        inputBufferQueue_(inputBufferQueue)
    {
    }

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    void OnBufferFilled(std::shared_ptr<AVBuffer> &avBuffer) override
    {
        if (inputBufferQueue_ != nullptr) {
            if (auto muxerFilter = muxerFilter_.lock()) {
                muxerFilter->OnBufferFilled(avBuffer, trackIndex_, streamType_, inputBufferQueue_.promote());
            } else {
                MEDIA_LOG_I("invalid muxerFilter");
            }
        }
    }

private:
    std::weak_ptr<MuxerFilter> muxerFilter_;
    int32_t trackIndex_;
    StreamType streamType_;
    wptr<AVBufferQueueProducer> inputBufferQueue_;
};

MuxerFilter::MuxerFilter(std::string name, FilterType type): Filter(name, type)
{
    MEDIA_LOG_I("MuxerFilter create");
}

MuxerFilter::~MuxerFilter()
{
    MEDIA_LOG_I("MuxerFilter destroy");
}

Status MuxerFilter::SetOutputParameter(int32_t appUid, int32_t appPid, int32_t fd, int32_t format)
{
    MEDIA_LOG_I("SetOutputParameter, appUid:" PUBLIC_LOG_D32 ", appPid:" PUBLIC_LOG_D32 ", format:" PUBLIC_LOG_D32,
        static_cast<int32_t>(appUid), static_cast<int32_t>(appPid), static_cast<int32_t>(format));
    mediaMuxer_ = std::make_shared<MediaMuxer>(appUid, appPid);
    Status ret = mediaMuxer_->Init(fd, (Plugins::OutputFormat)format);
    outputFormat_ = (Plugins::OutputFormat)format;
    if (ret != Status::OK) {
        SetFaultEvent("MuxerFilter::SetOutputParameter, muxerFilter init error", (int32_t)ret);
    }
    return ret;
}

Status MuxerFilter::SetTransCoderMode()
{
    MEDIA_LOG_I("SetTransCoderMode");
    isTransCoderMode = true;
    return Status::OK;
}
 
int64_t MuxerFilter::GetCurrentPtsMs()
{
    if (lastVideoPts_ != 0) {
        return lastVideoPts_ / US_TO_MS;
    } else {
        return lastAudioPts_ / US_TO_MS;
    }
}

void MuxerFilter::Init(const std::shared_ptr<EventReceiver> &receiver,
    const std::shared_ptr<FilterCallback> &callback)
{
    MEDIA_LOG_I("Init");
    MediaAVCodec::AVCodecTrace trace("MuxerFilter::Init");
    eventReceiver_ = receiver;
    filterCallback_ = callback;
}

Status MuxerFilter::DoPrepare()
{
    MEDIA_LOG_I("Prepare");
    MediaAVCodec::AVCodecTrace trace("MuxerFilter::Prepare");
    return Status::OK;
}

Status MuxerFilter::DoStart()
{
    MEDIA_LOG_I("Start");
    MediaAVCodec::AVCodecTrace trace("MuxerFilter::Start");
    startCount_++;
    if (startCount_ == preFilterCount_) {
        startCount_ = 0;
        Status ret = mediaMuxer_->Start();
        if (ret != Status::OK) {
            SetFaultEvent("MuxerFilter::DoStart error", (int32_t)ret);
        }
        return ret;
    } else {
        return Status::OK;
    }
}

Status MuxerFilter::DoPause()
{
    MediaAVCodec::AVCodecTrace trace("MuxerFilter::Pause");
    MEDIA_LOG_I("Pause");
    return Status::OK;
}

Status MuxerFilter::DoResume()
{
    MediaAVCodec::AVCodecTrace trace("MuxerFilter::Resume");
    MEDIA_LOG_I("Resume");
    return Status::OK;
}

Status MuxerFilter::DoStop()
{
    MEDIA_LOG_I("Stop");
    MediaAVCodec::AVCodecTrace trace("MuxerFilter::Stop");
    stopCount_++;
    Status ret = Status::OK;
    if (stopCount_ == preFilterCount_) {
        stopCount_ = 0;
        ret = mediaMuxer_->Stop();
        if (ret == Status::ERROR_WRONG_STATE) {
            return Status::OK;
        }
    }
    if (ret != Status::OK) {
        SetFaultEvent("MuxerFilter::DoStop error", (int32_t)ret);
    }
    return ret;
}

Status MuxerFilter::DoFlush()
{
    return Status::OK;
}

Status MuxerFilter::DoRelease()
{
    MEDIA_LOG_I("Release");
    return Status::OK;
}

void MuxerFilter::SetParameter(const std::shared_ptr<Meta> &parameter)
{
    MEDIA_LOG_I("SetParameter");
    MediaAVCodec::AVCodecTrace trace("MuxerFilter::SetParameter");
    mediaMuxer_->SetParameter(parameter);
}

void MuxerFilter::SetUserMeta(const std::shared_ptr<Meta> &userMeta)
{
    MEDIA_LOG_I("SetUserMeta enter");
    Status ret = mediaMuxer_->SetUserMeta(userMeta);
    if (ret != Status::OK) {
        MEDIA_LOG_I("SetUserMeta failed");
    }
}

void MuxerFilter::GetParameter(std::shared_ptr<Meta> &parameter)
{
    MEDIA_LOG_I("GetParameter");
    MediaAVCodec::AVCodecTrace trace("MuxerFilter::GetParameter");
}

Status MuxerFilter::LinkNext(const std::shared_ptr<Filter> &nextFilter, StreamType outType)
{
    return Status::OK;
}

Status MuxerFilter::UpdateNext(const std::shared_ptr<Filter> &nextFilter, StreamType outType)
{
    MEDIA_LOG_I("UpdateNext");
    return Status::OK;
}

Status MuxerFilter::UnLinkNext(const std::shared_ptr<Filter> &nextFilter, StreamType outType)
{
    MEDIA_LOG_I("UnLinkNext");
    return Status::OK;
}

FilterType MuxerFilter::GetFilterType()
{
    MEDIA_LOG_I("GetFilterType");
    return FilterType::FILTERTYPE_MUXER;
}

Status MuxerFilter::OnLinked(StreamType inType, const std::shared_ptr<Meta> &meta,
    const std::shared_ptr<FilterLinkCallback> &callback)
{
    MEDIA_LOG_I("OnLinked");
    MediaAVCodec::AVCodecTrace trace("MuxerFilter::OnLinked");
    int32_t trackIndex;
    std::string mimeType;
    meta->Get<Tag::MIME_TYPE>(mimeType);
    if (mimeType.find("audio/") == 0) {
        audioCodecMimeType_ = mimeType;
    } else if (mimeType.find("video/") == 0) {
        videoCodecMimeType_ = mimeType;
    } else if (mimeType.find("meta/") == 0) {
        metaDataCodecMimeType_ = mimeType;
        std::string srcMimeType;
        meta->Get<Tag::TIMED_METADATA_SRC_TRACK_MIME>(srcMimeType);
        if (trackIndexMap_.find(srcMimeType) != trackIndexMap_.end()) {
            auto sourceTrackIndex = trackIndexMap_.at(videoCodecMimeType_);
            meta->Set<Tag::TIMED_METADATA_SRC_TRACK>(sourceTrackIndex);
        }
    }
    auto ret = mediaMuxer_->AddTrack(trackIndex, meta);
    if (ret != Status::OK) {
        eventReceiver_->OnEvent({"muxer_filter", EventType::EVENT_ERROR, ret});
        SetFaultEvent("MuxerFilter::OnLinked error", (int32_t)ret);
        return ret;
    }
    trackIndexMap_.emplace(std::make_pair(mimeType, trackIndex));
    sptr<AVBufferQueueProducer> inputBufferQueue = mediaMuxer_->GetInputBufferQueue(trackIndex);
    callback->OnLinkedResult(inputBufferQueue, const_cast<std::shared_ptr<Meta> &>(meta));
    sptr<IBrokerListener> listener = new MuxerBrokerListener(shared_from_this(), trackIndex,
        inType, inputBufferQueue);
    inputBufferQueue->SetBufferFilledListener(listener);
    preFilterCount_++;
    bufferPtsMap_.insert(std::pair<int32_t, int64_t>(trackIndex, 0));
    return Status::OK;
}

Status MuxerFilter::OnUpdated(StreamType inType, const std::shared_ptr<Meta> &meta,
    const std::shared_ptr<FilterLinkCallback> &callback)
{
    MEDIA_LOG_I("OnUpdated");
    return Status::OK;
}


Status MuxerFilter::OnUnLinked(StreamType inType, const std::shared_ptr<FilterLinkCallback> &callback)
{
    MEDIA_LOG_I("OnUnLinked");
    return Status::OK;
}

void MuxerFilter::OnBufferFilled(std::shared_ptr<AVBuffer> &inputBuffer, int32_t trackIndex,
    StreamType streamType, sptr<AVBufferQueueProducer> inputBufferQueue)
{
    MEDIA_LOG_D("OnBufferFilled");
    MediaAVCodec::AVCodecTrace trace("MuxerFilter::OnBufferFilled");
    if (!isTransCoderMode) {
        int64_t currentBufferPts = inputBuffer->pts_;
        int64_t anotherBufferPts = 0;
        for (auto mapInterator = bufferPtsMap_.begin(); mapInterator != bufferPtsMap_.end(); mapInterator++) {
            if (mapInterator->first != trackIndex) {
                anotherBufferPts = mapInterator->second;
            }
        }
        bufferPtsMap_[trackIndex] = currentBufferPts;
        if (preFilterCount_ != 1 && std::abs(currentBufferPts - anotherBufferPts) >= WAIT_TIME_OUT_NS) {
            MEDIA_LOG_I("OnBufferFilled pts time interval is greater than 3 seconds");
        }
        MEDIA_LOG_D("OnBufferFilled buffer->pts" PUBLIC_LOG_D64, inputBuffer->pts_);
        inputBufferQueue->ReturnBuffer(inputBuffer, true);
        return;
    }
    OnTransCoderBufferFilled(inputBuffer, trackIndex, streamType, inputBufferQueue);
}

void MuxerFilter::OnTransCoderBufferFilled(std::shared_ptr<AVBuffer> &inputBuffer, int32_t trackIndex,
    StreamType streamType, sptr<AVBufferQueueProducer> inputBufferQueue)
{
    MEDIA_LOG_D("OnTransCoderBufferFilled");
    if ((inputBuffer->flag_ & BUFFER_IS_EOS) == 1) {
        eosCount_++;
        if (streamType == StreamType::STREAMTYPE_ENCODED_VIDEO) {
            MEDIA_LOG_I("video is eos");
            videoIsEos = true;
        } else if (streamType == StreamType::STREAMTYPE_ENCODED_AUDIO) {
            MEDIA_LOG_I("audio is eos");
            audioIsEos = true;
        }
    }
    if ((eosCount_ == preFilterCount_) || (videoIsEos && audioIsEos)) {
        eventReceiver_->OnEvent({"muxer_filter", EventType::EVENT_COMPLETE, Status::OK});
    }
    if (streamType == StreamType::STREAMTYPE_ENCODED_AUDIO) {
        lastAudioPts_ = inputBuffer->pts_;
        if (videoCodecMimeType_.empty()) {
            inputBufferQueue->ReturnBuffer(inputBuffer, true);
        } else if (inputBuffer->pts_ <= lastVideoPts_ || videoIsEos) {
            inputBufferQueue->ReturnBuffer(inputBuffer, true);
        } else {
            std::unique_lock<std::mutex> lock(stopMutex_);
            stopCondition_.wait_for(lock, std::chrono::milliseconds(US_TO_MS));
            inputBufferQueue->ReturnBuffer(inputBuffer, true);
        }
    } else if (streamType == StreamType::STREAMTYPE_ENCODED_VIDEO) {
        lastVideoPts_ = inputBuffer->pts_;
        std::unique_lock<std::mutex> lock(stopMutex_);
        stopCondition_.notify_all();
        inputBufferQueue->ReturnBuffer(inputBuffer, true);
    } else {
        inputBufferQueue->ReturnBuffer(inputBuffer, true);
    }
}

void MuxerFilter::SetFaultEvent(const std::string &errMsg, int32_t ret)
{
    SetFaultEvent(errMsg + ", ret = " + std::to_string(ret));
}

void MuxerFilter::SetFaultEvent(const std::string &errMsg)
{
    MuxerFaultInfo muxerFaultInfo;
    muxerFaultInfo.appName = bundleName_;
    muxerFaultInfo.instanceId = std::to_string(instanceId_);
    muxerFaultInfo.callerType = "player_framework";
    muxerFaultInfo.videoCodec = videoCodecMimeType_;
    muxerFaultInfo.audioCodec = audioCodecMimeType_;
    muxerFaultInfo.metaCodec = metaDataCodecMimeType_;
    muxerFaultInfo.containerFormat = GetContainerFormat(outputFormat_);
    muxerFaultInfo.errMsg = errMsg;
    FaultMuxerEventWrite(muxerFaultInfo);
}

const std::string &MuxerFilter::GetContainerFormat(Plugins::OutputFormat format)
{
    static std::string emptyFormat = "";
    FALSE_RETURN_V_MSG_E(FORMAT_TABLE.find(format) != FORMAT_TABLE.end(), emptyFormat,
        "The output format %{public}d is not supported!", format);
    return FORMAT_TABLE.at(format);
}

void MuxerFilter::SetCallingInfo(int32_t appUid, int32_t appPid,
    const std::string &bundleName, uint64_t instanceId)
{
    appUid_ = appUid;
    appPid_ = appPid;
    bundleName_ = bundleName;
    instanceId_ = instanceId;
}
} // namespace Pipeline
} // namespace MEDIA
} // namespace OHOS