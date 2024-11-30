/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "codec_param_checker.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "avcodec_trace.h"
#include "meta/meta_key.h"
#include "codec_ability_singleton.h"
#include "media_description.h"
#include "meta/meta_key.h"
#include "temporal_scalability.h"
#include "meta/video_types.h"
#include "surface_type.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN_FRAMEWORK, "CodecParamChecker"};
using namespace OHOS::Media;
using namespace OHOS::MediaAVCodec;

constexpr int32_t DEFAULT_QUALITY = 50;

const std::unordered_map<CodecScenario, std::string_view> CODEC_SCENARIO_TO_STRING = {
    {CodecScenario::CODEC_SCENARIO_ENC_NORMAL, "encoder normal"},
    {CodecScenario::CODEC_SCENARIO_ENC_TEMPORAL_SCALABILITY, "encoder temporal scalability"},
    {CodecScenario::CODEC_SCENARIO_DEC_NORMAL, "decoder normal"},
};

template<class T> void PrintParam(bool paramExist, const std::string_view tag, T value)
{
    if (!paramExist) {
        return;
    }
    using namespace std::string_literals;
    std::string logMsg = "Param "s + tag.data() + " set, value: "s + std::to_string(value);
    AVCODEC_LOGI("%{public}s", logMsg.c_str());
}

template<class T> void PrintParam(bool paramExist, const std::string_view tag, T value1, T value2)
{
    if (!paramExist) {
        return;
    }
    using namespace std::string_literals;
    std::string logMsg = "Param "s + tag.data() + " set, value: "s +
        std::to_string(value1) + " - " + std::to_string(value2);
    AVCODEC_LOGI("%{public}s", logMsg.c_str());
}

inline void PrintCodecScenario(CodecScenario scenario)
{
    auto scenarioName = CODEC_SCENARIO_TO_STRING.find(scenario);
    if (scenarioName != CODEC_SCENARIO_TO_STRING.end()) {
        AVCODEC_LOGI("Codec scenario is %{public}s", scenarioName->second.data());
    } else {
        AVCODEC_LOGI("Codec scenario is %{public}d", scenario);
    }
}

template<class T> bool IsSupported(std::vector<T> cap, T value)
{
    return std::find(cap.begin(), cap.end(), value) != cap.end();
}

// Video scenario checker
std::optional<CodecScenario> TemporalScalabilityChecker(CapabilityData &capData, const Format &format,
                                                        AVCodecType codecType);

// Video codec checker
int32_t ResolutionChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t PixelFormatChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t FramerateChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t BitrateAndQualityChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t VideoProfileChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t RotaitonChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t QPChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t TemporalGopSizeChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t TemporalGopReferenceModeChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t UniformlyScaledReferenceChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t ColorPrimariesChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t TransferCharacteristicsChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t MatrixCoefficientsChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t LTRFrameCountChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t ScalingModeChecker(CapabilityData &capData, Format &format, CodecScenario scenario);
int32_t PostProcessingChecker(CapabilityData &capData, Format &format, CodecScenario scenario);

// Checkers list define
using ScenarioCheckerType =
    std::optional<CodecScenario> (*)(CapabilityData &capData, const Format &format, AVCodecType codecType);
using ParamCheckerType = int32_t (*)(CapabilityData &capData, Format &format, CodecScenario scenario);
using ScenarioCheckerListType = std::vector<ScenarioCheckerType>;
using ParamCheckerListType = std::vector<ParamCheckerType>;
const ParamCheckerListType VIDEO_ENCODER_CONFIGURE_CHECKER_LIST = {
    ResolutionChecker,
    PixelFormatChecker,
    FramerateChecker,
    BitrateAndQualityChecker,
    VideoProfileChecker,
    QPChecker,
    ColorPrimariesChecker,
    TransferCharacteristicsChecker,
    MatrixCoefficientsChecker,
    LTRFrameCountChecker,
};

const ParamCheckerListType VIDEO_ENCODER_TEMPORAL_SCALABILITY_CONFIGURE_CHECKER_LIST = {
    ResolutionChecker,
    PixelFormatChecker,
    FramerateChecker,
    BitrateAndQualityChecker,
    VideoProfileChecker,
    QPChecker,
    TemporalGopSizeChecker,
    TemporalGopReferenceModeChecker,
    UniformlyScaledReferenceChecker,
    ColorPrimariesChecker,
    TransferCharacteristicsChecker,
    MatrixCoefficientsChecker,
    LTRFrameCountChecker,
};

