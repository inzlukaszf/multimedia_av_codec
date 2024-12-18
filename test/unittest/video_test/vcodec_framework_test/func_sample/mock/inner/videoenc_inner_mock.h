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

#ifndef VIDEO_ENC_INNER_MOCK_H
#define VIDEO_ENC_INNER_MOCK_H

#include "vcodec_mock.h"
#include "avcodec_video_encoder.h"

namespace OHOS {
namespace MediaAVCodec {
class VideoEncInnerMock : public VideoEncMock {
public:
    explicit VideoEncInnerMock(std::shared_ptr<AVCodecVideoEncoder> videoEnc) : videoEnc_(videoEnc) {}
    int32_t SetCallback(std::shared_ptr<AVCodecCallbackMock> cb) override;
    int32_t SetCallback(std::shared_ptr<MediaCodecCallbackMock> cb) override;
    int32_t SetCallback(std::shared_ptr<MediaCodecParameterCallbackMock> cb) override;
    int32_t SetCallback(std::shared_ptr<MediaCodecParameterWithAttrCallbackMock> cb) override;
    int32_t Configure(std::shared_ptr<FormatMock> format) override;
    int32_t Prepare() override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t Flush() override;
    int32_t Reset() override;
    int32_t Release() override;
    std::shared_ptr<FormatMock> GetOutputDescription() override;
    std::shared_ptr<FormatMock> GetInputDescription() override;
    int32_t SetParameter(std::shared_ptr<FormatMock> format) override;
    int32_t FreeOutputData(uint32_t index) override;
    int32_t NotifyEos() override;
    int32_t PushInputData(uint32_t index, OH_AVCodecBufferAttr &attr) override;
    int32_t PushInputBuffer(uint32_t index) override;
    int32_t PushInputParameter(uint32_t index) override;
    int32_t FreeOutputBuffer(uint32_t index) override;
    std::shared_ptr<SurfaceMock> CreateInputSurface() override;
    bool IsValid() override;
    int32_t SetCustomBuffer(std::shared_ptr<AVBufferMock> buffer) override;

private:
    std::shared_ptr<AVCodecVideoEncoder> videoEnc_ = nullptr;
};

class VideoEncCallbackMock : public AVCodecCallback {
public:
    explicit VideoEncCallbackMock(std::shared_ptr<AVCodecCallbackMock> cb);
    ~VideoEncCallbackMock() = default;
    void OnError(AVCodecErrorType errorType, int32_t errorCode) override;
    void OnOutputFormatChanged(const Format &format) override;
    void OnInputBufferAvailable(uint32_t index, std::shared_ptr<AVSharedMemory> buffer) override;
    void OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag,
                                 std::shared_ptr<AVSharedMemory> buffer) override;

private:
    std::shared_ptr<AVCodecCallbackMock> mockCb_ = nullptr;
};

class VideoEncCallbackExtMock : public MediaCodecCallback {
public:
    explicit VideoEncCallbackExtMock(std::shared_ptr<MediaCodecCallbackMock> cb);
    ~VideoEncCallbackExtMock() = default;
    void OnError(AVCodecErrorType errorType, int32_t errorCode) override;
    void OnOutputFormatChanged(const Format &format) override;
    void OnInputBufferAvailable(uint32_t index, std::shared_ptr<AVBuffer> buffer) override;
    void OnOutputBufferAvailable(uint32_t index, std::shared_ptr<AVBuffer> buffer) override;

private:
    std::shared_ptr<MediaCodecCallbackMock> mockCb_ = nullptr;
};

class VideoEncParamCallbackMock : public MediaCodecParameterCallback {
public:
    explicit VideoEncParamCallbackMock(std::shared_ptr<MediaCodecParameterCallbackMock> cb);
    ~VideoEncParamCallbackMock() = default;
    void OnInputParameterAvailable(uint32_t index, std::shared_ptr<Format> parameter) override;

private:
    std::shared_ptr<MediaCodecParameterCallbackMock> mockCb_ = nullptr;
};

class VideoEncParamWithAttrCallbackMock : public MediaCodecParameterWithAttrCallback {
public:
    explicit VideoEncParamWithAttrCallbackMock(std::shared_ptr<MediaCodecParameterWithAttrCallbackMock> cb);
    ~VideoEncParamWithAttrCallbackMock() = default;
    void OnInputParameterWithAttrAvailable(uint32_t index, std::shared_ptr<Format> attribute,
                                           std::shared_ptr<Format> parameter) override;

private:
    std::shared_ptr<MediaCodecParameterWithAttrCallbackMock> mockCb_ = nullptr;
};
} // namespace MediaAVCodec
} // namespace OHOS
#endif // VIDEO_ENC_INNER_MOCK_H