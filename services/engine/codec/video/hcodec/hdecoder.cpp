/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "hdecoder.h"
#include <cassert>
#include <sys/ioctl.h>
#include <linux/dma-buf.h>
#include "utils/hdf_base.h"
#include "codec_omx_ext.h"
#include "media_description.h"  // foundation/multimedia/av_codec/interfaces/inner_api/native/
#include "sync_fence.h"  // foundation/graphic/graphic_2d/utils/sync_fence/export/
#include "OMX_VideoExt.h"
#include "hcodec_log.h"
#include "hcodec_dfx.h"
#include "type_converter.h"
#include "surface_buffer.h"
#include "buffer_extra_data_impl.h"  // foundation/graphic/graphic_surface/surface/include/

namespace OHOS::MediaAVCodec {
using namespace std;
using namespace CodecHDI;

HDecoder::~HDecoder()
{
    MsgHandleLoop::Stop();
}

int32_t HDecoder::OnConfigure(const Format &format)
{
    configFormat_ = make_shared<Format>(format);

    int32_t ret = SetLowLatency(format);
    if (ret != AVCS_ERR_OK) {
        return ret;
    }
    SaveTransform(format);
    SaveScaleMode(format);
    (void)SetProcessName();
    (void)SetFrameRateAdaptiveMode(format);
    return SetupPort(format);
}

int32_t HDecoder::SetupPort(const Format &format)
{
    int32_t width;
    if (!format.GetIntValue(MediaDescriptionKey::MD_KEY_WIDTH, width) || width <= 0) {
        HLOGE("format should contain width");
        return AVCS_ERR_INVALID_VAL;
    }
    int32_t height;
    if (!format.GetIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, height) || height <= 0) {
        HLOGE("format should contain height");
        return AVCS_ERR_INVALID_VAL;
    }
    HLOGI("user set width %d, height %d", width, height);
    if (!GetPixelFmtFromUser(format)) {
        return AVCS_ERR_INVALID_VAL;
    }

    optional<double> frameRate = GetFrameRateFromUser(format);
    if (frameRate.has_value()) {
        codecRate_ = frameRate.value();
    } else {
        HLOGI("user don't set valid frame rate, use default 60.0");
        frameRate = 60.0;  // default frame rate 60.0
    }

    PortInfo inputPortInfo {static_cast<uint32_t>(width), static_cast<uint32_t>(height),
                            codingType_, std::nullopt, frameRate.value()};
    int32_t maxInputSize = 0;
    (void)format.GetIntValue(MediaDescriptionKey::MD_KEY_MAX_INPUT_SIZE, maxInputSize);
    if (maxInputSize > 0) {
        inputPortInfo.inputBufSize = static_cast<uint32_t>(maxInputSize);
    }
    int32_t ret = SetVideoPortInfo(OMX_DirInput, inputPortInfo);
    if (ret != AVCS_ERR_OK) {
        return ret;
    }

    PortInfo outputPortInfo = {static_cast<uint32_t>(width), static_cast<uint32_t>(height),
                               OMX_VIDEO_CodingUnused, configuredFmt_, frameRate.value()};
    ret = SetVideoPortInfo(OMX_DirOutput, outputPortInfo);
    if (ret != AVCS_ERR_OK) {
        return ret;
    }

    return AVCS_ERR_OK;
}

int32_t HDecoder::UpdateInPortFormat()
{
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParam(def);
    def.nPortIndex = OMX_DirInput;
    if (!GetParameter(OMX_IndexParamPortDefinition, def)) {
        HLOGE("get input port definition failed");
        return AVCS_ERR_UNKNOWN;
    }
    PrintPortDefinition(def);
    if (inputFormat_ == nullptr) {
        inputFormat_ = make_shared<Format>();
    }
    inputFormat_->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, def.format.video.nFrameWidth);
    inputFormat_->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, def.format.video.nFrameHeight);
    return AVCS_ERR_OK;
}

bool HDecoder::UpdateConfiguredFmt(OMX_COLOR_FORMATTYPE portFmt)
{
    auto graphicFmt = static_cast<GraphicPixelFormat>(portFmt);
    if (graphicFmt != configuredFmt_.graphicFmt) {
        optional<PixelFmt> fmt = TypeConverter::GraphicFmtToFmt(graphicFmt);
        if (!fmt.has_value()) {
            return false;
        }
        HLOGI("GraphicPixelFormat need update: configured(%s) -> portdefinition(%s)",
            configuredFmt_.strFmt.c_str(), fmt->strFmt.c_str());
        configuredFmt_ = fmt.value();
    }
    return true;
}