const ParamCheckerListType VIDEO_DECODER_CONFIGURE_CHECKER_LIST = {
    ResolutionChecker,
    PixelFormatChecker,
    FramerateChecker,
    RotaitonChecker,
    ScalingModeChecker,
    PostProcessingChecker,
};

const ParamCheckerListType VIDEO_ENCODER_PARAMETER_CHECKER_LIST = {
    FramerateChecker,
    BitrateAndQualityChecker,
    QPChecker,
};

const ParamCheckerListType VIDEO_DECODER_PARAMETER_CHECKER_LIST = {};

const ScenarioCheckerListType VIDEO_SCENARIO_CHECKER_LIST = {
    TemporalScalabilityChecker,
};

const std::vector<std::string_view> FORMAT_MERGE_LIST = {
    MediaDescriptionKey::MD_KEY_BITRATE,
    MediaDescriptionKey::MD_KEY_QUALITY,
    MediaDescriptionKey::MD_KEY_FRAME_RATE,
    Tag::VIDEO_ENCODER_QP_MIN,
    Tag::VIDEO_ENCODER_QP_MAX,
};

// Checkers table
const std::unordered_map<CodecScenario, ParamCheckerListType> CONFIGURE_CHECKERS_TABLE = {
    {CodecScenario::CODEC_SCENARIO_ENC_NORMAL, VIDEO_ENCODER_CONFIGURE_CHECKER_LIST},
    {CodecScenario::CODEC_SCENARIO_ENC_TEMPORAL_SCALABILITY, VIDEO_ENCODER_TEMPORAL_SCALABILITY_CONFIGURE_CHECKER_LIST},
    {CodecScenario::CODEC_SCENARIO_DEC_NORMAL, VIDEO_DECODER_CONFIGURE_CHECKER_LIST},
};

const std::unordered_map<CodecScenario, ParamCheckerListType> PARAMETER_CHECKERS_TABLE = {
    {CodecScenario::CODEC_SCENARIO_ENC_NORMAL, VIDEO_ENCODER_PARAMETER_CHECKER_LIST},
    {CodecScenario::CODEC_SCENARIO_ENC_TEMPORAL_SCALABILITY, VIDEO_ENCODER_PARAMETER_CHECKER_LIST},
    {CodecScenario::CODEC_SCENARIO_DEC_NORMAL, VIDEO_DECODER_PARAMETER_CHECKER_LIST},
};

// Checkers implementation
std::optional<CodecScenario> TemporalScalabilityChecker(CapabilityData &capData, const Format &format,
                                                        AVCodecType codecType)
{
    int32_t enable = 0;
    std::optional<CodecScenario> scenario = std::nullopt;
    bool enableExist = format.GetIntValue(Tag::VIDEO_ENCODER_ENABLE_TEMPORAL_SCALABILITY, enable);
    bool temporalGopSizeExist = format.ContainKey(Tag::VIDEO_ENCODER_TEMPORAL_GOP_SIZE);
    bool modeExist = format.ContainKey(Tag::VIDEO_ENCODER_TEMPORAL_GOP_REFERENCE_MODE);
    PrintParam(enableExist, Tag::VIDEO_ENCODER_ENABLE_TEMPORAL_SCALABILITY, enable);

    if (codecType == AVCODEC_TYPE_VIDEO_DECODER) {
        if (enableExist || temporalGopSizeExist || modeExist) {
            AVCODEC_LOGW("Temporal scalability is only supported in video encoder!");
        }
        return scenario;
    }
    if (!enableExist || !enable) {
        if (temporalGopSizeExist || modeExist) {
            AVCODEC_LOGW("Please enable key VIDEO_ENCODER_ENABLE_TEMPORAL_SCALABILITY!");
        }
        return scenario;
    }
    CHECK_AND_RETURN_RET_LOGW(capData.featuresMap.count(
        static_cast<int32_t>(AVCapabilityFeature::VIDEO_ENCODER_TEMPORAL_SCALABILITY)),
        scenario, "Not support temporal scalability");

    scenario = CodecScenario::CODEC_SCENARIO_ENC_TEMPORAL_SCALABILITY;
    return scenario;
}

