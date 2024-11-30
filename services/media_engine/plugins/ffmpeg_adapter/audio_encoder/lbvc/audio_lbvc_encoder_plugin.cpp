/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "audio_lbvc_encoder_plugin.h"
#include "avcodec_codec_name.h"
#include "avcodec_log.h"

namespace {
using namespace OHOS::Media;
using namespace OHOS::Media::Plugins;
using namespace Lbvc;
using namespace Hdi;
using namespace OHOS::HDI::Codec::V3_0;

constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN_AUDIO, "AvCodec-AudioLbvcEncoderPlugin"};
const std::string LBVC_ENCODER_COMPONENT_NAME = "OMX.audio.encoder.lbvc";
constexpr uint32_t BUFFER_FLAG_EOS = 1;
constexpr AudioSampleFormat SUPPORT_SAMPLE_FORMAT = AudioSampleFormat::SAMPLE_S16LE;
constexpr int32_t SUPPORT_CHANNELS = 1;
constexpr int32_t SUPPORT_SAMPLE_RATE = 16000;
constexpr uint32_t INPUT_BUFFER_SIZE_DEFAULT = 640;
constexpr uint32_t OUTPUT_BUFFER_SIZE_DEFAULT = 640;
constexpr int64_t SUPPORT_BITRATE = 6000;
constexpr uint32_t OMX_AUDIO_CODEC_PARAM_INDEX = 0x6F000000 + 0x00A0000B;

Status RegisterAudioEncoderPlugins(const std::shared_ptr<Register>& reg)
{
    CodecPluginDef definition;
    definition.name = std::string(OHOS::MediaAVCodec::AVCodecCodecName::AUDIO_ENCODER_LBVC_NAME);
    definition.pluginType = PluginType::AUDIO_ENCODER;
    definition.rank = 100;  // 100
    definition.SetCreator([](const std::string& name) -> std::shared_ptr<CodecPlugin> {
        return std::make_shared<AudioLbvcEncoderPlugin>(name);
    });

    Capability cap;
    cap.SetMime(MimeType::AUDIO_LBVC);
    cap.AppendFixedKey<CodecMode>(Tag::MEDIA_CODEC_MODE, CodecMode::SOFTWARE);

    definition.AddInCaps(cap);
    // do not delete the codec in the deleter
    if (reg->AddPlugin(definition) != Status::OK) {
        AVCODEC_LOGE("AudioLbvcEncoderPlugin Register Failure");
        return Status::ERROR_UNKNOWN;
    }

    return Status::OK;
}

void UnRegisterAudioEncoderPlugin() {}

PLUGIN_DEFINITION(LbvcAudioEncoder, LicenseType::VENDOR, RegisterAudioEncoderPlugins,
    UnRegisterAudioEncoderPlugin);
} // namespace

