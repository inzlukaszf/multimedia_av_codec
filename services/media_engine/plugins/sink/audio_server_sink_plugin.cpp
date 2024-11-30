/*
 * Copyright (C) 2023-2024 Huawei Device Co., Ltd.
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

#define MEDIA_PLUGIN
#define HST_LOG_TAG "ASSP"

#include "audio_server_sink_plugin.h"
#include <functional>
#include "audio_errors.h"
#include "avcodec_trace.h"
#include "common/log.h"
#include "common/media_core.h"
#include "meta/meta_key.h"
#include "osal/task/autolock.h"
#include "osal/task/jobutils.h"
#include "common/status.h"
#include "common/media_core.h"
#include "audio_info.h"
#include "cpp_ext/algorithm_ext.h"
#include "osal/utils/steady_clock.h"
#include "plugin/plugin_time.h"
#include "param_wrapper.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_ONLY_PRERELEASE, LOG_DOMAIN_SYSTEM_PLAYER, "HiStreamer" };
using namespace OHOS::Media::Plugins;
constexpr int TUPLE_SECOND_ITEM_INDEX = 2;
constexpr int32_t DEFAULT_BUFFER_NUM = 8;

const std::pair<AudioInterruptMode, OHOS::AudioStandard::InterruptMode> g_auInterruptMap[] = {
    {AudioInterruptMode::SHARE_MODE, OHOS::AudioStandard::InterruptMode::SHARE_MODE},
    {AudioInterruptMode::INDEPENDENT_MODE, OHOS::AudioStandard::InterruptMode::INDEPENDENT_MODE},
};

const std::vector<std::tuple<AudioSampleFormat, OHOS::AudioStandard::AudioSampleFormat, AVSampleFormat>> g_aduFmtMap = {
    {AudioSampleFormat::SAMPLE_S8, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::SAMPLE_U8, OHOS::AudioStandard::AudioSampleFormat::SAMPLE_U8, AV_SAMPLE_FMT_U8},
    {AudioSampleFormat::SAMPLE_S8P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::SAMPLE_U8P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_U8P},
    {AudioSampleFormat::SAMPLE_S16LE, OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S16LE, AV_SAMPLE_FMT_S16},
    {AudioSampleFormat::SAMPLE_U16, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::SAMPLE_S16P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_S16P},
    {AudioSampleFormat::SAMPLE_U16P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::SAMPLE_S24LE, OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S24LE, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::SAMPLE_U24, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::SAMPLE_S24P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::SAMPLE_U24P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::SAMPLE_S32LE, OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S32LE, AV_SAMPLE_FMT_S32},
    {AudioSampleFormat::SAMPLE_U32, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::SAMPLE_S32P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_S32P},
    {AudioSampleFormat::SAMPLE_U32P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::SAMPLE_F32LE, OHOS::AudioStandard::AudioSampleFormat::SAMPLE_F32LE, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::SAMPLE_F32P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_FLTP},
    {AudioSampleFormat::SAMPLE_F64, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_DBL},
    {AudioSampleFormat::SAMPLE_F64P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_DBLP},
    {AudioSampleFormat::SAMPLE_S64, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_S64},
    {AudioSampleFormat::SAMPLE_U64, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
    {AudioSampleFormat::SAMPLE_S64P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_S64P},
    {AudioSampleFormat::SAMPLE_U64P, OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH, AV_SAMPLE_FMT_NONE},
};

void AudioInterruptMode2InterruptMode(AudioInterruptMode audioInterruptMode,
    OHOS::AudioStandard::InterruptMode &interruptMode)
{
    for (const auto &item : g_auInterruptMap) {
        if (item.first == audioInterruptMode) {
            interruptMode = item.second;
        }
    }
}

void UpdateSupportedSampleRate(Capability &inCaps)
{
    auto supportedSampleRateList = OHOS::AudioStandard::AudioRenderer::GetSupportedSamplingRates();
    if (!supportedSampleRateList.empty()) {
        DiscreteCapability<uint32_t> values;
        for (const auto &rate : supportedSampleRateList) {
            values.push_back(static_cast<uint32_t>(rate));
        }
        if (!values.empty()) {
            inCaps.AppendDiscreteKeys<uint32_t>(OHOS::Media::Tag::AUDIO_SAMPLE_RATE, values);
        }
    }
}

void UpdateSupportedSampleFormat(Capability &inCaps)
{
    DiscreteCapability<AudioSampleFormat> values(g_aduFmtMap.size());
    for (const auto &item : g_aduFmtMap) {
        if (std::get<1>(item) != OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH ||
            std::get<TUPLE_SECOND_ITEM_INDEX>(item) != AV_SAMPLE_FMT_NONE) {
            values.emplace_back(std::get<0>(item));
        }
    }
    inCaps.AppendDiscreteKeys(OHOS::Media::Tag::AUDIO_SAMPLE_FORMAT, values);
}

OHOS::Media::Status AudioServerSinkRegister(const std::shared_ptr<Register> &reg)
{
    AudioSinkPluginDef definition;
    definition.name = "AudioServerSink";
    definition.description = "Audio sink for audio server of media standard";
    definition.rank = 100; // 100: max rank
    auto func = [](const std::string &name) -> std::shared_ptr<AudioSinkPlugin> {
        return std::make_shared<OHOS::Media::Plugins::AudioServerSinkPlugin>(name);
    };
    definition.SetCreator(func);
    Capability inCaps(MimeType::AUDIO_RAW);
    UpdateSupportedSampleRate(inCaps);
    UpdateSupportedSampleFormat(inCaps);
    definition.AddInCaps(inCaps);
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(AudioServerSink, LicenseType::APACHE_V2, AudioServerSinkRegister, [] {});

inline void ResetAudioRendererParams(OHOS::AudioStandard::AudioRendererParams &param)
{
    using namespace OHOS::AudioStandard;
    param.sampleFormat = OHOS::AudioStandard::INVALID_WIDTH;
    param.sampleRate = SAMPLE_RATE_8000;
    param.channelCount = OHOS::AudioStandard::MONO;
    param.encodingType = ENCODING_INVALID;
}
} // namespace

namespace OHOS {
namespace Media {
namespace Plugins {
using namespace OHOS::Media::Plugins;

AudioServerSinkPlugin::AudioRendererCallbackImpl::AudioRendererCallbackImpl(
    const std::shared_ptr<Pipeline::EventReceiver> &receiver, const bool &isPaused) : playerEventReceiver_(receiver),
    isPaused_(isPaused)
{
}

AudioServerSinkPlugin::AudioServiceDiedCallbackImpl::AudioServiceDiedCallbackImpl(
    std::shared_ptr<Pipeline::EventReceiver> &receiver) : playerEventReceiver_(receiver)
{
}

AudioServerSinkPlugin::AudioFirstFrameCallbackImpl::AudioFirstFrameCallbackImpl(
    std::shared_ptr<Pipeline::EventReceiver> &receiver)
{
    FALSE_RETURN(receiver != nullptr);
    playerEventReceiver_ = receiver;
}

void AudioServerSinkPlugin::AudioRendererCallbackImpl::OnInterrupt(
    const OHOS::AudioStandard::InterruptEvent &interruptEvent)
{
    MEDIA_LOG_D_SHORT("OnInterrupt forceType is " PUBLIC_LOG_U32, static_cast<uint32_t>(interruptEvent.forceType));
    if (interruptEvent.forceType == OHOS::AudioStandard::INTERRUPT_FORCE) {
        switch (interruptEvent.hintType) {
            case OHOS::AudioStandard::INTERRUPT_HINT_PAUSE:
                isPaused_ = true;
                break;
            default:
                isPaused_ = false;
                break;
        }
    }
    Event event {
        .srcFilter = "Audio interrupt event",
        .type = EventType::EVENT_AUDIO_INTERRUPT,
        .param = interruptEvent
    };
    FALSE_RETURN(playerEventReceiver_ != nullptr);
    MEDIA_LOG_D_SHORT("OnInterrupt event upload ");
    playerEventReceiver_->OnEvent(event);
}

void AudioServerSinkPlugin::AudioRendererCallbackImpl::OnStateChange(
    const OHOS::AudioStandard::RendererState state, const OHOS::AudioStandard::StateChangeCmdType cmdType)
{
    MEDIA_LOG_D_SHORT("RenderState is " PUBLIC_LOG_U32, static_cast<uint32_t>(state));
    if (cmdType == AudioStandard::StateChangeCmdType::CMD_FROM_SYSTEM) {
        Event event {
            .srcFilter = "Audio state change event",
            .type = EventType::EVENT_AUDIO_STATE_CHANGE,
            .param = state
        };
        FALSE_RETURN(playerEventReceiver_ != nullptr);
        playerEventReceiver_->OnEvent(event);
    }
}

void AudioServerSinkPlugin::AudioRendererCallbackImpl::OnOutputDeviceChange(
    const AudioStandard::DeviceInfo &deviceInfo, const AudioStandard::AudioStreamDeviceChangeReason reason)
{
    MEDIA_LOG_D_SHORT("DeviceChange reason is " PUBLIC_LOG_D32, static_cast<int32_t>(reason));
    auto param = std::make_pair(deviceInfo, reason);
    Event event {
        .srcFilter = "Audio deviceChange change event",
        .type = EventType::EVENT_AUDIO_DEVICE_CHANGE,
        .param = param
    };
    FALSE_RETURN(playerEventReceiver_ != nullptr);
    playerEventReceiver_->OnEvent(event);
}

void AudioServerSinkPlugin::AudioFirstFrameCallbackImpl::OnFirstFrameWriting(uint64_t latency)
{
    MEDIA_LOG_I_SHORT("OnFirstFrameWriting latency: " PUBLIC_LOG_U64, latency);
    Event event {
        .srcFilter = "Audio render first frame writing event",
        .type = EventType::EVENT_AUDIO_FIRST_FRAME,
        .param = latency
    };
    FALSE_RETURN(playerEventReceiver_ != nullptr);
    playerEventReceiver_->OnEvent(event);
    MEDIA_LOG_I_SHORT("OnFirstFrameWriting event upload ");
}

void AudioServerSinkPlugin::AudioServiceDiedCallbackImpl::OnAudioPolicyServiceDied()
{
    MEDIA_LOG_I_SHORT("OnAudioPolicyServiceDied enter");
    Event event {
        .srcFilter = "Audio service died event",
        .type = EventType::EVENT_AUDIO_SERVICE_DIED
    };
    FALSE_RETURN(playerEventReceiver_ != nullptr);
    playerEventReceiver_->OnEvent(event);
    MEDIA_LOG_I_SHORT("OnAudioPolicyServiceDied end");
}

AudioServerSinkPlugin::AudioServerSinkPlugin(std::string name)
    : Plugins::AudioSinkPlugin(std::move(name)), audioRenderer_(nullptr)
{
    SetUpParamsSetterMap();
    SetAudioDumpBySysParam();
}

AudioServerSinkPlugin::~AudioServerSinkPlugin()
{
    MEDIA_LOG_D("~AudioServerSinkPlugin() entered.");
    ReleaseRender();
    ReleaseFile();
}

Status AudioServerSinkPlugin::Init()
{
    MediaAVCodec::AVCodecTrace trace("AudioServerSinkPlugin::Init");
    MEDIA_LOG_I_SHORT("Init entered.");
    FALSE_RETURN_V_MSG(audioRenderer_ == nullptr, Status::OK, "audio renderer already create ");
    AudioStandard::AppInfo appInfo;
    appInfo.appPid = appPid_;
    appInfo.appUid = appUid_;
    MEDIA_LOG_I_SHORT("Create audio renderer for apppid_ " PUBLIC_LOG_D32 " appuid_ " PUBLIC_LOG_D32
                " contentType " PUBLIC_LOG_D32 " streamUsage " PUBLIC_LOG_D32 " rendererFlags " PUBLIC_LOG_D32
                " audioInterruptMode_ " PUBLIC_LOG_U32,
                appPid_, appUid_, audioRenderInfo_.contentType, audioRenderInfo_.streamUsage,
                audioRenderInfo_.rendererFlags, static_cast<uint32_t>(audioInterruptMode_));
    rendererOptions_.rendererInfo.contentType = static_cast<AudioStandard::ContentType>(audioRenderInfo_.contentType);
    rendererOptions_.rendererInfo.streamUsage = static_cast<AudioStandard::StreamUsage>(audioRenderInfo_.streamUsage);
    rendererOptions_.rendererInfo.rendererFlags = audioRenderInfo_.rendererFlags;
    rendererOptions_.streamInfo.samplingRate = rendererParams_.sampleRate;
    rendererOptions_.streamInfo.encoding =
        mimeType_ == MimeType::AUDIO_AVS3DA ? AudioStandard::ENCODING_AUDIOVIVID : AudioStandard::ENCODING_PCM;
    rendererOptions_.streamInfo.format = rendererParams_.sampleFormat;
    rendererOptions_.streamInfo.channels = rendererParams_.channelCount;
    MEDIA_LOG_I_SHORT("Create audio renderer for samplingRate " PUBLIC_LOG_D32 " encoding " PUBLIC_LOG_D32
                " sampleFormat " PUBLIC_LOG_D32 " channels " PUBLIC_LOG_D32,
                rendererOptions_.streamInfo.samplingRate, rendererOptions_.streamInfo.encoding,
                rendererOptions_.streamInfo.format, rendererOptions_.streamInfo.channels);
    audioRenderer_ = AudioStandard::AudioRenderer::Create(rendererOptions_, appInfo);
    if (audioRenderer_ == nullptr && playerEventReceiver_ != nullptr) {
        playerEventReceiver_->OnEvent({"audioSinkPlugin", EventType::EVENT_ERROR, MSERR_UNSUPPORT_AUD_SAMPLE_RATE});
    }
    FALSE_RETURN_V(audioRenderer_ != nullptr, Status::ERROR_NULL_POINTER);
    if (audioRenderSetFlag_ && (audioRenderInfo_.streamUsage == AudioStandard::STREAM_USAGE_MUSIC ||
        audioRenderInfo_.streamUsage == AudioStandard::STREAM_USAGE_AUDIOBOOK)) {
        audioRenderer_->SetOffloadAllowed(true);
    } else {
        audioRenderer_->SetOffloadAllowed(false);
    }
    audioRenderer_->SetInterruptMode(audioInterruptMode_);
    return Status::OK;
}

void AudioServerSinkPlugin::ReleaseRender()
{
    if (audioRenderer_ != nullptr && audioRenderer_->GetStatus() != AudioStandard::RendererState::RENDERER_RELEASED) {
        MEDIA_LOG_I_SHORT("AudioRenderer::Release start");
        FALSE_RETURN_MSG(audioRenderer_->Release(), "AudioRenderer::Release failed");
        MEDIA_LOG_I_SHORT("AudioRenderer::Release end");
    }
    audioRenderer_.reset();
}

void AudioServerSinkPlugin::ReleaseFile()
{
    if (entireDumpFile_ != nullptr) {
        (void)fclose(entireDumpFile_);
        entireDumpFile_ = nullptr;
    }
    if (sliceDumpFile_ != nullptr) {
        (void)fclose(sliceDumpFile_);
        sliceDumpFile_ = nullptr;
    }
}

Status AudioServerSinkPlugin::Deinit()
{
    MEDIA_LOG_D_SHORT("Deinit entered");
    ReleaseRender();
    return Status::OK;
}

Status AudioServerSinkPlugin::Prepare()
{
    MediaAVCodec::AVCodecTrace trace("AudioServerSinkPlugin::Prepare");
    MEDIA_LOG_D_SHORT("Prepare >>");
    auto types = AudioStandard::AudioRenderer::GetSupportedEncodingTypes();
    bool ret = CppExt::AnyOf(types.begin(), types.end(), [](AudioStandard::AudioEncodingType tmp) -> bool {
        return tmp == AudioStandard::ENCODING_PCM;
    });
    FALSE_RETURN_V_MSG(ret, Status::ERROR_INVALID_PARAMETER, "audio renderer do not support pcm encoding");
    {
        FALSE_RETURN_V(audioRenderer_ != nullptr, Status::ERROR_NULL_POINTER);
        if (audioRendererCallback_ == nullptr) {
            audioRendererCallback_ = std::make_shared<AudioRendererCallbackImpl>(playerEventReceiver_, isForcePaused_);
            audioRenderer_->SetRendererCallback(audioRendererCallback_);
            audioRenderer_->RegisterOutputDeviceChangeWithInfoCallback(audioRendererCallback_);
        }
        if (audioFirstFrameCallback_ == nullptr) {
            audioFirstFrameCallback_ = std::make_shared<AudioFirstFrameCallbackImpl>(playerEventReceiver_);
            audioRenderer_->SetRendererFirstFrameWritingCallback(audioFirstFrameCallback_);
        }
        if (audioServiceDiedCallback_ == nullptr) {
            audioServiceDiedCallback_ = std::make_shared<AudioServiceDiedCallbackImpl>(playerEventReceiver_);
            audioRenderer_->RegisterAudioPolicyServerDiedCb(getprocpid(), audioServiceDiedCallback_);
        }
    }
    MEDIA_LOG_I_SHORT("Prepare <<");
    return Status::OK;
}

bool AudioServerSinkPlugin::StopRender()
{
    if (audioRenderer_) {
        FALSE_RETURN_V_MSG(audioRenderer_->GetStatus() != AudioStandard::RendererState::RENDERER_STOPPED,
            true, "AudioRenderer is already in stopped state.");
        sliceCount_++;
        return audioRenderer_->Stop();
    }
    return true;
}

Status AudioServerSinkPlugin::Reset()
{
    MediaAVCodec::AVCodecTrace trace("AudioServerSinkPlugin::Reset");
    MEDIA_LOG_I_SHORT("Reset entered.");
    FALSE_RETURN_V_MSG(StopRender(), Status::ERROR_UNKNOWN, "stop render error");
    ResetAudioRendererParams(rendererParams_);
    fmtSupported_ = false;
    reSrcFfFmt_ = AV_SAMPLE_FMT_NONE;
    channels_ = 0;
    bitRate_ = 0;
    sampleRate_ = 0;
    samplesPerFrame_ = 0;
    needReformat_ = false;
    if (resample_) {
        resample_.reset();
    }
    return Status::OK;
}

Status AudioServerSinkPlugin::Start()
{
    MediaAVCodec::AVCodecTrace trace("AudioServerSinkPlugin::Start");
    MEDIA_LOG_D_SHORT("Start entered.");
    if (audioRenderer_ == nullptr) {
        return Status::ERROR_WRONG_STATE;
    }
    bool ret = audioRenderer_->Start();
    FALSE_RETURN_V_MSG(ret, Status::ERROR_UNKNOWN, "AudioRenderer::Start failed");
    MEDIA_LOG_I_SHORT("AudioRenderer::Start end");
    return Status::OK;
}

Status AudioServerSinkPlugin::Stop()
{
    MediaAVCodec::AVCodecTrace trace("AudioServerSinkPlugin::Stop");
    MEDIA_LOG_D_SHORT("Stop entered.");
    FALSE_RETURN_V_MSG(StopRender(), Status::ERROR_UNKNOWN, "stop render failed");
    MEDIA_LOG_I_SHORT("stop render success");
    return Status::OK;
}

int32_t AudioServerSinkPlugin::SetVolumeWithRamp(float targetVolume, int32_t duration)
{
    MEDIA_LOG_D_SHORT("SetVolumeWithRamp entered.");
    int32_t ret = 0;
    {
        if (audioRenderer_ == nullptr) {
            return 0;
        }
        if (duration == 0) {
            ret = audioRenderer_->SetVolume(targetVolume);
        } else {
            MEDIA_LOG_I_SHORT("plugin set vol AudioRenderer::SetVolumeWithRamp start");
            ret = audioRenderer_->SetVolumeWithRamp(targetVolume, duration);
            MEDIA_LOG_I_SHORT("plugin set vol AudioRenderer::SetVolumeWithRamp end");
        }
    }
    OHOS::Media::SleepInJob(duration + 40); // fade out sleep more 40 ms
    MEDIA_LOG_I_SHORT("SetVolumeWithRamp end.");
    return ret;
}

Status AudioServerSinkPlugin::GetParameter(std::shared_ptr<Meta> &meta)
{
    AudioStandard::AudioRendererParams params;
    meta->Set<Tag::MEDIA_BITRATE>(bitRate_);
    if (audioRenderer_ && audioRenderer_->GetParams(params) == AudioStandard::SUCCESS) {
        MEDIA_LOG_I_SHORT("get param with fmt " PUBLIC_LOG_D32 " sampleRate " PUBLIC_LOG_D32 " channel " PUBLIC_LOG_D32
                    " encode type " PUBLIC_LOG_D32,
                    params.sampleFormat, params.sampleRate, params.channelCount, params.encodingType);
        if (params.sampleRate != rendererParams_.sampleRate) {
            MEDIA_LOG_W("samplingRate has changed from " PUBLIC_LOG_U32 " to " PUBLIC_LOG_U32,
                        rendererParams_.sampleRate, params.sampleRate);
        }
        meta->Set<Tag::AUDIO_SAMPLE_RATE>(params.sampleRate);
        if (params.sampleFormat != rendererParams_.sampleFormat) {
            MEDIA_LOG_W("sampleFormat has changed from " PUBLIC_LOG_U32 " to " PUBLIC_LOG_U32,
                        rendererParams_.sampleFormat, params.sampleFormat);
        }
        for (const auto &item : g_aduFmtMap) {
            if (std::get<1>(item) == params.sampleFormat) {
                meta->Set<Tag::AUDIO_SAMPLE_FORMAT>(std::get<0>(item));
            }
        }
    }
    return Status::OK;
}

bool AudioServerSinkPlugin::AssignSampleRateIfSupported(uint32_t sampleRate)
{
    sampleRate_ = sampleRate;
    auto supportedSampleRateList = OHOS::AudioStandard::AudioRenderer::GetSupportedSamplingRates();
    FALSE_RETURN_V_MSG(!supportedSampleRateList.empty(), false, "GetSupportedSamplingRates fail");
    for (const auto &rate : supportedSampleRateList) {
        if (static_cast<uint32_t>(rate) == sampleRate) {
            rendererParams_.sampleRate = rate;
            MEDIA_LOG_D_SHORT("sampleRate: " PUBLIC_LOG_U32, rendererParams_.sampleRate);
            return true;
        }
    }
    MEDIA_LOG_E_SHORT("sample rate " PUBLIC_LOG_U32 "not supported", sampleRate);
    return false;
}

bool AudioServerSinkPlugin::AssignChannelNumIfSupported(uint32_t channelNum)
{
    auto supportedChannelsList = OHOS::AudioStandard::AudioRenderer::GetSupportedChannels();
    FALSE_RETURN_V_MSG(!supportedChannelsList.empty(), false, "GetSupportedChannels fail");
    for (const auto &channel : supportedChannelsList) {
        if (static_cast<uint32_t>(channel) == channelNum) {
            rendererParams_.channelCount = channel;
            MEDIA_LOG_D_SHORT("channelCount: " PUBLIC_LOG_U32, rendererParams_.channelCount);
            return true;
        }
    }
    MEDIA_LOG_E_SHORT("channel num " PUBLIC_LOG_U32 "not supported", channelNum);
    return false;
}

bool AudioServerSinkPlugin::AssignSampleFmtIfSupported(Plugins::AudioSampleFormat sampleFormat)
{
    MEDIA_LOG_I_SHORT("AssignSampleFmtIfSupported AudioSampleFormat " PUBLIC_LOG_D32, sampleFormat);
    const auto &item = std::find_if(g_aduFmtMap.begin(), g_aduFmtMap.end(), [&sampleFormat](const auto &tmp) -> bool {
        return std::get<0>(tmp) == sampleFormat;
    });
    auto stdFmt = std::get<1>(*item);
    if (stdFmt == OHOS::AudioStandard::AudioSampleFormat::INVALID_WIDTH) {
        if (std::get<2>(*item) == AV_SAMPLE_FMT_NONE) { // 2
            MEDIA_LOG_I_SHORT("AssignSampleFmtIfSupported AV_SAMPLE_FMT_NONE");
            fmtSupported_ = false;
        } else {
            MEDIA_LOG_I_SHORT("AssignSampleFmtIfSupported needReformat_");
            fmtSupported_ = true;
            needReformat_ = true;
            reSrcFfFmt_ = std::get<2>(*item); // 2
            rendererParams_.sampleFormat = reStdDestFmt_;
        }
    } else {
        auto supportedFmts = OHOS::AudioStandard::AudioRenderer::GetSupportedFormats();
        if (CppExt::AnyOf(supportedFmts.begin(), supportedFmts.end(),
                          [&stdFmt](const auto &tmp) -> bool { return tmp == stdFmt; })) {
            MEDIA_LOG_I_SHORT("AssignSampleFmtIfSupported needReformat_ false");
            fmtSupported_ = true;
            needReformat_ = false;
        } else {
            fmtSupported_ = false;
            needReformat_ = false;
        }
        rendererParams_.sampleFormat = stdFmt;
    }
    return fmtSupported_;
}

void AudioServerSinkPlugin::SetInterruptMode(AudioStandard::InterruptMode interruptMode)
{
    if (audioRenderer_) {
        audioRenderer_->SetInterruptMode(interruptMode);
    }
}

void AudioServerSinkPlugin::SetUpParamsSetterMap()
{
    SetUpMimeTypeSetter();
    SetUpSampleRateSetter();
    SetUpAudioOutputChannelsSetter();
    SetUpMediaBitRateSetter();
    SetUpAudioSampleFormatSetter();
    SetUpAudioOutputChannelLayoutSetter();
    SetUpAudioSamplePerFrameSetter();
    SetUpBitsPerCodedSampleSetter();
    SetUpMediaSeekableSetter();
    SetUpAppPidSetter();
    SetUpAppUidSetter();
    SetUpAudioRenderInfoSetter();
    SetUpAudioRenderSetFlagSetter();
    SetUpAudioInterruptModeSetter();
}

void AudioServerSinkPlugin::SetUpMimeTypeSetter()
{
    paramsSetterMap_[Tag::MIME_TYPE] = [this](const ValueType &para) {
        FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<std::string>(para), Status::ERROR_MISMATCHED_TYPE,
                             "mimeType type should be string");
        mimeType_ = AnyCast<std::string>(para);
        MEDIA_LOG_I_SHORT("Set mimeType: " PUBLIC_LOG_S, mimeType_.c_str());
        return Status::OK;
    };
}

void AudioServerSinkPlugin::SetUpSampleRateSetter()
{
    paramsSetterMap_[Tag::AUDIO_SAMPLE_RATE] = [this](const ValueType &para) {
        FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<int32_t>(para), Status::ERROR_MISMATCHED_TYPE,
                             "sample rate type should be int32_t");
        if (!AssignSampleRateIfSupported(AnyCast<int32_t>(para))) {
            MEDIA_LOG_E_SHORT("sampleRate isn't supported");
            playerEventReceiver_->OnEvent({"sampleRate isn't supported",
                EventType::EVENT_ERROR, MSERR_UNSUPPORT_AUD_SAMPLE_RATE});
            return Status::ERROR_INVALID_PARAMETER;
        }
        return Status::OK;
    };
}

void AudioServerSinkPlugin::SetUpAudioOutputChannelsSetter()
{
    paramsSetterMap_[Tag::AUDIO_OUTPUT_CHANNELS] = [this](const ValueType &para) {
        FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<int32_t>(para), Status::ERROR_MISMATCHED_TYPE,
                             "channels type should be int32_t");
        channels_ = AnyCast<uint32_t>(para);
        MEDIA_LOG_I_SHORT("Set outputChannels: " PUBLIC_LOG_U32, channels_);
        if (!AssignChannelNumIfSupported(channels_)) {
            MEDIA_LOG_E_SHORT("channel isn't supported");
            playerEventReceiver_->OnEvent({"channel isn't supported",
                EventType::EVENT_ERROR, MSERR_UNSUPPORT_AUD_CHANNEL_NUM});
            return Status::ERROR_INVALID_PARAMETER;
        }
        return Status::OK;
    };
}

void AudioServerSinkPlugin::SetUpMediaBitRateSetter()
{
    paramsSetterMap_[Tag::MEDIA_BITRATE] = [this](const ValueType &para) {
        FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<int64_t>(para), Status::ERROR_MISMATCHED_TYPE,
                             "bit rate type should be int64_t");
        bitRate_ = AnyCast<int64_t>(para);
        return Status::OK;
    };
}
void AudioServerSinkPlugin::SetUpAudioSampleFormatSetter()
{
    paramsSetterMap_[Tag::AUDIO_SAMPLE_FORMAT] = [this](const ValueType &para) {
        MEDIA_LOG_I_SHORT("SetUpAudioSampleFormat");
        FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<AudioSampleFormat>(para), Status::ERROR_MISMATCHED_TYPE,
                             "AudioSampleFormat type should be AudioSampleFormat");
        if (!AssignSampleFmtIfSupported(AnyCast<AudioSampleFormat>(para))) {
            MEDIA_LOG_E_SHORT("sampleFmt isn't supported by audio renderer or resample lib");
            playerEventReceiver_->OnEvent({"sampleFmt isn't supported",
                EventType::EVENT_ERROR, MSERR_UNSUPPORT_AUD_PARAMS});
            return Status::ERROR_INVALID_PARAMETER;
        }
        return Status::OK;
    };
}
void AudioServerSinkPlugin::SetUpAudioOutputChannelLayoutSetter()
{
    paramsSetterMap_[Tag::AUDIO_OUTPUT_CHANNEL_LAYOUT] = [this](const ValueType &para) {
        FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<AudioChannelLayout>(para), Status::ERROR_MISMATCHED_TYPE,
                             "channel layout type should be AudioChannelLayout");
        channelLayout_ = AnyCast<AudioChannelLayout>(para);
        MEDIA_LOG_I_SHORT("Set outputChannelLayout: " PUBLIC_LOG_U64, channelLayout_);
        return Status::OK;
    };
}
void AudioServerSinkPlugin::SetUpAudioSamplePerFrameSetter()
{
    paramsSetterMap_[Tag::AUDIO_SAMPLE_PER_FRAME] = [this](const ValueType &para) {
        FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<int32_t>(para), Status::ERROR_MISMATCHED_TYPE,
                             "SAMPLE_PER_FRAME type should be int32_t");
        samplesPerFrame_ = AnyCast<uint32_t>(para);
        return Status::OK;
    };
}
void AudioServerSinkPlugin::SetUpBitsPerCodedSampleSetter()
{
    paramsSetterMap_[Tag::AUDIO_BITS_PER_CODED_SAMPLE] = [this](const ValueType &para) {
        FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<int32_t>(para), Status::ERROR_MISMATCHED_TYPE,
                             "BITS_PER_CODED_SAMPLE type should be int32_t");
        bitsPerSample_ = AnyCast<uint32_t>(para);
        return Status::OK;
    };
}
void AudioServerSinkPlugin::SetUpMediaSeekableSetter()
{
    paramsSetterMap_[Tag::MEDIA_SEEKABLE] = [this](const ValueType &para) {
        FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<Seekable>(para), Status::ERROR_MISMATCHED_TYPE,
                             "MEDIA_SEEKABLE type should be Seekable");
        seekable_ = AnyCast<Plugins::Seekable>(para);
        return Status::OK;
    };
}
void AudioServerSinkPlugin::SetUpAppPidSetter()
{
    paramsSetterMap_[Tag::APP_PID] = [this](const ValueType &para) {
        FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<int32_t>(para), Status::ERROR_MISMATCHED_TYPE,
                             "APP_PID type should be int32_t");
        appPid_ = AnyCast<int32_t>(para);
        return Status::OK;
    };
}
void AudioServerSinkPlugin::SetUpAppUidSetter()
{
    paramsSetterMap_[Tag::APP_UID] = [this](const ValueType &para) {
        FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<int32_t>(para), Status::ERROR_MISMATCHED_TYPE,
                             "APP_UID type should be int32_t");
        appUid_ = AnyCast<int32_t>(para);
        return Status::OK;
    };
}
void AudioServerSinkPlugin::SetUpAudioRenderInfoSetter()
{
    paramsSetterMap_[Tag::AUDIO_RENDER_INFO] = [this](const ValueType &para) {
        FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<AudioRenderInfo>(para), Status::ERROR_MISMATCHED_TYPE,
                             "AUDIO_RENDER_INFO type should be AudioRenderInfo");
        audioRenderInfo_ = AnyCast<AudioRenderInfo>(para);
        return Status::OK;
    };
}

void AudioServerSinkPlugin::SetUpAudioRenderSetFlagSetter()
{
    paramsSetterMap_[Tag::AUDIO_RENDER_SET_FLAG] = [this](const ValueType &para) {
        if (!Any::IsSameTypeWith<bool>(para)) {
            MEDIA_LOG_E_SHORT("AUDIO_RENDER_SET_FLAG type should be bool");
            playerEventReceiver_->OnEvent({"AUDIO_RENDER_SET_FLAG type is not bool",
                EventType::EVENT_ERROR, MSERR_EXT_API9_INVALID_PARAMETER});
            return Status::ERROR_MISMATCHED_TYPE;
        }
        audioRenderSetFlag_ = AnyCast<bool>(para);
        return Status::OK;
    };
}

void AudioServerSinkPlugin::SetUpAudioInterruptModeSetter()
{
    paramsSetterMap_[Tag::AUDIO_INTERRUPT_MODE] = [this](const ValueType &para) {
        FALSE_RETURN_V_MSG_E(Any::IsSameTypeWith<int32_t>(para), Status::ERROR_MISMATCHED_TYPE,
            "AUDIO_INTERRUPT_MODE type should be int32_t");
        AudioInterruptMode2InterruptMode(AnyCast<AudioInterruptMode>(para), audioInterruptMode_);
        SetInterruptMode(audioInterruptMode_);
        return Status::OK;
    };
}

Status AudioServerSinkPlugin::SetParameter(const std::shared_ptr<Meta> &meta)
{
    for (MapIt iter = meta->begin(); iter != meta->end(); iter++) {
        MEDIA_LOG_D_SHORT("SetParameter iter->first " PUBLIC_LOG_S, iter->first.c_str());
        if (paramsSetterMap_.find(iter->first) == paramsSetterMap_.end()) {
            continue;
        }
        std::function<Status(const ValueType &para)> paramSetter = paramsSetterMap_[iter->first];
        paramSetter(iter->second);
    }
    return Status::OK;
}

Status AudioServerSinkPlugin::GetVolume(float &volume)
{
    MEDIA_LOG_I_SHORT("GetVolume entered.");
    if (audioRenderer_ != nullptr) {
        volume = audioRenderer_->GetVolume();
        return Status::OK;
    }
    return Status::ERROR_WRONG_STATE;
}

Status AudioServerSinkPlugin::SetVolume(float volume)
{
    MEDIA_LOG_D("SetVolume entered.");
    if (audioRenderer_ != nullptr) {
        int32_t ret = audioRenderer_->SetVolume(volume);
        FALSE_RETURN_V_MSG_E(ret == OHOS::AudioStandard::SUCCESS, Status::ERROR_UNKNOWN,
            "set volume failed with code " PUBLIC_LOG_D32, ret);
        MEDIA_LOG_D("SetVolume succ");
        audioRendererVolume_ = volume;
        return Status::OK;
    }
    return Status::ERROR_WRONG_STATE;
}

Status AudioServerSinkPlugin::GetAudioEffectMode(int32_t &effectMode)
{
    MEDIA_LOG_I_SHORT("GetAudioEffectMode entered.");
    if (audioRenderer_ != nullptr) {
        effectMode = audioRenderer_->GetAudioEffectMode();
        MEDIA_LOG_I_SHORT("GetAudioEffectMode %{public}d", effectMode);
        return Status::OK;
    }
    return Status::ERROR_WRONG_STATE;
}

Status AudioServerSinkPlugin::SetAudioEffectMode(int32_t effectMode)
{
    MEDIA_LOG_I_SHORT("SetAudioEffectMode %{public}d", effectMode);
    if (audioRenderer_ != nullptr) {
        int32_t ret = audioRenderer_->SetAudioEffectMode(static_cast<OHOS::AudioStandard::AudioEffectMode>(effectMode));
        FALSE_RETURN_V_MSG(ret == OHOS::AudioStandard::SUCCESS, Status::ERROR_UNKNOWN,
            "set AudioEffectMode failed with code " PUBLIC_LOG_D32, ret);
        return Status::OK;
    }
    return Status::ERROR_WRONG_STATE;
}

Status AudioServerSinkPlugin::GetSpeed(float &speed)
{
    MEDIA_LOG_I_SHORT("GetSpeed entered.");
    if (audioRenderer_ != nullptr) {
        speed = audioRenderer_->GetSpeed();
        return Status::OK;
    }
    return Status::ERROR_WRONG_STATE;
}

Status AudioServerSinkPlugin::SetSpeed(float speed)
{
    MEDIA_LOG_I_SHORT("SetSpeed entered.");
    if (audioRenderer_ != nullptr) {
        int32_t ret = audioRenderer_->SetSpeed(speed);
        FALSE_RETURN_V_MSG(ret == OHOS::AudioStandard::SUCCESS, Status::ERROR_UNKNOWN,
            "set speed failed with code " PUBLIC_LOG_D32, ret);
        return Status::OK;
    }
    return Status::ERROR_WRONG_STATE;
}

Status AudioServerSinkPlugin::Resume()
{
    MediaAVCodec::AVCodecTrace trace("AudioServerSinkPlugin::Resume");
    MEDIA_LOG_D_SHORT("Resume entered");
    return Start();
}

Status AudioServerSinkPlugin::Pause()
{
    MediaAVCodec::AVCodecTrace trace("AudioServerSinkPlugin::Pause");
    MEDIA_LOG_I_SHORT("Pause entered");
    FALSE_RETURN_V_MSG(audioRenderer_ != nullptr, Status::ERROR_UNKNOWN, "audio renderer pause fail");
    FALSE_RETURN_V_MSG(audioRenderer_->GetStatus() == OHOS::AudioStandard::RENDERER_RUNNING,
        Status::OK, "audio renderer no need pause");
    sliceCount_++;
    FALSE_RETURN_V_MSG_W(audioRenderer_->Pause(), Status::ERROR_UNKNOWN, "renderer pause fail.");
    MEDIA_LOG_I_SHORT("audio renderer pause success");
    return Status::OK;
}

Status AudioServerSinkPlugin::PauseTransitent()
{
    MediaAVCodec::AVCodecTrace trace("AudioServerSinkPlugin::PauseTransitent");
    MEDIA_LOG_I_SHORT("PauseTransitent entered.");
    OHOS::Media::AutoLock lock(renderMutex_);
    FALSE_RETURN_V_MSG(audioRenderer_ != nullptr, Status::ERROR_UNKNOWN, "audio renderer pauseTransitent fail");
    FALSE_RETURN_V_MSG(audioRenderer_->GetStatus() == OHOS::AudioStandard::RENDERER_RUNNING,
        Status::OK, "audio renderer no need pauseTransitent");
    sliceCount_++;
    FALSE_RETURN_V_MSG_W(audioRenderer_->PauseTransitent(), Status::ERROR_UNKNOWN, "renderer pauseTransitent fail.");
    MEDIA_LOG_I_SHORT("audio renderer pauseTransitent success");
    return Status::OK;
}

Status AudioServerSinkPlugin::GetLatency(uint64_t &hstTime)
{
    FALSE_RETURN_V(audioRenderer_ != nullptr, Status::ERROR_NULL_POINTER);
    audioRenderer_->GetLatency(hstTime); // audioRender->getLatency lack accuracy
    return Status::OK;
}

Status AudioServerSinkPlugin::DrainCacheData(bool render)
{
    MediaAVCodec::AVCodecTrace trace("AudioServerSinkPlugin::DrainCacheData " + std::to_string(render));
    if (!render) {
        MEDIA_LOG_D("Drop cache buffer because render=false");
        cachedBuffers_.clear();
        return Status::OK;
    }
    if (cachedBuffers_.empty()) {
        return Status::OK;
    }
    AudioStandard::RendererState rendererState = (audioRenderer_ != nullptr) ?
        audioRenderer_->GetStatus() : AudioStandard::RendererState::RENDERER_INVALID;
    FALSE_RETURN_V_MSG(rendererState != AudioStandard::RendererState::RENDERER_PAUSED,
        Status::ERROR_AGAIN, "audioRenderer_ is still paused, try again later");
    if (rendererState != AudioStandard::RendererState::RENDERER_RUNNING) {
        cachedBuffers_.clear();
        MEDIA_LOG_W("Drop cache buffer because audioRenderer_ state invalid");
        return Status::ERROR_UNKNOWN;
    }
    while (cachedBuffers_.size() > 0) { // do drain cache buffers
        auto currBuffer = cachedBuffers_.front();
        uint8_t* destBuffer = currBuffer.data();
        size_t destLength = currBuffer.size();
        bool shouldDrop = false;
        size_t remained = WriteAudioBuffer(destBuffer, destLength, shouldDrop);
        if (remained == 0) { // write ok
            cachedBuffers_.pop_front();
            continue;
        }
        if (shouldDrop) { // write error and drop buffer
            cachedBuffers_.clear();
            MEDIA_LOG_W("Drop cache buffer, error happens during drain");
            return Status::ERROR_UNKNOWN;
        }
        if (remained < destLength) { // some data written, then audioRender paused again, update cache
            std::vector<uint8_t> tmpBuffer(destBuffer + destLength - remained, destBuffer + destLength);
            cachedBuffers_.pop_front();
            cachedBuffers_.emplace_front(std::move(tmpBuffer));
        } // else no data written, no need to update front cache
        MEDIA_LOG_W("Audiorender pause again during drain cache buffers");
        return Status::ERROR_AGAIN;
    }
    MEDIA_LOG_I("Drain cache buffer success");
    return Status::OK;
}

void AudioServerSinkPlugin::CacheData(uint8_t* inputBuffer, size_t bufferSize)
{
    MediaAVCodec::AVCodecTrace trace("AudioServerSinkPlugin::CacheData " + std::to_string(bufferSize));
    std::vector<uint8_t> tmpBuffer(inputBuffer, inputBuffer + bufferSize);
    cachedBuffers_.emplace_back(std::move(tmpBuffer));
    MEDIA_LOG_I("Cache one audio buffer, data size is " PUBLIC_LOG_U64, bufferSize);
    while (cachedBuffers_.size() > DEFAULT_BUFFER_NUM) {
        auto dropSize = cachedBuffers_.front().size();
        MEDIA_LOG_W("Drop one cached buffer size " PUBLIC_LOG_U64 " because max cache size reached.", dropSize);
        cachedBuffers_.pop_front();
    }
}

size_t AudioServerSinkPlugin::WriteAudioBuffer(uint8_t* inputBuffer, size_t bufferSize, bool& shouldDrop)
{
    uint8_t* destBuffer = inputBuffer;
    size_t destLength = bufferSize;
    while (destLength > 0) {
        MediaAVCodec::AVCodecTrace trace("AudioServerSinkPlugin::WriteAudioBuffer: " + std::to_string(destLength));
        int32_t ret = audioRenderer_->Write(destBuffer, destLength);
        if (ret < 0) {
            if (audioRenderer_->GetStatus() == AudioStandard::RendererState::RENDERER_PAUSED) {
                MEDIA_LOG_W("WriteAudioBuffer error because audioRenderer_ paused, cache data.");
                shouldDrop = false;
            } else {
                MEDIA_LOG_W("WriteAudioBuffer error because audioRenderer_ error, drop data.");
                shouldDrop = true;
            }
            break;
        } else if (static_cast<size_t>(ret) < destLength) {
            OHOS::Media::SleepInJob(5); // 5ms
        }
        if (static_cast<size_t>(ret) > destLength) {
            MEDIA_LOG_W("audioRenderer_ return ret " PUBLIC_LOG_D32 "> destLength " PUBLIC_LOG_U64,
                ret, destLength);
            ret = static_cast<int32_t>(destLength);
        }
        destBuffer += ret;
        destLength -= static_cast<size_t>(ret);
        MEDIA_LOG_D("Written data size " PUBLIC_LOG_D32 ", bufferSize " PUBLIC_LOG_U64, ret, bufferSize);
    }
    return destLength;
}

Status AudioServerSinkPlugin::Write(const std::shared_ptr<OHOS::Media::AVBuffer> &inputBuffer)
{
    MEDIA_LOG_D_SHORT("Write buffer to audio framework");
    FALSE_RETURN_V_MSG_W(inputBuffer != nullptr && inputBuffer->memory_->GetSize() != 0, Status::OK,
                         "Receive empty buffer."); // return ok
    MediaAVCodec::AVCodecTrace trace("AudioServerSinkPlugin::Write, bufferSize: "
        + std::to_string(inputBuffer->memory_->GetSize()));
    if (mimeType_ == MimeType::AUDIO_AVS3DA) {
        return WriteAudioVivid(inputBuffer) >= 0 ? Status::OK : Status::ERROR_UNKNOWN;
    }
    auto mem = inputBuffer->memory_;
    auto srcBuffer = mem->GetAddr();
    auto destBuffer = const_cast<uint8_t *>(srcBuffer);
    auto srcLength = mem->GetSize();
    size_t destLength = static_cast<size_t>(srcLength);
    while (isForcePaused_ && seekable_ == Seekable::SEEKABLE) {
        OHOS::Media::SleepInJob(5); // 5ms
    }
    if (audioRenderer_ == nullptr) {
        DrainCacheData(false);
        return Status::ERROR_NULL_POINTER;
    }
    auto drainCacheRet = DrainCacheData(true);
    if (drainCacheRet != Status::OK) {
        if (drainCacheRet == Status::ERROR_AGAIN) {
            CacheData(destBuffer, destLength);
        }
        return Status::ERROR_UNKNOWN;
    }
    bool shouldDrop = false;
    size_t remained = WriteAudioBuffer(destBuffer, destLength, shouldDrop);
    if (remained == 0) {
        return Status::OK;
    }
    if (!shouldDrop) {
        CacheData(destBuffer + destLength - remained, remained);
    }
    return Status::ERROR_UNKNOWN;
}

int32_t AudioServerSinkPlugin::WriteAudioVivid(const std::shared_ptr<OHOS::Media::AVBuffer> &inputBuffer)
{
    MediaAVCodec::AVCodecTrace trace("AudioServerSinkPlugin::WriteAudioVivid");
    auto mem = inputBuffer->memory_;
    auto pcmBuffer = mem->GetAddr();
    auto pcmBufferSize = mem->GetSize();
    auto meta = inputBuffer->meta_;
    std::vector<uint8_t> metaData;
    meta->GetData(Tag::OH_MD_KEY_AUDIO_VIVID_METADATA, metaData);
    FALSE_RETURN_V(audioRenderer_ != nullptr, -1);
    return audioRenderer_->Write(pcmBuffer, pcmBufferSize, metaData.data(), metaData.size());
}

Status AudioServerSinkPlugin::Flush()
{
    MEDIA_LOG_I_SHORT("Flush entered.");
    DrainCacheData(false);
    if (audioRenderer_ == nullptr) {
        return Status::ERROR_WRONG_STATE;
    }
    if (audioRenderer_->Flush()) {
        MEDIA_LOG_D_SHORT("flush renderer success");
        return Status::OK;
    }
    MEDIA_LOG_E_SHORT("flush renderer fail");
    return Status::ERROR_UNKNOWN;
}

Status AudioServerSinkPlugin::Drain()
{
    MEDIA_LOG_I_SHORT("Drain entered.");
    if (audioRenderer_ == nullptr) {
        DrainCacheData(false);
        return Status::ERROR_WRONG_STATE;
    }
    DrainCacheData(true); // try to drain
    cachedBuffers_.clear(); // force clear cached data, no matter drain success or not
    if (!audioRenderer_->Drain()) {
        uint64_t latency = 0;
        audioRenderer_->GetLatency(latency);
        latency /= 1000;    // 1000 cast into ms
        if (latency > 50) { // 50 latency too large
            MEDIA_LOG_W("Drain failed and latency is too large, will sleep " PUBLIC_LOG_U64 " ms, aka. latency.",
                        latency);
            OHOS::Media::SleepInJob(latency);
        }
    }
    MEDIA_LOG_I_SHORT("drain renderer success");
    return Status::OK;
}

int64_t AudioServerSinkPlugin::GetPlayedOutDurationUs(int64_t nowUs)
{
    FALSE_RETURN_V(audioRenderer_ != nullptr && rendererParams_.sampleRate != 0, -1);
    uint32_t numFramesPlayed = 0;
    AudioStandard::Timestamp ts;
    bool res = audioRenderer_->GetAudioTime(ts, AudioStandard::Timestamp::Timestampbase::MONOTONIC);
    if (res) {
        numFramesPlayed = ts.framePosition;
    }
    return numFramesPlayed;
}

Status AudioServerSinkPlugin::GetFramePosition(int32_t &framePosition)
{
    AudioStandard::Timestamp ts;
    if (audioRenderer_ == nullptr) {
        return Status::ERROR_WRONG_STATE;
    }
    bool res = audioRenderer_->GetAudioTime(ts, AudioStandard::Timestamp::Timestampbase::MONOTONIC);
    if (!res) {
        return Status::ERROR_UNKNOWN;
    }
    framePosition = static_cast<int32_t>(ts.framePosition);
    return Status::OK;
}

void AudioServerSinkPlugin::SetEventReceiver(const std::shared_ptr<Pipeline::EventReceiver>& receiver)
{
    FALSE_RETURN(receiver != nullptr);
    playerEventReceiver_ = receiver;
}

void AudioServerSinkPlugin::SetAudioDumpBySysParam()
{
    std::string dumpAllEnable;
    enableEntireDump_ = false;
    int32_t dumpAllRes = OHOS::system::GetStringParameter("sys.media.sink.entiredump.enable", dumpAllEnable, "");
    if (dumpAllRes == 0 && !dumpAllEnable.empty() && dumpAllEnable == "true") {
        enableEntireDump_ = true;
    }
    std::string dumpSliceEnable;
    enableDumpSlice_ = false;
    int32_t sliceDumpRes = OHOS::system::GetStringParameter("sys.media.sink.slicedump.enable", dumpSliceEnable, "");
    if (sliceDumpRes == 0 && !dumpSliceEnable.empty() && dumpSliceEnable == "true") {
        enableDumpSlice_ = true;
    }
    MEDIA_LOG_D_SHORT("sys.media.sink.entiredump.enable: " PUBLIC_LOG_S ", sys.media.sink.slicedump.enable: "
        PUBLIC_LOG_S, dumpAllEnable.c_str(), dumpSliceEnable.c_str());
}

void AudioServerSinkPlugin::DumpEntireAudioBuffer(uint8_t* buffer, const size_t& bytesSingle)
{
    if (!enableEntireDump_) {
        return;
    }

    if (entireDumpFile_ == nullptr) {
        std::string path = "data/media/audio-sink-entire.pcm";
        entireDumpFile_ = fopen(path.c_str(), "wb+");
    }
    if (entireDumpFile_ == nullptr) {
        return;
    }
    (void)fwrite(buffer, bytesSingle, 1, entireDumpFile_);
    (void)fflush(entireDumpFile_);
}

void AudioServerSinkPlugin::DumpSliceAudioBuffer(uint8_t* buffer, const size_t& bytesSingle)
{
    if (!enableDumpSlice_) {
        return;
    }

    if (curCount_ != sliceCount_) {
        curCount_ = sliceCount_;
        if (sliceDumpFile_ != nullptr) {
            (void)fclose(sliceDumpFile_);
        }
        std::string path = "data/media/audio-sink-slice-" + std::to_string(sliceCount_) + ".pcm";
        sliceDumpFile_ = fopen(path.c_str(), "wb+");
    }
    if (sliceDumpFile_ == nullptr) {
        return;
    }
    (void)fwrite(buffer, bytesSingle, 1, sliceDumpFile_);
    (void)fflush(sliceDumpFile_);
}

Status AudioServerSinkPlugin::SetMuted(bool isMuted)
{
    FALSE_RETURN_V(audioRenderer_ != nullptr, Status::ERROR_NULL_POINTER);
    MEDIA_LOG_D("SetMuted in");
    audioRenderer_->SetSilentModeAndMixWithOthers(isMuted);
    MEDIA_LOG_I("SetMuted out");
    return Status::OK;
}
} // namespace Plugin
} // namespace Media
} // namespace OHOS