int32_t HDecoder::UpdateOutPortFormat()
{
    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParam(def);
    def.nPortIndex = OMX_DirOutput;
    if (!GetParameter(OMX_IndexParamPortDefinition, def)) {
        HLOGE("get output port definition failed");
        return AVCS_ERR_UNKNOWN;
    }
    PrintPortDefinition(def);
    if (def.nBufferCountActual == 0) {
        HLOGE("invalid bufferCount");
        return AVCS_ERR_UNKNOWN;
    }
    (void)UpdateConfiguredFmt(def.format.video.eColorFormat);

    uint32_t w = def.format.video.nFrameWidth;
    uint32_t h = def.format.video.nFrameHeight;

    // save into member variable
    OHOS::Rect damage{};
    GetCropFromOmx(w, h, damage);
    outBufferCnt_ = def.nBufferCountActual;
    requestCfg_.timeout = 0;
    requestCfg_.width = damage.w;
    requestCfg_.height = damage.h;
    requestCfg_.strideAlignment = STRIDE_ALIGNMENT;
    requestCfg_.format = configuredFmt_.graphicFmt;
    requestCfg_.usage = GetProducerUsage();

    // save into format
    if (outputFormat_ == nullptr) {
        outputFormat_ = make_shared<Format>();
    }
    if (!outputFormat_->ContainKey(MediaDescriptionKey::MD_KEY_WIDTH)) {
        outputFormat_->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, w); // deprecated
    }
    if (!outputFormat_->ContainKey(MediaDescriptionKey::MD_KEY_HEIGHT)) {
        outputFormat_->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, h); // deprecated
    }
    outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_DISPLAY_WIDTH, damage.w);
    outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_DISPLAY_HEIGHT, damage.h);
    outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_PIC_WIDTH, damage.w);
    outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_PIC_HEIGHT, damage.h);
    outputFormat_->PutIntValue(MediaDescriptionKey::MD_KEY_PIXEL_FORMAT,
        static_cast<int32_t>(configuredFmt_.innerFmt));
    HLOGI("output format: %s", outputFormat_->Stringify().c_str());
    return AVCS_ERR_OK;
}

void HDecoder::UpdateColorAspects()
{
    CodecVideoColorspace param;
    InitOMXParamExt(param);
    param.portIndex = OMX_DirOutput;
    if (!GetParameter(OMX_IndexColorAspects, param, true)) {
        return;
    }
    HLOGI("range:%d, primary:%d, transfer:%d, matrix:%d)",
        param.aspects.range, param.aspects.primaries, param.aspects.transfer, param.aspects.matrixCoeffs);
    if (outputFormat_) {
        outputFormat_->PutIntValue(MediaDescriptionKey::MD_KEY_RANGE_FLAG, param.aspects.range);
        outputFormat_->PutIntValue(MediaDescriptionKey::MD_KEY_COLOR_PRIMARIES, param.aspects.primaries);
        outputFormat_->PutIntValue(MediaDescriptionKey::MD_KEY_TRANSFER_CHARACTERISTICS, param.aspects.transfer);
        outputFormat_->PutIntValue(MediaDescriptionKey::MD_KEY_MATRIX_COEFFICIENTS, param.aspects.matrixCoeffs);
        HLOGI("output format changed: %s", outputFormat_->Stringify().c_str());
        callback_->OnOutputFormatChanged(*(outputFormat_.get()));
    }
}

void HDecoder::GetCropFromOmx(uint32_t w, uint32_t h, OHOS::Rect& damage)
{
    damage.x = 0;
    damage.y = 0;
    damage.w = static_cast<int32_t>(w);
    damage.h = static_cast<int32_t>(h);

    OMX_CONFIG_RECTTYPE rect;
    InitOMXParam(rect);
    rect.nPortIndex = OMX_DirOutput;
    if (!GetParameter(OMX_IndexConfigCommonOutputCrop, rect, true)) {
        HLOGW("get crop failed, use default");
        return;
    }
    if (rect.nLeft < 0 || rect.nTop < 0 ||
        rect.nWidth == 0 || rect.nHeight == 0 ||
        rect.nLeft + static_cast<int32_t>(rect.nWidth) > static_cast<int32_t>(w) ||
        rect.nTop + static_cast<int32_t>(rect.nHeight) > static_cast<int32_t>(h)) {
        HLOGW("wrong crop rect (%d, %d, %u, %u) vs. frame (%u," \
              "%u), use default", rect.nLeft, rect.nTop, rect.nWidth, rect.nHeight, w, h);
        return;
    }
    HLOGI("crop rect (%d, %d, %u, %u)",
          rect.nLeft, rect.nTop, rect.nWidth, rect.nHeight);
    damage.x = rect.nLeft;
    damage.y = rect.nTop;
    damage.w = static_cast<int32_t>(rect.nWidth);
    damage.h = static_cast<int32_t>(rect.nHeight);
    if (outputFormat_) {
        outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_CROP_LEFT, rect.nLeft);
        outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_CROP_TOP, rect.nTop);
        outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_CROP_RIGHT,
            static_cast<int32_t>(rect.nLeft + rect.nWidth) - 1);
        outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_CROP_BOTTOM,
            static_cast<int32_t>(rect.nTop + rect.nHeight) - 1);
    }
}

int32_t HDecoder::OnSetOutputSurface(const sptr<Surface> &surface, bool cfg)
{
    return cfg ? OnSetOutputSurfaceWhenCfg(surface) : OnSetOutputSurfaceWhenRunning(surface);
}

int32_t HDecoder::OnSetOutputSurfaceWhenCfg(const sptr<Surface> &surface)
{
    SCOPED_TRACE();
    HLOGI(">>");
    if (surface == nullptr) {
        HLOGE("surface is null");
        return AVCS_ERR_INVALID_VAL;
    }
    if (surface->IsConsumer()) {
        HLOGE("expect a producer surface but got a consumer surface");
        return AVCS_ERR_INVALID_VAL;
    }
    int32_t ret = RegisterListenerToSurface(surface);
    if (ret != AVCS_ERR_OK) {
        return ret;
    }
    currSurface_ = SurfaceItem(surface);
    HLOGI("set surface(%" PRIu64 ")(%s) succ", surface->GetUniqueId(), surface->GetName().c_str());
    return AVCS_ERR_OK;
}