int32_t ResolutionChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)scenario;
    int32_t width = 0;
    int32_t height = 0;
    bool widthExist = format.GetIntValue(MediaDescriptionKey::MD_KEY_WIDTH, width);
    bool heightExist = format.GetIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, height);
    CHECK_AND_RETURN_RET_LOG(widthExist && heightExist, AVCS_ERR_INVALID_VAL, "Key param missing, width or height");
    PrintParam(widthExist && heightExist, "resolution", width, height);

    bool resolutionValid = true;
    if (capData.supportSwapWidthHeight) {
        AVCODEC_LOGI("Codec support swap width and height");
        resolutionValid = (capData.width.InRange(width) && capData.height.InRange(height)) ||
                          (capData.width.InRange(height) && capData.height.InRange(width));
    } else {
        resolutionValid = capData.width.InRange(width) && capData.height.InRange(height);
    }
    CHECK_AND_RETURN_RET_LOG(resolutionValid, AVCS_ERR_INVALID_VAL,
        "Param invalid, resolution: %{public}d*%{public}d, range: [%{public}d*%{public}d]-[%{public}d*%{public}d]",
        width, height, capData.width.minVal, capData.height.minVal, capData.width.maxVal, capData.height.maxVal);
    return AVCS_ERR_OK;
}

int32_t PixelFormatChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)scenario;
    int32_t pixelFormat;
    bool paramExist = format.GetIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, pixelFormat);
    if (!paramExist || pixelFormat == static_cast<int32_t>(VideoPixelFormat::SURFACE_FORMAT)) {
        return AVCS_ERR_OK;
    }
    PrintParam(paramExist, MediaDescriptionKey::MD_KEY_PIXEL_FORMAT, pixelFormat);

    bool paramValid = IsSupported(capData.pixFormat, pixelFormat);
    CHECK_AND_RETURN_RET_LOG(paramValid, AVCS_ERR_UNSUPPORT,
        "Param invalid, %{public}s: %{public}d, please check codec capabilities",
        MediaDescriptionKey::MD_KEY_PIXEL_FORMAT.data(), pixelFormat);     // Invalid pixel format

    return AVCS_ERR_OK;
}

int32_t FramerateChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)capData;
    (void)scenario;
    double framerate;
    bool paramExist = format.GetDoubleValue(MediaDescriptionKey::MD_KEY_FRAME_RATE, framerate);
    PrintParam(paramExist, MediaDescriptionKey::MD_KEY_FRAME_RATE, framerate);
    if (paramExist == false) {
        return AVCS_ERR_OK;
    }

    bool paramValid = framerate > 0 ? true : false;
    CHECK_AND_RETURN_RET_LOG(paramValid, AVCS_ERR_CODEC_PARAM_INCORRECT,
        "Param invalid, %{public}s: %{public}.2f, should be greater than 0",
        MediaDescriptionKey::MD_KEY_FRAME_RATE.data(), framerate);     // Invalid framerate

    return AVCS_ERR_OK;
}