namespace OHOS {
namespace Media {
namespace Plugins {
namespace Lbvc {
AudioLbvcEncoderPlugin::AudioLbvcEncoderPlugin(const std::string &name)
    : CodecPlugin(std::move(name)),
      audioSampleFormat_(AudioSampleFormat::INVALID_WIDTH),
      channels_(0),
      sampleRate_(0),
      bitRate_(0),
      maxInputSize_(0),
      maxOutputSize_(0),
      eosFlag_(false),
      dataCallback_(nullptr),
      hdiCodec_(std::make_shared<HdiCodec>())
{
    AVCODEC_LOGI("AudioLbvcEncoderPlugin init");
}

AudioLbvcEncoderPlugin::~AudioLbvcEncoderPlugin()
{
    hdiCodec_.reset();
    hdiCodec_ = nullptr;
}

Status AudioLbvcEncoderPlugin::Init()
{
    Status ret = hdiCodec_->InitComponent(LBVC_ENCODER_COMPONENT_NAME);
    return ret;
}

Status AudioLbvcEncoderPlugin::Prepare()
{
    Status ret = hdiCodec_->InitBuffers(maxInputSize_);
    return ret;
}

Status AudioLbvcEncoderPlugin::Reset()
{
    return hdiCodec_->Reset();
}

Status AudioLbvcEncoderPlugin::Start()
{
    Status ret = hdiCodec_->SendCommand(CODEC_COMMAND_STATE_SET, CODEC_STATE_EXECUTING);
    return ret;
}

Status AudioLbvcEncoderPlugin::Stop()
{
    Status ret = hdiCodec_->SendCommand(CODEC_COMMAND_STATE_SET, CODEC_STATE_IDLE);
    return ret;
}

Status AudioLbvcEncoderPlugin::SetParameter(const std::shared_ptr<Meta> &parameter)
{
    Status ret = GetMetaData(parameter);
    if (ret != Status::OK) {
        AVCODEC_LOGE("GetMetaData Fail");
        return ret;
    }

    if (!CheckFormat()) {
        AVCODEC_LOGE("CheckFormat Fail");
        return Status::ERROR_INVALID_PARAMETER;
    }

    AudioCodecOmxParam param;
    hdiCodec_->InitParameter(param);
    param.sampleRate = static_cast<uint32_t>(sampleRate_);
    param.sampleFormat = static_cast<uint32_t>(audioSampleFormat_);
    param.channels = static_cast<uint32_t>(channels_);
    param.bitRate = static_cast<uint32_t>(bitRate_);
    maxInputSize_ = INPUT_BUFFER_SIZE_DEFAULT;
    maxOutputSize_ = OUTPUT_BUFFER_SIZE_DEFAULT;

    int8_t *p = reinterpret_cast<int8_t *>(&param);
    std::vector<int8_t> paramVec(p, p + sizeof(AudioCodecOmxParam));
    ret = hdiCodec_->SetParameter(OMX_AUDIO_CODEC_PARAM_INDEX, paramVec);
    if (ret != Status::OK) {
        AVCODEC_LOGE("SetParameter failed!");
    }

    ret = Prepare();
    if (ret != Status::OK) {
        AVCODEC_LOGE("Prepare failed!");
    }
    return Status::OK;
}

Status AudioLbvcEncoderPlugin::GetParameter(std::shared_ptr<Meta> &parameter)
{
    AudioCodecOmxParam param;
    hdiCodec_->InitParameter(param);
    Status ret = hdiCodec_->GetParameter(OMX_AUDIO_CODEC_PARAM_INDEX, param);
    if (ret != Status::OK) {
        AVCODEC_LOGE("GetParameter failed!");
        return ret;
    }
    
    parameter->Set<Tag::AUDIO_CHANNEL_COUNT>(static_cast<int32_t>(param.channels));
    parameter->Set<Tag::AUDIO_SAMPLE_RATE>(static_cast<int32_t>(param.sampleRate));
    parameter->Set<Tag::AUDIO_SAMPLE_FORMAT>(static_cast<AudioSampleFormat>(param.sampleFormat));
    parameter->Set<Tag::MEDIA_BITRATE>(static_cast<int64_t>(param.bitRate));
    parameter->Set<Tag::AUDIO_MAX_INPUT_SIZE>(static_cast<int32_t>(maxInputSize_));
    parameter->Set<Tag::AUDIO_MAX_OUTPUT_SIZE>(static_cast<int32_t>(maxOutputSize_));
    return Status::OK;
}

Status AudioLbvcEncoderPlugin::QueueInputBuffer(const std::shared_ptr<AVBuffer> &inputBuffer)
{
    if (inputBuffer->flag_ == BUFFER_FLAG_EOS) {
        AVCODEC_LOGI("QueueInputBuffer Eos!");
        eosFlag_ = true;
        return Status::OK;
    }

    Status ret = hdiCodec_->EmptyThisBuffer(inputBuffer);
    if (ret != Status::OK) {
        AVCODEC_LOGE("EmptyThisBuffer failed!");
        return ret;
    }

    dataCallback_->OnInputBufferDone(inputBuffer);
    AVCODEC_LOGD("OnInputBufferDone");
    return Status::OK;
}

Status AudioLbvcEncoderPlugin::QueueOutputBuffer(std::shared_ptr<AVBuffer> &outputBuffer)
{
    if (eosFlag_ == true) {
        AVCODEC_LOGI("QueueOutputBuffer Eos!");
        outputBuffer->flag_ = BUFFER_FLAG_EOS;
        outputBuffer->memory_->SetSize(0);
        eosFlag_ = false;
        dataCallback_->OnOutputBufferDone(outputBuffer);
        return Status::OK;
    }

    Status ret = hdiCodec_->FillThisBuffer(outputBuffer);
    if (ret != Status::OK) {
        AVCODEC_LOGE("FillThisBuffer failed!");
        return ret;
    }

    dataCallback_->OnOutputBufferDone(outputBuffer);
    AVCODEC_LOGD("OnOutputBufferDone");
    return Status::OK;
}

Status AudioLbvcEncoderPlugin::GetInputBuffers(std::vector<std::shared_ptr<AVBuffer>> &inputBuffers)
{
    return Status::OK;
}

Status AudioLbvcEncoderPlugin::GetOutputBuffers(std::vector<std::shared_ptr<AVBuffer>> &outputBuffers)
{
    return Status::OK;
}

Status AudioLbvcEncoderPlugin::Flush()
{
    Status ret = hdiCodec_->SendCommand(CODEC_COMMAND_FLUSH, CODEC_STATE_EXECUTING);
    return ret;
}

Status AudioLbvcEncoderPlugin::Release()
{
    hdiCodec_->Release();
    return Status::OK;
}

Status AudioLbvcEncoderPlugin::GetMetaData(const std::shared_ptr<Meta> &meta)
{
    if (!meta->Get<Tag::AUDIO_CHANNEL_COUNT>(channels_)) {
        AVCODEC_LOGE("AudioLbvcEncoderPlugin no AUDIO_CHANNEL_COUNT");
        return Status::ERROR_INVALID_PARAMETER;
    }
    if (!meta->Get<Tag::AUDIO_SAMPLE_RATE>(sampleRate_)) {
        AVCODEC_LOGE("AudioLbvcEncoderPlugin no AUDIO_SAMPLE_RATE");
        return Status::ERROR_INVALID_PARAMETER;
    }
    if (!meta->Get<Tag::AUDIO_SAMPLE_FORMAT>(audioSampleFormat_)) {
        AVCODEC_LOGE("AudioLbvcEncoderPlugin no AUDIO_SAMPLE_FORMAT");
        return Status::ERROR_INVALID_PARAMETER;
    }
    if (!meta->Get<Tag::MEDIA_BITRATE>(bitRate_)) {
        AVCODEC_LOGE("AudioLbvcEncoderPlugin no BITRATE");
        return Status::ERROR_INVALID_PARAMETER;
    }

    return Status::OK;
}

bool AudioLbvcEncoderPlugin::CheckFormat()
{
    if (channels_ != SUPPORT_CHANNELS) {
        AVCODEC_LOGE("AudioLbvcEncoderPlugin channels not supported");
        return false;
    }
    if (sampleRate_ != SUPPORT_SAMPLE_RATE) {
        AVCODEC_LOGE("AudioLbvcEncoderPlugin sampleRate not supported");
        return false;
    }
    if (bitRate_ != SUPPORT_BITRATE) {
        AVCODEC_LOGE("AudioLbvcEncoderPlugin bitrate not supported");
        return false;
    }
    if (audioSampleFormat_ != SUPPORT_SAMPLE_FORMAT) {
        AVCODEC_LOGE("AudioLbvcEncoderPlugin SampleFormat not supported");
        return false;
    }

    return true;
}
} // namespace Lbvc
} // namespace Plugins
} // namespace Media
} // namespace OHOS