int32_t HDecoder::OnSetParameters(const Format &format)
{
    int32_t ret = SaveTransform(format, true);
    if (ret != AVCS_ERR_OK) {
        return ret;
    }
    ret = SaveScaleMode(format, true);
    if (ret != AVCS_ERR_OK) {
        return ret;
    }
    optional<double> frameRate = GetFrameRateFromUser(format);
    if (frameRate.has_value()) {
        OMX_PARAM_U32TYPE framerateCfgType;
        InitOMXParam(framerateCfgType);
        framerateCfgType.nPortIndex = OMX_DirInput;
        framerateCfgType.nU32 = frameRate.value() * FRAME_RATE_COEFFICIENT;
        if (SetParameter(OMX_IndexCodecExtConfigOperatingRate, framerateCfgType, true)) {
            HLOGI("succ to set frameRate %.f", frameRate.value());
        } else {
            HLOGW("succ to set frameRate %.f", frameRate.value());
        }
        codecRate_ = frameRate.value();
    }
    return AVCS_ERR_OK;
}

int32_t HDecoder::SaveTransform(const Format &format, bool set)
{
    int32_t orientation = 0;
    if (format.GetIntValue(OHOS::Media::Tag::VIDEO_ORIENTATION_TYPE, orientation)) {
        HLOGI("GraphicTransformType = %d", orientation);
        transform_ = static_cast<GraphicTransformType>(orientation);
        if (set) {
            return SetTransform();
        }
        return AVCS_ERR_OK;
    }
    int32_t rotate;
    if (!format.GetIntValue(MediaDescriptionKey::MD_KEY_ROTATION_ANGLE, rotate)) {
        return AVCS_ERR_OK;
    }
    optional<GraphicTransformType> transform = TypeConverter::InnerRotateToDisplayRotate(
        static_cast<VideoRotation>(rotate));
    if (!transform.has_value()) {
        return AVCS_ERR_INVALID_VAL;
    }
    HLOGI("VideoRotation = %d, GraphicTransformType = %d", rotate, transform.value());
    transform_ = transform.value();
    if (set) {
        return SetTransform();
    }
    return AVCS_ERR_OK;
}

int32_t HDecoder::SetTransform()
{
    if (currSurface_.surface_ == nullptr) {
        return AVCS_ERR_INVALID_VAL;
    }
    GSError err = currSurface_.surface_->SetTransform(transform_);
    if (err != GSERROR_OK) {
        HLOGW("set GraphicTransformType %d to surface failed", transform_);
        return AVCS_ERR_UNKNOWN;
    }
    HLOGI("set GraphicTransformType %d to surface succ", transform_);
    return AVCS_ERR_OK;
}

int32_t HDecoder::SaveScaleMode(const Format &format, bool set)
{
    int scaleType;
    if (!format.GetIntValue(MediaDescriptionKey::MD_KEY_SCALE_TYPE, scaleType)) {
        return AVCS_ERR_OK;
    }
    auto scaleMode = static_cast<ScalingMode>(scaleType);
    if (scaleMode != SCALING_MODE_SCALE_TO_WINDOW && scaleMode != SCALING_MODE_SCALE_CROP) {
        HLOGW("user set invalid scale mode %d", scaleType);
        return AVCS_ERR_INVALID_VAL;
    }
    HLOGI("user set ScalingType = %d", scaleType);
    scaleMode_ = scaleMode;
    if (set) {
        return SetScaleMode();
    }
    return AVCS_ERR_OK;
}

int32_t HDecoder::SetScaleMode()
{
    if (currSurface_.surface_ == nullptr || !scaleMode_.has_value()) {
        return AVCS_ERR_INVALID_VAL;
    }
    GSError err = currSurface_.surface_->SetScalingMode(scaleMode_.value());
    if (err != GSERROR_OK) {
        HLOGW("set ScalingMode %d to surface failed", scaleMode_.value());
        return AVCS_ERR_UNKNOWN;
    }
    HLOGI("set ScalingMode %d to surface succ", scaleMode_.value());
    return AVCS_ERR_OK;
}

int32_t HDecoder::SubmitOutputBuffersToOmxNode()
{
    for (BufferInfo& info : outputBufferPool_) {
        if (info.owner != BufferOwner::OWNED_BY_US) {
            continue;
        }
        if (info.surfaceBuffer != nullptr) {
            int32_t ret = NotifyOmxToFillThisOutBuffer(info);
            if (ret != AVCS_ERR_OK) {
                return ret;
            }
        }
    }
    auto inCnt = std::count_if(inputBufferPool_.begin(), inputBufferPool_.end(), [](const BufferInfo& info) {
        return info.owner == BufferOwner::OWNED_BY_OMX;
    });
    inCnt++; // at least submit one out buffer to omx
    while (inCnt > 0) {
        SubmitDynamicBufferIfPossible();
        inCnt--;
    }
    return AVCS_ERR_OK;
}

