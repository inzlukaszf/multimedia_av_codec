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

#include <string>
#include "gtest/gtest.h"
#include "av_codec_sample_error.h"
#include "sample_helper.h"

namespace {
using namespace testing::ext;

constexpr int32_t FRAME_INTERVAL_33MS = 33;        // 33ms
constexpr int32_t FRAME_INTERVAL_50MS = 50;        // 50ms

constexpr std::string_view FILE_AVC_1280_720_30_10M   = "/data/test/media/1280_720_10M_30.h264";
constexpr std::string_view FILE_AVC_1280_720_60_10M   = "/data/test/media/1280_720_10M_60.h264";
constexpr std::string_view FILE_AVC_1920_1080_30_20M  = "/data/test/media/1920_1080_20M_30.h264";
constexpr std::string_view FILE_AVC_1920_1080_60_20M  = "/data/test/media/1920_1080_20M_60.h264";
constexpr std::string_view FILE_AVC_3840_2160_30_30M  = "/data/test/media/3840_2160_30M_30.h264";
constexpr std::string_view FILE_AVC_3840_2160_60_30M  = "/data/test/media/3840_2160_30M_60.h264";

constexpr std::string_view FILE_HEVC_1280_720_30_10M  = "/data/test/media/1280_720_10M_30.h265";
constexpr std::string_view FILE_HEVC_1280_720_60_10M  = "/data/test/media/1280_720_10M_60.h265";
constexpr std::string_view FILE_HEVC_1920_1080_30_20M = "/data/test/media/1920_1080_20M_30.h265";
constexpr std::string_view FILE_HEVC_1920_1080_60_20M = "/data/test/media/1920_1080_20M_60.h265";
constexpr std::string_view FILE_HEVC_3840_2160_30_30M = "/data/test/media/3840_2160_30M_30.h265";
constexpr std::string_view FILE_HEVC_3840_2160_60_30M = "/data/test/media/3840_2160_30M_60.h265";

constexpr std::string_view FILE_1280_720  = "/data/test/media/1280_720_nv.yuv";
constexpr std::string_view FILE_1920_1080 = "/data/test/media/1920_1080_nv.yuv";
constexpr std::string_view FILE_3840_2160 = "/data/test/media/3840_2160_nv.yuv";
} // namespace

namespace OHOS::MediaAVCodec::Sample {
class VideoPerfTestSuilt : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp(void) {};
    void TearDown(void) {};
};

/*  Generate case name
 *
 *  Example:
 *  GENERATE_CASE_NAME(FRAME_DELAY, BUFFER_SHARED_MEMORY, AVC, 1280, 720, 30, 10)
 *
 *  To be:
 *  CASE_FRAME_DELAY_BUFFER_SHARED_MEMORY_AVC_1280_720_30_10M
 */
#define GENERATE_CASE_NAME(codecType, testMode, codecRunMode, mime, width, height, fps, bitrate)                    \
    CASE_##codecType##_##testMode##_##codecRunMode##_##mime##_##width##_##height##_##fps##_##bitrate##M

/*  Template of perf test case
 *
 *  Example:
 *  ADD_CASE(FRAME_DELAY, BUFFER_SHARED_MEMORY, AVC, 1280, 720, 30, 10, FRAME_INTERVAL_33MS)
 *
 *  To be:
 *  HWTEST_F(VideoDecoderPerfTestSuilt, CASE_FRAME_DELAY_BUFFER_SHARED_MEMORY_AVC_1280_720_30_10M, TestSize.Level1)
 *  {
 *      std::string inputFileName = (codecType) == CodecType::VIDEO_DECODER ?
 *          FILE_AVC_1280_720_30_10M.data() : FILE_1280_720.data();
 *      SampleInfo sampleInfo = {
 *          inputFileName, MIME_VIDEO_AVC, 1280, 720, 30, BITRATE_10M,
 *          CodecRunMode::BUFFER_SHARED_MEMORY, TestMode::FRAME_DELAY, FRAME_INTERVAL_33MS
 *      };
 *      ASSERT_EQ(AVCODEC_SAMPLE_ERR_OK, RunSample(sampleInfo));
 *  }
 */