int32_t BitrateAndQualityChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)scenario;
    int64_t bitrate;
    int32_t quality;
    int32_t bitrateMode;
    bool bitrateExist = format.GetLongValue(MediaDescriptionKey::MD_KEY_BITRATE, bitrate);
    bool qualityExist = format.GetIntValue(MediaDescriptionKey::MD_KEY_QUALITY, quality);
    bool bitrateModeExist = format.GetIntValue(MediaDescriptionKey::MD_KEY_VIDEO_ENCODE_BITRATE_MODE, bitrateMode);
    PrintParam(bitrateExist, MediaDescriptionKey::MD_KEY_BITRATE, bitrate);
    PrintParam(qualityExist, MediaDescriptionKey::MD_KEY_QUALITY, quality);
    PrintParam(bitrateModeExist, MediaDescriptionKey::MD_KEY_VIDEO_ENCODE_BITRATE_MODE, bitrateMode);

    CHECK_AND_RETURN_RET_LOG(!(bitrateExist && qualityExist), AVCS_ERR_CODEC_PARAM_INCORRECT,
        "Param invalid, bitrate and quality mutually include");  // bitrate and quality mutually include

    if (bitrateExist) {
        bool bitrateValid = capData.bitrate.InRange(bitrate);
        CHECK_AND_RETURN_RET_LOG(bitrateValid, AVCS_ERR_CODEC_PARAM_INCORRECT,
            "Param invalid, %{public}s: %{public}d, range: %{public}d-%{public}d",
            MediaDescriptionKey::MD_KEY_BITRATE.data(), static_cast<int32_t>(bitrate),
            capData.bitrate.minVal, capData.bitrate.maxVal);
    }
    if (qualityExist) {
        bool qualityValid = capData.encodeQuality.InRange(quality);
        CHECK_AND_RETURN_RET_LOG(qualityValid, AVCS_ERR_CODEC_PARAM_INCORRECT,
            "Param invalid, %{public}s: %{public}d, range: %{public}d-%{public}d",
            MediaDescriptionKey::MD_KEY_QUALITY.data(), quality,
            capData.encodeQuality.minVal, capData.encodeQuality.maxVal);
    }
    if (bitrateModeExist) {
        bool bitrateModeValid = IsSupported(capData.bitrateMode, bitrateMode);
        CHECK_AND_RETURN_RET_LOG(bitrateModeValid, AVCS_ERR_CODEC_PARAM_INCORRECT,
            "Param invalid, %{public}s: %{public}d",
            MediaDescriptionKey::MD_KEY_VIDEO_ENCODE_BITRATE_MODE.data(), bitrateMode);     // Invalid bitrate mode
        CHECK_AND_RETURN_RET_LOG(!(bitrateExist && bitrateMode == VideoEncodeBitrateMode::CQ),
            AVCS_ERR_CODEC_PARAM_INCORRECT, "Param invalid, in CQ mode but set bitrate!");
        CHECK_AND_RETURN_RET_LOG(!(qualityExist && bitrateMode != VideoEncodeBitrateMode::CQ),
            AVCS_ERR_CODEC_PARAM_INCORRECT, "Param invalid, not in CQ mode but set quality!");
        if (!qualityExist && bitrateMode == VideoEncodeBitrateMode::CQ) {
            format.PutIntValue(MediaDescriptionKey::MD_KEY_QUALITY, DEFAULT_QUALITY);
            AVCODEC_LOGW("In CQ mode but not set quality, set default quality: %{public}d", DEFAULT_QUALITY);
        }
    } else {
        if (qualityExist && IsSupported(capData.bitrateMode, static_cast<int32_t>(VideoEncodeBitrateMode::CQ))) {
            bitrateMode = VideoEncodeBitrateMode::CQ;
            format.PutIntValue(MediaDescriptionKey::MD_KEY_VIDEO_ENCODE_BITRATE_MODE, bitrateMode);
        }
    }

    return AVCS_ERR_OK;
}

int32_t VideoProfileChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)scenario;
    int32_t profile;
    bool paramExist = format.GetIntValue(MediaDescriptionKey::MD_KEY_PROFILE, profile);
    if (paramExist == false) {
        return AVCS_ERR_OK;
    }
    PrintParam(paramExist, MediaDescriptionKey::MD_KEY_PROFILE, profile);

    bool paramValid = IsSupported(capData.profiles, profile);
    CHECK_AND_RETURN_RET_LOG(paramValid, AVCS_ERR_CODEC_PARAM_INCORRECT,
        "Param invalid, %{public}s: %{public}d, please check codec capabilities",
        MediaDescriptionKey::MD_KEY_PROFILE.data(), profile);     // Invalid profile

    return AVCS_ERR_OK;
}

int32_t RotaitonChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)capData;
    (void)scenario;
    int32_t rotation;
    bool paramExist = format.GetIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, rotation);
    if (paramExist == false) {
        return AVCS_ERR_OK;
    }
    PrintParam(paramExist, MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, rotation);

    // valid rotation: 0, 90, 180, 270
    CHECK_AND_RETURN_RET_LOG(rotation == 0 || rotation == 90 || rotation == 180 || rotation == 270,
        AVCS_ERR_CODEC_PARAM_INCORRECT, "Param invalid, %{public}s: %{public}d, only support {0, 90, 180, 270}",
        MediaDescriptionKey::MD_KEY_ROTATION_ANGLE.data(), rotation);    //  Invalid rotation

    return AVCS_ERR_OK;
}

