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

#ifndef MEDIA_AVCODEC_VIDEO_DECODER_H
#define MEDIA_AVCODEC_VIDEO_DECODER_H

#include "avcodec_common.h"
#include "avcodec_info.h"
#include "buffer/avsharedmemory.h"
#include "meta/format.h"
#include "surface.h"
#include "foundation/multimedia/drm_framework/services/drm_service/ipc/i_keysession_service.h"

namespace OHOS {
namespace MediaAVCodec {
class AVCodecVideoDecoder {
public:
    virtual ~AVCodecVideoDecoder() = default;

    /**
     * @brief Configure the decoder.
     *
     * @param format The format of the input data and the desired format of the output data.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t Configure(const Format &format) = 0;

    /**
     * @brief Prepare for decoding.
     *
     * This function must be called after {@link Configure} and before {@link Start}
     *
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t Prepare() = 0;

    /**
     * @brief Start decoding.
     *
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t Start() = 0;

    /**
     * @brief Stop decoding.
     *
     * This function must be called during running
     *
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t Stop() = 0;

    /**
     * @brief Flush both input and output buffers of the decoder.
     *
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t Flush() = 0;

    /**
     * @brief Restores the decoder to the initial state.
     *
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t Reset() = 0;

    /**
     * @brief Releases decoder resources. All methods are unavailable after calling this.
     *
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t Release() = 0;

    /**
     * @brief Sets the surface on which to render the output of this decoder.
     *
     * This function must be called before {@link Prepare}
     *
     * @param index The index of the output buffer.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t SetOutputSurface(sptr<Surface> surface) = 0;

    /**
     * @brief Submits input buffer to decoder.
     *
     * This function must be called during running
     *
     * @param index The index of the input buffer.
     * @param info The info of the input buffer. For details, see {@link AVCodecBufferInfo}
     * @param flag The flag of the input buffer. For details, see {@link AVCodecBufferFlag}
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) = 0;

    /**
     * @brief Submits input buffer to decoder.
     *
     * This function must be called during running
     *
     * @param index The index of the input buffer.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t QueueInputBuffer(uint32_t index) = 0;

    /**
     * @brief Gets the format of the output data.
     *
     * This function must be called after {@link Configure}
     *
     * @param format
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t GetOutputFormat(Format &format) = 0;

    /**
     * @brief Returns the output buffer to the decoder.
     *
     * This function must be called during running
     *
     * @param index The index of the output buffer.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t ReleaseOutputBuffer(uint32_t index, bool render) = 0;

    /**
     * @brief Sets the parameters to the decoder.
     *
     * This function must be called after {@link Configure}
     *
     * @param format The parameters.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t SetParameter(const Format &format) = 0;

    /**
     * @brief Registers a decoder listener.
     *
     * This function must be called before {@link Configure}
     *
     * @param callback Indicates the decoder listener to register. For details, see {@link AVCodecCallback}.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 3.1
     * @version 3.1
     */
    virtual int32_t SetCallback(const std::shared_ptr<AVCodecCallback> &callback) = 0;

    /**
     * @brief Registers a decoder listener.
     *
     * This function must be called before {@link Configure}
     *
     * @param callback Indicates the decoder listener to register. For details, see {@link MediaCodecCallback}.
     * @return Returns {@link AVCS_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t SetCallback(const std::shared_ptr<MediaCodecCallback> &callback) = 0;

    /*
     * @brief Set media key session which includes a decrypt module and a svp flag for decrypt video.
     * @param svp is the flag whether use secure decoder
     * @return Returns AV_ERR_OK if the execution is successful,
     * otherwise returns a specific error code, refer to {@link OH_AVErrCode}
     * @since
     */
    virtual int32_t SetDecryptConfig(const sptr<DrmStandard::IMediaKeySessionService> &keySession,
        const bool svpFlag)
    {
        (void)keySession;
        (void)svpFlag;
        return 0;
    }
};

class __attribute__((visibility("default"))) VideoDecoderFactory {
public:
#ifdef UNSUPPORT_CODEC
    static std::shared_ptr<AVCodecVideoDecoder> CreateByMime(const std::string &mime)
    {
        (void)mime;
        return nullptr;
    }

    static std::shared_ptr<AVCodecVideoDecoder> CreateByName(const std::string &name)
    {
        (void)name;
        return nullptr;
    }
#else
    /**
     * @brief Instantiate the preferred decoder of the given mime type.
     *
     * @param mime The mime type.
     * @return Returns the preferred decoder.
     * @since 3.1
     * @version 3.1
     */
    static std::shared_ptr<AVCodecVideoDecoder> CreateByMime(const std::string &mime);

    /**
     * @brief Instantiates the designated decoder.
     *
     * @param name The decoder's name.
     * @return Returns the designated decoder.
     * @since 3.1
     * @version 3.1
     */
    static std::shared_ptr<AVCodecVideoDecoder> CreateByName(const std::string &name);
#endif
private:
    VideoDecoderFactory() = default;
    ~VideoDecoderFactory() = default;
};
} // namespace MediaAVCodec
} // namespace OHOS
#endif // MEDIA_AVCODEC_VIDEO_DECODER_H