bool HDecoder::UseHandleOnOutputPort(bool isDynamic)
{
    UseBufferType useBufferTypes;
    InitOMXParamExt(useBufferTypes);
    useBufferTypes.portIndex = OMX_DirOutput;
    useBufferTypes.bufferType = (isDynamic ? CODEC_BUFFER_TYPE_DYNAMIC_HANDLE : CODEC_BUFFER_TYPE_HANDLE);
    return SetParameter(OMX_IndexParamUseBufferType, useBufferTypes);
}

bool HDecoder::ReadyToStart()
{
    if (callback_ == nullptr || outputFormat_ == nullptr || inputFormat_ == nullptr) {
        return false;
    }
    if (currSurface_.surface_ == nullptr) {
        if (!UseHandleOnOutputPort(false)) {
            HLOGE("invalid output buffer type");
            return false;
        }
        HLOGI("buffer mode");
        return true;
    }
    // has surface
    SupportBufferType type;
    InitOMXParamExt(type);
    type.portIndex = OMX_DirOutput;
    if (GetParameter(OMX_IndexParamSupportBufferType, type) &&
        (type.bufferTypes & CODEC_BUFFER_TYPE_DYNAMIC_HANDLE) &&
        UseHandleOnOutputPort(true)) {
        HLOGI("surface dynamic mode");
        isDynamic_ = true;
    } else if (UseHandleOnOutputPort(false)) {
        HLOGI("surface normal mode");
        isDynamic_ = false;
    } else {
        HLOGE("invalid output buffer type");
        return false;
    }
    SetTransform();
    SetScaleMode();
    return true;
}

int32_t HDecoder::AllocateBuffersOnPort(OMX_DIRTYPE portIndex)
{
    if (portIndex == OMX_DirInput) {
        return AllocateAvLinearBuffers(portIndex);
    }
    int32_t ret;
    if (currSurface_.surface_) {
        ret = isDynamic_ ? AllocOutDynamicSurfaceBuf() : AllocateOutputBuffersFromSurface();
    } else {
        ret = AllocateAvSurfaceBuffers(portIndex);
    }
    if (ret == AVCS_ERR_OK) {
        UpdateFormatFromSurfaceBuffer();
    }
    return ret;
}

void HDecoder::SetCallerToBuffer(int fd)
{
    if (currSurface_.surface_ == nullptr) {
        return; // only set on surface mode
    }
    string pid = std::to_string(calledByAvcodec_ ? avcodecCaller_.pid : playerCaller_.pid);
    int ret = ioctl(fd, DMA_BUF_SET_NAME_A, pid.c_str());
    if (ret != 0) {
        HLOGW("set pid %s to fd %d failed", pid.c_str(), fd);
        return;
    }
    HLOGI("set pid %s to fd %d succ", pid.c_str(), fd);
}

void HDecoder::UpdateFormatFromSurfaceBuffer()
{
    if (outputBufferPool_.empty()) {
        return;
    }
    sptr<SurfaceBuffer> surfaceBuffer = outputBufferPool_.front().surfaceBuffer;
    if (surfaceBuffer == nullptr) {
        return;
    }
    HLOGI(">>");
    outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_DISPLAY_WIDTH, surfaceBuffer->GetWidth());
    outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_DISPLAY_HEIGHT, surfaceBuffer->GetHeight());
    outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_PIC_WIDTH, surfaceBuffer->GetWidth());
    outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_PIC_HEIGHT, surfaceBuffer->GetHeight());
    int32_t stride = surfaceBuffer->GetStride();
    if (stride <= 0) {
        HLOGW("invalid stride %d", stride);
        return;
    }
    outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_STRIDE, stride);
    outputFormat_->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, stride); // deprecated

    OH_NativeBuffer_Planes *planes = nullptr;
    GSError err = surfaceBuffer->GetPlanesInfo(reinterpret_cast<void**>(&planes));
    if (err != GSERROR_OK || planes == nullptr) {
        HLOGW("get plane info failed, GSError=%d", err);
        return;
    }
    for (uint32_t i = 0; i < planes->planeCount; i++) {
        HLOGI("plane[%u]: offset=%" PRIu64 ", rowStride=%u, columnStride=%u",
              i, planes->planes[i].offset, planes->planes[i].rowStride, planes->planes[i].columnStride);
    }
    int32_t sliceHeight = static_cast<int32_t>(static_cast<int64_t>(planes->planes[1].offset) / stride);
    HLOGI("[%dx%d][%dx%d]", surfaceBuffer->GetWidth(), surfaceBuffer->GetHeight(), stride, sliceHeight);
    outputFormat_->PutIntValue(OHOS::Media::Tag::VIDEO_SLICE_HEIGHT, sliceHeight);
    outputFormat_->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, sliceHeight);
}

int32_t HDecoder::SubmitAllBuffersOwnedByUs()
{
    HLOGI(">>");
    if (isBufferCirculating_) {
        HLOGI("buffer is already circulating, no need to do again");
        return AVCS_ERR_OK;
    }
    int32_t ret = SubmitOutputBuffersToOmxNode();
    if (ret != AVCS_ERR_OK) {
        return ret;
    }
    for (BufferInfo& info : inputBufferPool_) {
        if (info.owner == BufferOwner::OWNED_BY_US) {
            NotifyUserToFillThisInBuffer(info);
        }
    }
    isBufferCirculating_ = true;
    return AVCS_ERR_OK;
}