#define ADD_CASE(codecType, testMode, codecRunMode, mime, width, height, fps, bitrate, inteval)                        \
HWTEST_F(VideoPerfTestSuilt,                                                                                           \
GENERATE_CASE_NAME(codecType, testMode, codecRunMode, mime, width, height, fps, bitrate), TestSize.Level1)             \
{                                                                                                                      \
    std::string inputFileName = (codecType) == CodecType::VIDEO_DECODER ?                                              \
        FILE_##mime##_##width##_##height##_##fps##_##bitrate##M.data() : FILE_##width##_##height.data();               \
    SampleInfo sampleInfo = {                                                                                          \
        codecType, inputFileName,                                                                                      \
        MIME_VIDEO_##mime.data(), (width), (height), (fps), BITRATE_##bitrate##M, (codecRunMode), (inteval)            \
    };                                                                                                                 \
    ASSERT_EQ(AVCODEC_SAMPLE_ERR_OK, RunSample(sampleInfo));                                                           \
}

ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_ORIGIN,       AVC,  1280,  720, 30, 10, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, AVC,  1280,  720, 30, 10, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_AVBUFFER,     AVC,  1280,  720, 30, 10, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_AVBUFFER,      AVC,  1280,  720, 30, 10, FRAME_INTERVAL_33MS)

ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_ORIGIN,       AVC,  1280,  720, 60, 10, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, AVC,  1280,  720, 60, 10, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_AVBUFFER,     AVC,  1280,  720, 60, 10, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_AVBUFFER,      AVC,  1280,  720, 60, 10, FRAME_INTERVAL_33MS)

ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_ORIGIN,       AVC,  1920, 1080, 30, 20, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, AVC,  1920, 1080, 30, 20, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_AVBUFFER,     AVC,  1920, 1080, 30, 20, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_AVBUFFER,      AVC,  1920, 1080, 30, 20, FRAME_INTERVAL_33MS)

ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_ORIGIN,       AVC,  1920, 1080, 60, 20, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, AVC,  1920, 1080, 60, 20, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_AVBUFFER,     AVC,  1920, 1080, 60, 20, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_AVBUFFER,      AVC,  1920, 1080, 60, 20, FRAME_INTERVAL_33MS)

ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_ORIGIN,       AVC,  3840, 2160, 30, 30, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, AVC,  3840, 2160, 30, 30, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_AVBUFFER,     AVC,  3840, 2160, 30, 30, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_AVBUFFER,      AVC,  3840, 2160, 30, 30, FRAME_INTERVAL_33MS)

ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_ORIGIN,       AVC,  3840, 2160, 60, 30, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, AVC,  3840, 2160, 60, 30, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_AVBUFFER,     AVC,  3840, 2160, 60, 30, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_AVBUFFER,      AVC,  3840, 2160, 60, 30, FRAME_INTERVAL_33MS)

ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_ORIGIN,       HEVC, 1280,  720, 30, 10, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, HEVC, 1280,  720, 30, 10, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_AVBUFFER,     HEVC, 1280,  720, 30, 10, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_AVBUFFER,      HEVC, 1280,  720, 30, 10, FRAME_INTERVAL_33MS)

ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_ORIGIN,       HEVC, 1280,  720, 60, 10, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, HEVC, 1280,  720, 60, 10, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_AVBUFFER,     HEVC, 1280,  720, 60, 10, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_AVBUFFER,      HEVC, 1280,  720, 60, 10, FRAME_INTERVAL_33MS)

ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_ORIGIN,       HEVC, 1920, 1080, 30, 20, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, HEVC, 1920, 1080, 30, 20, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_AVBUFFER,     HEVC, 1920, 1080, 30, 20, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_AVBUFFER,      HEVC, 1920, 1080, 30, 20, FRAME_INTERVAL_33MS)

ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_ORIGIN,       HEVC, 1920, 1080, 60, 20, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, HEVC, 1920, 1080, 60, 20, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_AVBUFFER,     HEVC, 1920, 1080, 60, 20, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_AVBUFFER,      HEVC, 1920, 1080, 60, 20, FRAME_INTERVAL_33MS)

ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_ORIGIN,       HEVC, 3840, 2160, 30, 30, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, HEVC, 3840, 2160, 30, 30, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_AVBUFFER,     HEVC, 3840, 2160, 30, 30, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_AVBUFFER,      HEVC, 3840, 2160, 30, 30, FRAME_INTERVAL_33MS)

ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_ORIGIN,       HEVC, 3840, 2160, 60, 30, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, HEVC, 3840, 2160, 60, 30, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, SURFACE_AVBUFFER,     HEVC, 3840, 2160, 60, 30, FRAME_INTERVAL_33MS)
ADD_CASE(VIDEO_DECODER, FRAME_DELAY, BUFFER_AVBUFFER,      HEVC, 3840, 2160, 60, 30, FRAME_INTERVAL_33MS)

ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_ORIGIN,       AVC,  1280,  720, 30, 10, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, AVC,  1280,  720, 30, 10, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_AVBUFFER,     AVC,  1280,  720, 30, 10, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_AVBUFFER,      AVC,  1280,  720, 30, 10, 0)

ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_ORIGIN,       AVC,  1280,  720, 60, 10, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, AVC,  1280,  720, 60, 10, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_AVBUFFER,     AVC,  1280,  720, 60, 10, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_AVBUFFER,      AVC,  1280,  720, 60, 10, 0)

ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_ORIGIN,       AVC,  1920, 1080, 30, 20, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, AVC,  1920, 1080, 30, 20, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_AVBUFFER,     AVC,  1920, 1080, 30, 20, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_AVBUFFER,      AVC,  1920, 1080, 30, 20, 0)

ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_ORIGIN,       AVC,  1920, 1080, 60, 20, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, AVC,  1920, 1080, 60, 20, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_AVBUFFER,     AVC,  1920, 1080, 60, 20, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_AVBUFFER,      AVC,  1920, 1080, 60, 20, 0)

ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_ORIGIN,       AVC,  3840, 2160, 30, 30, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, AVC,  3840, 2160, 30, 30, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_AVBUFFER,     AVC,  3840, 2160, 30, 30, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_AVBUFFER,      AVC,  3840, 2160, 30, 30, 0)

ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_ORIGIN,       AVC,  3840, 2160, 60, 30, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, AVC,  3840, 2160, 60, 30, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_AVBUFFER,     AVC,  3840, 2160, 60, 30, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_AVBUFFER,      AVC,  3840, 2160, 60, 30, 0)

ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_ORIGIN,       HEVC, 1280,  720, 30, 10, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, HEVC, 1280,  720, 30, 10, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_AVBUFFER,     HEVC, 1280,  720, 30, 10, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_AVBUFFER,      HEVC, 1280,  720, 30, 10, 0)

ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_ORIGIN,       HEVC, 1280,  720, 60, 10, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, HEVC, 1280,  720, 60, 10, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_AVBUFFER,     HEVC, 1280,  720, 60, 10, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_AVBUFFER,      HEVC, 1280,  720, 60, 10, 0)

ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_ORIGIN,       HEVC, 1920, 1080, 30, 20, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, HEVC, 1920, 1080, 30, 20, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_AVBUFFER,     HEVC, 1920, 1080, 30, 20, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_AVBUFFER,      HEVC, 1920, 1080, 30, 20, 0)

ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_ORIGIN,       HEVC, 1920, 1080, 60, 20, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, HEVC, 1920, 1080, 60, 20, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_AVBUFFER,     HEVC, 1920, 1080, 60, 20, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_AVBUFFER,      HEVC, 1920, 1080, 60, 20, 0)

ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_ORIGIN,       HEVC, 3840, 2160, 30, 30, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, HEVC, 3840, 2160, 30, 30, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_AVBUFFER,     HEVC, 3840, 2160, 30, 30, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_AVBUFFER,      HEVC, 3840, 2160, 30, 30, 0)

ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_ORIGIN,       HEVC, 3840, 2160, 60, 30, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, HEVC, 3840, 2160, 60, 30, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  SURFACE_AVBUFFER,     HEVC, 3840, 2160, 60, 30, 0)
ADD_CASE(VIDEO_DECODER, FRAME_RATE,  BUFFER_AVBUFFER,      HEVC, 3840, 2160, 60, 30, 0)

ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, SURFACE_ORIGIN,        AVC, 1280,  720, 30, 10, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_SHARED_MEMORY,  AVC, 1280,  720, 30, 10, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_AVBUFFER,       AVC, 1280,  720, 30, 10, FRAME_INTERVAL_50MS)

ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, SURFACE_ORIGIN,        AVC, 1280,  720, 60, 10, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_SHARED_MEMORY,  AVC, 1280,  720, 60, 10, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_AVBUFFER,       AVC, 1280,  720, 60, 10, FRAME_INTERVAL_50MS)

ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, SURFACE_ORIGIN,        AVC, 1920, 1080, 30, 20, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_SHARED_MEMORY,  AVC, 1920, 1080, 30, 20, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_AVBUFFER,       AVC, 1920, 1080, 30, 20, FRAME_INTERVAL_50MS)

ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, SURFACE_ORIGIN,        AVC, 1920, 1080, 60, 20, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_SHARED_MEMORY,  AVC, 1920, 1080, 60, 20, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_AVBUFFER,       AVC, 1920, 1080, 60, 20, FRAME_INTERVAL_50MS)

ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, SURFACE_ORIGIN,        AVC, 3840, 2160, 30, 30, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_SHARED_MEMORY,  AVC, 3840, 2160, 30, 30, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_AVBUFFER,       AVC, 3840, 2160, 30, 30, FRAME_INTERVAL_50MS)

ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, SURFACE_ORIGIN,        AVC, 3840, 2160, 60, 30, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_SHARED_MEMORY,  AVC, 3840, 2160, 60, 30, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_AVBUFFER,       AVC, 3840, 2160, 60, 30, FRAME_INTERVAL_50MS)

ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, SURFACE_ORIGIN,       HEVC, 1280,  720, 30, 10, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, HEVC, 1280,  720, 30, 10, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_AVBUFFER,      HEVC, 1280,  720, 30, 10, FRAME_INTERVAL_50MS)

ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, SURFACE_ORIGIN,       HEVC, 1280,  720, 60, 10, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, HEVC, 1280,  720, 60, 10, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_AVBUFFER,      HEVC, 1280,  720, 60, 10, FRAME_INTERVAL_50MS)

ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, SURFACE_ORIGIN,       HEVC, 1920, 1080, 30, 20, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, HEVC, 1920, 1080, 30, 20, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_AVBUFFER,      HEVC, 1920, 1080, 30, 20, FRAME_INTERVAL_50MS)

ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, SURFACE_ORIGIN,       HEVC, 1920, 1080, 60, 20, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, HEVC, 1920, 1080, 60, 20, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_AVBUFFER,      HEVC, 1920, 1080, 60, 20, FRAME_INTERVAL_50MS)

ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, SURFACE_ORIGIN,       HEVC, 3840, 2160, 30, 30, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, HEVC, 3840, 2160, 30, 30, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_AVBUFFER,      HEVC, 3840, 2160, 30, 30, FRAME_INTERVAL_50MS)

ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, SURFACE_ORIGIN,       HEVC, 3840, 2160, 60, 30, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_SHARED_MEMORY, HEVC, 3840, 2160, 60, 30, FRAME_INTERVAL_50MS)
ADD_CASE(VIDEO_ENCODER, FRAME_DELAY, BUFFER_AVBUFFER,      HEVC, 3840, 2160, 60, 30, FRAME_INTERVAL_50MS)

ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  SURFACE_ORIGIN,        AVC, 1280,  720, 30, 10, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_SHARED_MEMORY,  AVC, 1280,  720, 30, 10, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_AVBUFFER,       AVC, 1280,  720, 30, 10, 0)

ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  SURFACE_ORIGIN,        AVC, 1280,  720, 60, 10, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_SHARED_MEMORY,  AVC, 1280,  720, 60, 10, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_AVBUFFER,       AVC, 1280,  720, 60, 10, 0)

ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  SURFACE_ORIGIN,        AVC, 1920, 1080, 30, 20, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_SHARED_MEMORY,  AVC, 1920, 1080, 30, 20, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_AVBUFFER,       AVC, 1920, 1080, 30, 20, 0)

ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  SURFACE_ORIGIN,        AVC, 1920, 1080, 60, 20, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_SHARED_MEMORY,  AVC, 1920, 1080, 60, 20, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_AVBUFFER,       AVC, 1920, 1080, 60, 20, 0)

ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  SURFACE_ORIGIN,        AVC, 3840, 2160, 30, 30, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_SHARED_MEMORY,  AVC, 3840, 2160, 30, 30, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_AVBUFFER,       AVC, 3840, 2160, 30, 30, 0)

ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  SURFACE_ORIGIN,        AVC, 3840, 2160, 60, 30, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_SHARED_MEMORY,  AVC, 3840, 2160, 60, 30, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_AVBUFFER,       AVC, 3840, 2160, 60, 30, 0)

ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  SURFACE_ORIGIN,       HEVC, 1280,  720, 30, 10, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, HEVC, 1280,  720, 30, 10, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_AVBUFFER,      HEVC, 1280,  720, 30, 10, 0)

ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  SURFACE_ORIGIN,       HEVC, 1280,  720, 60, 10, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, HEVC, 1280,  720, 60, 10, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_AVBUFFER,      HEVC, 1280,  720, 60, 10, 0)

ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  SURFACE_ORIGIN,       HEVC, 1920, 1080, 30, 20, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, HEVC, 1920, 1080, 30, 20, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_AVBUFFER,      HEVC, 1920, 1080, 30, 20, 0)

ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  SURFACE_ORIGIN,       HEVC, 1920, 1080, 60, 20, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, HEVC, 1920, 1080, 60, 20, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_AVBUFFER,      HEVC, 1920, 1080, 60, 20, 0)

ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  SURFACE_ORIGIN,       HEVC, 3840, 2160, 30, 30, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, HEVC, 3840, 2160, 30, 30, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_AVBUFFER,      HEVC, 3840, 2160, 30, 30, 0)

ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  SURFACE_ORIGIN,       HEVC, 3840, 2160, 60, 30, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_SHARED_MEMORY, HEVC, 3840, 2160, 60, 30, 0)
ADD_CASE(VIDEO_ENCODER, FRAME_RATE,  BUFFER_AVBUFFER,      HEVC, 3840, 2160, 60, 30, 0)
}   // namespace OHOS::MediaAVCodec::Sample