int32_t PostProcessingChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    if (scenario != CodecScenario::CODEC_SCENARIO_DEC_NORMAL) {
        return AVCS_ERR_OK;
    }
    int32_t colorSpace;
    bool hasColorSpace = format.GetIntValue(MediaDescriptionKey::MD_KEY_VIDEO_DECODER_OUTPUT_COLOR_SPACE, colorSpace);
    if (!hasColorSpace) {
        return AVCS_ERR_OK;
    }
    CHECK_AND_RETURN_RET_LOG((colorSpace >= 0) &&    // 0: OH_COLORSAPCE_NONE
                                 (colorSpace <= 31), // 31: OH_COLORSPACE_DISPLAY_BT2020_PQ
                             AVCS_ERR_INVALID_VAL, "The output color space %{public}d is invaild", colorSpace);
    CHECK_AND_RETURN_RET_LOG(capData.mimeType == CodecMimeType::VIDEO_HEVC && capData.isVendor,
                             AVCS_ERR_VIDEO_UNSUPPORT_COLOR_SPACE_CONVERSION,
                             "colorspace conversion is not available for the codec.");

    constexpr int32_t colorSpaceBt709Limited = 8; // see OH_COLORSPACE_BT709_LIMITED in native_buffer.h;
    CHECK_AND_RETURN_RET_LOG(colorSpace == colorSpaceBt709Limited, AVCS_ERR_VIDEO_UNSUPPORT_COLOR_SPACE_CONVERSION,
                             "The output color space %{public}d is not supported", colorSpace);
    PrintParam(true, MediaDescriptionKey::MD_KEY_VIDEO_DECODER_OUTPUT_COLOR_SPACE, colorSpace);

    return AVCS_ERR_OK;
}

int32_t QPChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)capData;
    (void)scenario;
    constexpr int32_t MAX_QP = 51;
    int32_t qpMin;
    int32_t qpMax;
    bool qpMinExist = format.GetIntValue(Tag::VIDEO_ENCODER_QP_MIN, qpMin);
    bool qpMaxExist = format.GetIntValue(Tag::VIDEO_ENCODER_QP_MAX, qpMax);
    if (!qpMinExist && !qpMaxExist) {
        return AVCS_ERR_OK;
    }
    CHECK_AND_RETURN_RET_LOG(!(qpMinExist != qpMaxExist), AVCS_ERR_INVALID_VAL,
        "Param invalid, QPmin and QPmax are expected to be set in pairs in format");
    PrintParam(qpMinExist && qpMaxExist, "QP", qpMin, qpMax);

    CHECK_AND_RETURN_RET_LOG(qpMin >= 0 && qpMin <= qpMax, AVCS_ERR_INVALID_VAL,
        "Param invalid, QP range: %{public}d-%{public}d", qpMin, qpMax);
    CHECK_AND_RETURN_RET_LOG(qpMax <= MAX_QP && qpMax >= qpMin, AVCS_ERR_INVALID_VAL,
        "Param invalid, QP range: %{public}d-%{public}d", qpMin, qpMax);
    
    return AVCS_ERR_OK;
}

int32_t TemporalGopSizeChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)capData;
    (void)scenario;
    int32_t gopSize;
    int32_t temporalGopSize;
    double frameRate;
    int32_t iFrameInterval;

    bool frameRateExist = format.GetDoubleValue(Tag::VIDEO_FRAME_RATE, frameRate);
    bool iFrameIntervalExist = format.GetIntValue(Tag::VIDEO_I_FRAME_INTERVAL, iFrameInterval);
    CHECK_AND_RETURN_RET_LOG(!(iFrameIntervalExist && iFrameInterval == 0), AVCS_ERR_INVALID_VAL,
        "Not support all key frame in temporal scalability");
    
    if (!frameRateExist) {
        frameRate = DEFAULT_FRAMERATE;
        format.PutDoubleValue(Tag::VIDEO_FRAME_RATE, DEFAULT_FRAMERATE);
    }
    if (!iFrameIntervalExist) {
        iFrameInterval = DEFAULT_I_FRAME_INTERVAL;
        format.PutIntValue(Tag::VIDEO_I_FRAME_INTERVAL, DEFAULT_I_FRAME_INTERVAL);
    }
    gopSize = iFrameInterval < 0 ? INT32_MAX : static_cast<int32_t>(frameRate * iFrameInterval / 1000); // 1000: ms to s
    CHECK_AND_RETURN_RET_LOG(gopSize > MIN_TEMPORAL_GOPSIZE, AVCS_ERR_INVALID_VAL,
        "Unsuppoted gop size, should be greater than %{public}d!", MIN_TEMPORAL_GOPSIZE);
    format.PutIntValue("video_encoder_gop_size", gopSize);

    bool temporalGopSizeExist = format.GetIntValue(Tag::VIDEO_ENCODER_TEMPORAL_GOP_SIZE, temporalGopSize);
    if (!temporalGopSizeExist) {
        return AVCS_ERR_OK;
    }
    PrintParam(temporalGopSizeExist, Tag::VIDEO_ENCODER_TEMPORAL_GOP_SIZE, temporalGopSize);
    CHECK_AND_RETURN_RET_LOG(temporalGopSize >= MIN_TEMPORAL_GOPSIZE, AVCS_ERR_INVALID_VAL,
        "Param invalid, %{public}s: %{public}d, expect greater or equal than %{public}d",
        Tag::VIDEO_ENCODER_TEMPORAL_GOP_SIZE, temporalGopSize, MIN_TEMPORAL_GOPSIZE);
    CHECK_AND_RETURN_RET_LOG(temporalGopSize < gopSize, AVCS_ERR_INVALID_VAL,
        "Param invalid, %{public}s: %{public}d, expect less than gop_size: %{public}d",
        Tag::VIDEO_ENCODER_TEMPORAL_GOP_SIZE, temporalGopSize, gopSize);
    
    return AVCS_ERR_OK;
}

int32_t TemporalGopReferenceModeChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)capData;
    (void)scenario;
    int32_t mode;
    bool modeExist = format.GetIntValue(Tag::VIDEO_ENCODER_TEMPORAL_GOP_REFERENCE_MODE, mode);
    if (!modeExist) {
        return AVCS_ERR_OK;
    }
    PrintParam(modeExist, Tag::VIDEO_ENCODER_TEMPORAL_GOP_REFERENCE_MODE, mode);

    using namespace OHOS::Media::Plugins;
    if (mode < static_cast<int32_t>(TemporalGopReferenceMode::ADJACENT_REFERENCE) ||
        mode >= static_cast<int32_t>(TemporalGopReferenceMode::UNKNOWN)) {
        AVCODEC_LOGE("Param invalid, %{public}s: %{public}d", Tag::VIDEO_ENCODER_TEMPORAL_GOP_REFERENCE_MODE, mode);
        return AVCS_ERR_INVALID_VAL;
    }
    return AVCS_ERR_OK;
}

int32_t UniformlyScaledReferenceChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)capData;
    (void)scenario;
    int32_t mode = -1;
    format.GetIntValue(Tag::VIDEO_ENCODER_TEMPORAL_GOP_REFERENCE_MODE, mode);
    using namespace OHOS::Media::Plugins;
    if (mode == static_cast<int32_t>(TemporalGopReferenceMode::UNIFORMLY_SCALED_REFERENCE)) {
        int32_t temporalGopSize;
        bool temporalGopSizeExist = format.GetIntValue(Tag::VIDEO_ENCODER_TEMPORAL_GOP_SIZE, temporalGopSize);
        if (!temporalGopSizeExist) {
            return AVCS_ERR_OK;
        }
        CHECK_AND_RETURN_RET_LOG(temporalGopSize == MIN_TEMPORAL_GOPSIZE || temporalGopSize == DEFAULT_TEMPORAL_GOPSIZE,
                                 AVCS_ERR_INVALID_VAL,
                                 "Current temporal reference mode param invalid, %{public}s: %{public}d, expect 2 or 4",
                                 Tag::VIDEO_ENCODER_TEMPORAL_GOP_SIZE, temporalGopSize);
    }
    return AVCS_ERR_OK;
}

int32_t ColorPrimariesChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)capData;
    (void)scenario;
    int32_t colorPrimaries;
    bool colorPrimariesExist = format.GetIntValue(Tag::VIDEO_COLOR_PRIMARIES, colorPrimaries);
    if (!colorPrimariesExist) {
        return AVCS_ERR_OK;
    }
    PrintParam(colorPrimariesExist, Tag::VIDEO_COLOR_PRIMARIES, colorPrimaries);

    if (colorPrimaries < static_cast<int32_t>(ColorPrimary::COLOR_PRIMARY_BT709) ||
        colorPrimaries > static_cast<int32_t>(ColorPrimary::COLOR_PRIMARY_P3D65)) {
        AVCODEC_LOGE("Param invalid, %{public}s: %{public}d", Tag::VIDEO_COLOR_PRIMARIES, colorPrimaries);
        return AVCS_ERR_INVALID_VAL;
    }
    return AVCS_ERR_OK;
}

int32_t TransferCharacteristicsChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)capData;
    (void)scenario;
    int32_t transferCharacteristics;
    bool transferCharacteristicsExist = format.GetIntValue(Tag::VIDEO_COLOR_TRC, transferCharacteristics);
    if (!transferCharacteristicsExist) {
        return AVCS_ERR_OK;
    }
    PrintParam(transferCharacteristicsExist, Tag::VIDEO_COLOR_TRC, transferCharacteristics);

    if (transferCharacteristics < static_cast<int32_t>(TransferCharacteristic::TRANSFER_CHARACTERISTIC_BT709) ||
        transferCharacteristics > static_cast<int32_t>(TransferCharacteristic::TRANSFER_CHARACTERISTIC_HLG)) {
        AVCODEC_LOGE("Param invalid, %{public}s: %{public}d", Tag::VIDEO_COLOR_TRC, transferCharacteristics);
        return AVCS_ERR_INVALID_VAL;
    }
    return AVCS_ERR_OK;
}