void HDecoder::EraseBufferFromPool(OMX_DIRTYPE portIndex, size_t i)
{
    vector<BufferInfo>& pool = (portIndex == OMX_DirInput) ? inputBufferPool_ : outputBufferPool_;
    if (i >= pool.size()) {
        return;
    }
    BufferInfo& info = pool[i];
    if (portIndex == OMX_DirOutput && info.owner != BufferOwner::OWNED_BY_SURFACE) {
        CancelBufferToSurface(info);
    }
    FreeOmxBuffer(portIndex, info);
    ReduceOwner((portIndex == OMX_DirInput), info.owner);
    pool.erase(pool.begin() + i);
}

void HDecoder::OnClearBufferPool(OMX_DIRTYPE portIndex)
{
    if ((portIndex == OMX_DirOutput) && currSurface_.surface_) {
        GSError err = currSurface_.surface_->CleanCache();
        if (err != GSERROR_OK) {
            HLOGW("clean cache failed, GSError=%d", err);
        }
    }
}

uint64_t HDecoder::GetProducerUsage()
{
    uint64_t producerUsage = currSurface_.surface_ ? SURFACE_MODE_PRODUCER_USAGE : BUFFER_MODE_REQUEST_USAGE;

    GetBufferHandleUsageParams vendorUsage;
    InitOMXParamExt(vendorUsage);
    vendorUsage.portIndex = static_cast<uint32_t>(OMX_DirOutput);
    if (GetParameter(OMX_IndexParamGetBufferHandleUsage, vendorUsage)) {
        HLOGI("vendor producer usage = 0x%" PRIx64 "", vendorUsage.usage);
        producerUsage |= vendorUsage.usage;
    } else {
        HLOGW("get vendor producer usage failed, add CPU_READ");
        producerUsage |= BUFFER_USAGE_CPU_READ;
    }
    HLOGI("decoder producer usage = 0x%" PRIx64 "", producerUsage);
    return producerUsage;
}

void HDecoder::CombineConsumerUsage()
{
    uint32_t consumerUsage = currSurface_.surface_->GetDefaultUsage();
    uint64_t finalUsage = requestCfg_.usage | consumerUsage;
    HLOGI("producer usage 0x%" PRIx64 " | consumer usage 0x%x -> 0x%" PRIx64 "",
        requestCfg_.usage, consumerUsage, finalUsage);
    requestCfg_.usage = finalUsage;
}

int32_t HDecoder::SetQueueSize(const sptr<Surface> &surface, uint32_t targetSize)
{
    GSError err = surface->SetQueueSize(targetSize);
    if (err != GSERROR_OK) {
        HLOGE("surface(%" PRIu64 "), SetQueueSize to %u failed, GSError=%d",
              surface->GetUniqueId(), targetSize, err);
        return AVCS_ERR_UNKNOWN;
    }
    HLOGI("surface(%" PRIu64 "), SetQueueSize to %u succ", surface->GetUniqueId(), targetSize);
    return AVCS_ERR_OK;
}

int32_t HDecoder::AllocOutDynamicSurfaceBuf()
{
    SCOPED_TRACE();
    currSurface_.surface_->CleanCache();
    int32_t ret = SetQueueSize(currSurface_.surface_, outBufferCnt_);
    if (ret != AVCS_ERR_OK) {
        return ret;
    }
    outputBufferPool_.clear();

    for (uint32_t i = 0; i < outBufferCnt_; ++i) {
        shared_ptr<OmxCodecBuffer> omxBuffer = DynamicSurfaceBufferToOmxBuffer();
        shared_ptr<OmxCodecBuffer> outBuffer = make_shared<OmxCodecBuffer>();
        ret = compNode_->UseBuffer(OMX_DirOutput, *omxBuffer, *outBuffer);
        if (ret != HDF_SUCCESS) {
            HLOGE("Failed to UseBuffer on input port");
            return AVCS_ERR_UNKNOWN;
        }
        BufferInfo info {};
        info.isInput = false;
        info.owner = BufferOwner::OWNED_BY_US;
        info.surfaceBuffer = nullptr;
        info.avBuffer = AVBuffer::CreateAVBuffer();
        info.omxBuffer = outBuffer;
        info.bufferId = outBuffer->bufferId;
        outputBufferPool_.push_back(info);
    }
    HLOGI("succ");
    return AVCS_ERR_OK;
}

int32_t HDecoder::AllocateOutputBuffersFromSurface()
{
    SCOPED_TRACE();
    currSurface_.surface_->CleanCache();
    int32_t ret = SetQueueSize(currSurface_.surface_, outBufferCnt_);
    if (ret != AVCS_ERR_OK) {
        return ret;
    }
    outputBufferPool_.clear();
    CombineConsumerUsage();
    for (uint32_t i = 0; i < outBufferCnt_; ++i) {
        sptr<SurfaceBuffer> surfaceBuffer;
        sptr<SyncFence> fence;
        GSError err = currSurface_.surface_->RequestBuffer(surfaceBuffer, fence, requestCfg_);
        if (err != GSERROR_OK || surfaceBuffer == nullptr) {
            HLOGE("RequestBuffer %u failed, GSError=%d", i, err);
            return AVCS_ERR_UNKNOWN;
        }
        shared_ptr<OmxCodecBuffer> omxBuffer = SurfaceBufferToOmxBuffer(surfaceBuffer);
        if (omxBuffer == nullptr) {
            currSurface_.surface_->CancelBuffer(surfaceBuffer);
            return AVCS_ERR_UNKNOWN;
        }
        shared_ptr<OmxCodecBuffer> outBuffer = make_shared<OmxCodecBuffer>();
        int32_t hdfRet = compNode_->UseBuffer(OMX_DirOutput, *omxBuffer, *outBuffer);
        if (hdfRet != HDF_SUCCESS) {
            currSurface_.surface_->CancelBuffer(surfaceBuffer);
            HLOGE("Failed to UseBuffer with output port");
            return AVCS_ERR_NO_MEMORY;
        }
        SetCallerToBuffer(surfaceBuffer->GetFileDescriptor());
        outBuffer->fenceFd = -1;
        BufferInfo info {};
        info.isInput = false;
        info.owner = BufferOwner::OWNED_BY_US;
        info.surfaceBuffer = surfaceBuffer;
        info.avBuffer = AVBuffer::CreateAVBuffer();
        info.omxBuffer = outBuffer;
        info.bufferId = outBuffer->bufferId;
        outputBufferPool_.push_back(info);
        HLOGI("bufferId=%u, seq=%u", info.bufferId, surfaceBuffer->GetSeqNum());
    }
    return AVCS_ERR_OK;
}

void HDecoder::CancelBufferToSurface(BufferInfo& info)
{
    if (currSurface_.surface_ && info.surfaceBuffer) {
        GSError err = currSurface_.surface_->CancelBuffer(info.surfaceBuffer);
        if (err != GSERROR_OK) {
            HLOGW("surface(%" PRIu64 "), CancelBuffer(seq=%u) failed, GSError=%d",
                  currSurface_.surface_->GetUniqueId(), info.surfaceBuffer->GetSeqNum(), err);
        } else {
            HLOGI("surface(%" PRIu64 "), CancelBuffer(seq=%u) succ",
                  currSurface_.surface_->GetUniqueId(), info.surfaceBuffer->GetSeqNum());
        }
    }
    ChangeOwner(info, BufferOwner::OWNED_BY_SURFACE); // change owner even if cancel failed
}

int32_t HDecoder::RegisterListenerToSurface(const sptr<Surface> &surface)
{
    uint64_t surfaceId = surface->GetUniqueId();
    std::weak_ptr<HCodec> weakThis = weak_from_this();
    GSError err = surface->RegisterReleaseListener([weakThis, surfaceId](sptr<SurfaceBuffer>&) {
        std::shared_ptr<HCodec> codec = weakThis.lock();
        if (codec == nullptr) {
            LOGD("decoder is gone");
            return GSERROR_OK;
        }
        return codec->OnBufferReleasedByConsumer(surfaceId);
    });
    if (err != GSERROR_OK) {
        HLOGE("surface(%" PRIu64 "), RegisterReleaseListener failed, GSError=%d", surfaceId, err);
        return AVCS_ERR_UNKNOWN;
    }
    return AVCS_ERR_OK;
}

GSError HDecoder::OnBufferReleasedByConsumer(uint64_t surfaceId)
{
    ParamSP param = make_shared<ParamBundle>();
    param->SetValue("surfaceId", surfaceId);
    SendAsyncMsg(MsgWhat::GET_BUFFER_FROM_SURFACE, param);
    return GSERROR_OK;
}

bool HDecoder::RequestAndFindBelongTo(
    sptr<SurfaceBuffer>& buffer, sptr<SyncFence>& fence, std::vector<BufferInfo>::iterator& iter)
{
    SCOPED_TRACE();
    GSError err = currSurface_.surface_->RequestBuffer(buffer, fence, requestCfg_);
    if (err != GSERROR_OK || buffer == nullptr || buffer->GetBufferHandle() == nullptr) {
        HLOGW("GSError=%d", err);
        return false;
    }
    BufferHandle* handle = buffer->GetBufferHandle();
    iter = std::find_if(outputBufferPool_.begin(), outputBufferPool_.end(), [handle](const BufferInfo& info) {
        return (info.owner == BufferOwner::OWNED_BY_SURFACE) &&
               info.surfaceBuffer && (info.surfaceBuffer->GetBufferHandle() == handle);
    });
    return true;
}

void HDecoder::OnGetBufferFromSurface(const ParamSP& param)
{
    uint64_t surfaceId = 0;
    param->GetValue("surfaceId", surfaceId);
    if (!currSurface_.surface_ || currSurface_.surface_->GetUniqueId() != surfaceId) {
        return;
    }
    sptr<SurfaceBuffer> buffer;
    sptr<SyncFence> fence;
    auto iter = outputBufferPool_.end();
    if (!RequestAndFindBelongTo(buffer, fence, iter)) {
        return;
    }
    if (iter == outputBufferPool_.end()) {
        HLOGI("seq=%u dont belong to output set, cancel it", buffer->GetSeqNum());
        currSurface_.surface_->CancelBuffer(buffer);
        return;
    }
    WaitFence(fence);
    ChangeOwner(*iter, BufferOwner::OWNED_BY_US);
    NotifyOmxToFillThisOutBuffer(*iter);
}