int32_t MatrixCoefficientsChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)capData;
    (void)scenario;
    int32_t matrixCoefficients;
    bool matrixCoefficientsExist = format.GetIntValue(Tag::VIDEO_COLOR_MATRIX_COEFF, matrixCoefficients);
    if (!matrixCoefficientsExist) {
        return AVCS_ERR_OK;
    }
    PrintParam(matrixCoefficientsExist, Tag::VIDEO_COLOR_MATRIX_COEFF, matrixCoefficients);

    if (matrixCoefficients < static_cast<int32_t>(MatrixCoefficient::MATRIX_COEFFICIENT_IDENTITY) ||
        matrixCoefficients > static_cast<int32_t>(MatrixCoefficient::MATRIX_COEFFICIENT_ICTCP)) {
        AVCODEC_LOGE("Param invalid, %{public}s: %{public}d", Tag::VIDEO_COLOR_MATRIX_COEFF, matrixCoefficients);
        return AVCS_ERR_INVALID_VAL;
    }
    return AVCS_ERR_OK;
}

int32_t LTRFrameCountChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    int32_t ltrFrameCount;
    bool ltrFrameCountExist = format.GetIntValue(Tag::VIDEO_ENCODER_LTR_FRAME_COUNT, ltrFrameCount);
    if (!ltrFrameCountExist) {
        return AVCS_ERR_OK;
    }
    PrintParam(ltrFrameCountExist, Tag::VIDEO_ENCODER_LTR_FRAME_COUNT, ltrFrameCount);

    CHECK_AND_RETURN_RET_LOG(scenario != CodecScenario::CODEC_SCENARIO_ENC_TEMPORAL_SCALABILITY,
        AVCS_ERR_UNSUPPORT, "Param invalid, not supported to set LTR frame count in temporal scalability scenario");

    auto ltrCap =
        capData.featuresMap.find(static_cast<int32_t>(AVCapabilityFeature::VIDEO_ENCODER_LONG_TERM_REFERENCE));
    if (ltrCap == capData.featuresMap.end()) {
        AVCODEC_LOGW("Not support LTR but set LTR frame count");
        format.RemoveKey(Tag::VIDEO_ENCODER_LTR_FRAME_COUNT);
        return AVCS_ERR_OK;
    }
    int32_t maxLTRFrameCount = 0;
    bool maxLTRFrameCountExist =
        ltrCap->second.GetIntValue(Tag::FEATURE_PROPERTY_VIDEO_ENCODER_MAX_LTR_FRAME_COUNT, maxLTRFrameCount);
    CHECK_AND_RETURN_RET_LOG(maxLTRFrameCountExist, AVCS_ERR_UNKNOWN, "Max LTR frame count not defined");

    CHECK_AND_RETURN_RET_LOG(ltrFrameCount >= 0 && ltrFrameCount <= maxLTRFrameCount, AVCS_ERR_INVALID_VAL,
        "Param invalid, LTR frame count range: %{public}d-%{public}d", 0, maxLTRFrameCount);

    return AVCS_ERR_OK;
}

int32_t ScalingModeChecker(CapabilityData &capData, Format &format, CodecScenario scenario)
{
    (void)capData;
    (void)scenario;
    int32_t scalingMode;
    bool scalingModeExist = format.GetIntValue(Tag::VIDEO_SCALE_TYPE, scalingMode);
    if (!scalingModeExist) {
        return AVCS_ERR_OK;
    }
    PrintParam(scalingModeExist, Tag::VIDEO_SCALE_TYPE, scalingMode);

    if (scalingMode < static_cast<int32_t>(OHOS::ScalingMode::SCALING_MODE_SCALE_TO_WINDOW) ||
        scalingMode > static_cast<int32_t>(OHOS::ScalingMode::SCALING_MODE_SCALE_CROP)) {
        AVCODEC_LOGE("Param invalid, %{public}s: %{public}d", Tag::VIDEO_SCALE_TYPE, scalingMode);
        return AVCS_ERR_INVALID_VAL;
    }
    return AVCS_ERR_OK;
}
} // namespace