void HDecoder::SubmitDynamicBufferIfPossible()
{
    if (!currSurface_.surface_ || !isDynamic_) {
        return;
    }
    auto idleIter = std::find_if(outputBufferPool_.begin(), outputBufferPool_.end(), [](const BufferInfo& info) {
        return info.surfaceBuffer == nullptr;
    });
    if (idleIter == outputBufferPool_.end()) {
        return;
    }
    for (size_t i = 0; i < outputBufferPool_.size(); i++) {
        sptr<SurfaceBuffer> buffer;
        sptr<SyncFence> fence;
        auto iter = outputBufferPool_.end();
        if (!RequestAndFindBelongTo(buffer, fence, iter)) {
            return;
        }
        WaitFence(fence);
        if (iter != outputBufferPool_.end()) {
            ChangeOwner(*iter, BufferOwner::OWNED_BY_US);
            NotifyOmxToFillThisOutBuffer(*iter);
            continue; // keep request until we got a new surfacebuffer
        }
        SetCallerToBuffer(buffer->GetFileDescriptor());
        HLOGI("bufferId=%u, seq=%u", idleIter->bufferId, buffer->GetSeqNum());
        WrapSurfaceBufferToSlot(*idleIter, buffer, 0, 0);
        if (idleIter == outputBufferPool_.begin()) {
            UpdateFormatFromSurfaceBuffer();
        }
        NotifyOmxToFillThisOutBuffer(*idleIter);
        return;
    }
}

int32_t HDecoder::NotifySurfaceToRenderOutputBuffer(BufferInfo &info)
{
    SCOPED_TRACE_WITH_ID(info.bufferId);
    info.lastFlushTime = chrono::steady_clock::now();
    if (std::abs(lastFlushRate_ - codecRate_) > std::numeric_limits<float>::epsilon()) {
        sptr<BufferExtraData> extraData = new BufferExtraDataImpl();
        extraData->ExtraSet("VIDEO_RATE", codecRate_);
        info.surfaceBuffer->SetExtraData(extraData);
        lastFlushRate_ = codecRate_;
        HLOGI("flush video rate(%d)", static_cast<int32_t>(codecRate_));
    }
    BufferFlushConfig cfg {
        .damage = {.x = 0, .y = 0, .w = info.surfaceBuffer->GetWidth(), .h = info.surfaceBuffer->GetHeight() },
        .timestamp = info.omxBuffer->pts,
    };
    GSError ret = currSurface_.surface_->FlushBuffer(info.surfaceBuffer, -1, cfg);
    if (ret != GSERROR_OK) {
        HLOGW("surface(%" PRIu64 "), FlushBuffer(seq=%u) failed, GSError=%d",
              currSurface_.surface_->GetUniqueId(), info.surfaceBuffer->GetSeqNum(), ret);
        return AVCS_ERR_UNKNOWN;
    }
    ChangeOwner(info, BufferOwner::OWNED_BY_SURFACE);
    return AVCS_ERR_OK;
}

void HDecoder::OnOMXEmptyBufferDone(uint32_t bufferId, BufferOperationMode mode)
{
    SCOPED_TRACE_WITH_ID(bufferId);
    BufferInfo *info = FindBufferInfoByID(OMX_DirInput, bufferId);
    if (info == nullptr) {
        HLOGE("unknown buffer id %u", bufferId);
        return;
    }
    if (info->owner != BufferOwner::OWNED_BY_OMX) {
        HLOGE("wrong ownership: buffer id=%d, owner=%s", bufferId, ToString(info->owner));
        return;
    }
    ChangeOwner(*info, BufferOwner::OWNED_BY_US);

    switch (mode) {
        case KEEP_BUFFER:
            return;
        case RESUBMIT_BUFFER: {
            if (!inputPortEos_) {
                NotifyUserToFillThisInBuffer(*info);
            }
            return;
        }
        default: {
            HLOGE("SHOULD NEVER BE HERE");
            return;
        }
    }
}

void HDecoder::OnReleaseOutputBuffer(const BufferInfo &info)
{
    if (currSurface_.surface_) {
        HLOGI("outBufId = %u, discard by user, pts = %" PRId64, info.bufferId, info.omxBuffer->pts);
    }
}

void HDecoder::OnRenderOutputBuffer(const MsgInfo &msg, BufferOperationMode mode)
{
    if (currSurface_.surface_ == nullptr) {
        HLOGE("can only render in surface mode");
        ReplyErrorCode(msg.id, AVCS_ERR_INVALID_OPERATION);
        return;
    }
    uint32_t bufferId = 0;
    (void)msg.param->GetValue(BUFFER_ID, bufferId);
    SCOPED_TRACE_WITH_ID(bufferId);
    optional<size_t> idx = FindBufferIndexByID(OMX_DirOutput, bufferId);
    if (!idx.has_value()) {
        ReplyErrorCode(msg.id, AVCS_ERR_INVALID_VAL);
        return;
    }
    BufferInfo& info = outputBufferPool_[idx.value()];
    if (info.owner != BufferOwner::OWNED_BY_USER) {
        HLOGE("wrong ownership: buffer id=%d, owner=%s", bufferId, ToString(info.owner));
        ReplyErrorCode(msg.id, AVCS_ERR_INVALID_VAL);
        return;
    }
    info.omxBuffer->pts = info.avBuffer->pts_;
    ChangeOwner(info, BufferOwner::OWNED_BY_US);
    ReplyErrorCode(msg.id, AVCS_ERR_OK);

    NotifySurfaceToRenderOutputBuffer(info);
    if (mode == FREE_BUFFER) {
        EraseBufferFromPool(OMX_DirOutput, idx.value());
    } else {
        SubmitDynamicBufferIfPossible();
    }
}