namespace OHOS {
namespace MediaAVCodec {
int32_t CodecParamChecker::CheckConfigureValid(Media::Format &format, const std::string &codecName,
                                               CodecScenario scenario)
{
    AVCODEC_SYNC_TRACE;
    auto capData = CodecAbilitySingleton::GetInstance().GetCapabilityByName(codecName);
    CHECK_AND_RETURN_RET_LOG(capData != std::nullopt,
        AVCS_ERR_INVALID_OPERATION, "Get codec capability from codec list failed");

    auto checkers = CONFIGURE_CHECKERS_TABLE.find(scenario);
    CHECK_AND_RETURN_RET_LOG(checkers != CONFIGURE_CHECKERS_TABLE.end(), AVCS_ERR_UNSUPPORT,
        "This scenario can not find any checkers");

    int32_t result = AVCS_ERR_OK;
    for (const auto &checker : checkers->second) {
        auto ret = checker(capData.value(), format, scenario);
        if (ret == AVCS_ERR_CODEC_PARAM_INCORRECT) {
            result = AVCS_ERR_CODEC_PARAM_INCORRECT;
        }
        CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK || ret == AVCS_ERR_CODEC_PARAM_INCORRECT,
            ret, "Param check failed");
    }
    return result;
}

int32_t CodecParamChecker::CheckParameterValid(const Media::Format &format, Media::Format &oldFormat,
                                               const std::string &codecName, CodecScenario scenario)
{
    AVCODEC_SYNC_TRACE;
    auto capData = CodecAbilitySingleton::GetInstance().GetCapabilityByName(codecName);
    CHECK_AND_RETURN_RET_LOG(capData != std::nullopt,
        AVCS_ERR_INVALID_OPERATION, "Get codec capability from codec list failed");

    auto checkers = PARAMETER_CHECKERS_TABLE.find(scenario);
    CHECK_AND_RETURN_RET_LOG(checkers != PARAMETER_CHECKERS_TABLE.end(), AVCS_ERR_UNSUPPORT,
        "This scenario can not find any checkers");

    MergeFormat(format, oldFormat);

    for (const auto &checker : checkers->second) {
        auto ret = checker(capData.value(), oldFormat, scenario);
        CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, ret, "Param check failed");
    }
    return AVCS_ERR_OK;
}

std::optional<CodecScenario> CodecParamChecker::CheckCodecScenario(const Media::Format &format, AVCodecType codecType,
                                                                   const std::string &codecName)
{
    auto capData = CodecAbilitySingleton::GetInstance().GetCapabilityByName(codecName);
    CHECK_AND_RETURN_RET_LOG(capData != std::nullopt,
        std::nullopt, "Get codec capability from codec list failed");

    CodecScenario scenario = CodecScenario::CODEC_SCENARIO_DEC_NORMAL;
    if (codecType == AVCODEC_TYPE_VIDEO_ENCODER) {
        scenario = CodecScenario::CODEC_SCENARIO_ENC_NORMAL;
    }
    
    for (const auto& checker : VIDEO_SCENARIO_CHECKER_LIST) {
        auto ret = checker(capData.value(), format, codecType);
        if (ret == std::nullopt) {
            continue;
        }
        scenario = ret.value();
        break;
    }

    PrintCodecScenario(scenario);
    return scenario;
}

void CodecParamChecker::MergeFormat(const Media::Format &format, Media::Format &oldFormat)
{
    for (const auto& key : FORMAT_MERGE_LIST) {
        if (!format.ContainKey(key)) {
            continue;
        }
        auto keyType = format.GetValueType(key);
        switch (keyType) {
            case FORMAT_TYPE_INT32: {
                int32_t value;
                format.GetIntValue(key, value);
                oldFormat.PutIntValue(key, value);
                break;
            }
            case FORMAT_TYPE_INT64: {
                int64_t value;
                format.GetLongValue(key, value);
                oldFormat.PutLongValue(key, value);
                break;
            }
            case FORMAT_TYPE_FLOAT: {
                float value;
                format.GetFloatValue(key, value);
                oldFormat.PutFloatValue(key, value);
                break;
            }
            case FORMAT_TYPE_DOUBLE: {
                double value;
                format.GetDoubleValue(key, value);
                oldFormat.PutDoubleValue(key, value);
                break;
            }
            case FORMAT_TYPE_STRING: {
                std::string value;
                format.GetStringValue(key, value);
                oldFormat.PutStringValue(key, value);
                break;
            }
            default:
                break;
        }
    }
}
} // namespace MediaAVCodec
} // namespace OHOS