void HDecoder::OnEnterUninitializedState()
{
    currSurface_.Release();
}

HDecoder::SurfaceItem::SurfaceItem(const sptr<Surface> &surface)
    : surface_(surface), originalTransform_(surface->GetTransform()) {}

void HDecoder::SurfaceItem::Release()
{
    if (surface_) {
        LOGI("before release surface(%" PRIu64 "), refCnt=%d",
             surface_->GetUniqueId(), surface_->GetSptrRefCount());
        if (originalTransform_.has_value()) {
            surface_->SetTransform(originalTransform_.value());
            originalTransform_ = std::nullopt;
        }
        surface_ = nullptr;
    }
}

int32_t HDecoder::OnSetOutputSurfaceWhenRunning(const sptr<Surface> &newSurface)
{
    SCOPED_TRACE();
    if (currSurface_.surface_ == nullptr) {
        HLOGE("can only switch surface on surface mode");
        return AVCS_ERR_INVALID_OPERATION;
    }
    if (newSurface == nullptr) {
        HLOGE("surface is null");
        return AVCS_ERR_INVALID_VAL;
    }
    if (newSurface->IsConsumer()) {
        HLOGE("expect a producer surface but got a consumer surface");
        return AVCS_ERR_INVALID_VAL;
    }
    uint64_t oldId = currSurface_.surface_->GetUniqueId();
    uint64_t newId = newSurface->GetUniqueId();
    HLOGI("surface %" PRIu64 " -> %" PRIu64, oldId, newId);
    if (oldId == newId) {
        HLOGI("same surface, no need to set again");
        return AVCS_ERR_OK;
    }
    int32_t ret = RegisterListenerToSurface(newSurface);
    if (ret != AVCS_ERR_OK) {
        return ret;
    }
    ret = SetQueueSize(newSurface, outBufferCnt_);
    if (ret != AVCS_ERR_OK) {
        return ret;
    }
    currSurface_.surface_->CleanCache(true); // make sure old surface is empty and go black
    newSurface->Connect(); // cleancache will work only if the surface is connected by us
    newSurface->CleanCache(); // make sure new surface is empty
    ret = AttachToNewSurface(newSurface);
    if (ret != AVCS_ERR_OK) {
        return ret;
    }

    currSurface_.Release();
    currSurface_ = SurfaceItem(newSurface);
    SetTransform();
    SetScaleMode();
    HLOGI("set surface(%" PRIu64 ")(%s) succ", newId, newSurface->GetName().c_str());
    return AVCS_ERR_OK;
}

int32_t HDecoder::AttachToNewSurface(const sptr<Surface> &newSurface)
{
    uint64_t newId = newSurface->GetUniqueId();
    for (BufferInfo& info : outputBufferPool_) {
        if (info.surfaceBuffer == nullptr) {
            continue;
        }
        GSError err = newSurface->AttachBufferToQueue(info.surfaceBuffer);
        if (err != GSERROR_OK) {
            HLOGE("surface(%" PRIu64 "), AttachBufferToQueue(seq=%u) failed, GSError=%d",
                  newId, info.surfaceBuffer->GetSeqNum(), err);
            return AVCS_ERR_UNKNOWN;
        }
        if (info.owner == OWNED_BY_SURFACE) {
            ChangeOwner(info, BufferOwner::OWNED_BY_US);
        }
        if ((info.owner == OWNED_BY_US) && (currState_->GetName() == "Running")) {
            NotifyOmxToFillThisOutBuffer(info);
        }
    }
    return AVCS_ERR_OK;
}

int32_t HDecoder::PushBlankBufferToCurrSurface()
{
    BufferRequestConfig reqCfg {
        .width = 1,
        .height = 1,
        .strideAlignment = STRIDE_ALIGNMENT,
        .format = GRAPHIC_PIXEL_FMT_RGBA_8888,
        .usage = 0,
        .timeout = 0,
    };

    sptr<SurfaceBuffer> buffer;
    int32_t fence = -1;
    GSError err = currSurface_.surface_->RequestBuffer(buffer, fence, reqCfg);
    if (err != GSERROR_OK) {
        HLOGW("surface(%" PRIu64 "), RequestBuffer failed, GSError=%d",
                currSurface_.surface_->GetUniqueId(), err);
        return AVCS_ERR_UNKNOWN;
    }
    BufferFlushConfig flushCfg {
        .damage = {},
        .timestamp = 0,
    };
    err = currSurface_.surface_->FlushBuffer(buffer, fence, flushCfg);
    if (err != GSERROR_OK) {
        HLOGW("surface(%" PRIu64 "), FlushBuffer failed, GSError=%d",
                currSurface_.surface_->GetUniqueId(), err);
        return AVCS_ERR_UNKNOWN;
    }

    return AVCS_ERR_OK;
}
} // namespace OHOS::MediaAVCodec