/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <cstdio>
#include <atomic>
#include <fstream>
#include <thread>
#include <mutex>
#include <queue>
#include <string>

#include "gtest/gtest.h"
#include "videodec_sample.h"
#include "native_avcodec_videodecoder.h"
#include "native_avcodec_base.h"
#include "native_avcapability.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;
using namespace testing::ext;
namespace {
string g_codecName;
string g_codecNameHEVC;
OH_AVCapability *cap = nullptr;
OH_AVCapability *cap_hevc = nullptr;
constexpr uint32_t MAX_THREAD = 16;
} // namespace
namespace OHOS {
namespace Media {
class HwdecPerfNdkTest : public testing::Test {
public:
    static void SetUpTestCase();    // 第一个测试用例执行前
    static void TearDownTestCase(); // 最后一个测试用例执行后
    void SetUp() override;          // 每个测试用例执行前
    void TearDown() override;       // 每个测试用例执行后
    void InputFunc();
    void OutputFunc();
    void Release();
    int64_t GetSystemTimeUs();
    int32_t Stop();

protected:
    OH_AVCodec *vdec_;
    bool createCodecSuccess_ = false;
    const char *INP_DIR = "/data/test/media/1920x1080_30_10M.h264";
    const char *INP_DIR_720_30 = "/data/test/media/1280x720_30_10M.h264";
    const char *INP_DIR_1080_30 = "/data/test/media/1920x1080_30_10M.h264";
    const char *INP_DIR_2160_30 = "/data/test/media/3840x2160_30_50M.h264";
    const char *INP_DIR_720_60 = "/data/test/media/1280x720_60_10Mb.h264";
    const char *INP_DIR_1080_60 = "/data/test/media/1920x1080_60_20Mb.h264";
    const char *INP_DIR_2160_60 = "/data/test/media/3840x2160_60_50Mb.h264";

    const char *INP_DIR_720_30_1 = "/data/test/media/1280x720_30_10M_1.h264";
    const char *INP_DIR_1080_30_1 = "/data/test/media/1920x1080_30_10M_1.h264";
    const char *INP_DIR_2160_30_1 = "/data/test/media/3840x2160_30_50M_1.h264";
    const char *INP_DIR_720_60_1 = "/data/test/media/1280x720_60_10Mb_1.h264";
    const char *INP_DIR_1080_60_1 = "/data/test/media/1920x1080_60_20Mb_1.h264";
    const char *INP_DIR_2160_60_1 = "/data/test/media/3840x2160_60_50Mb_1.h264";

    const char *INP_DIR_720_30_264 = "/data/test/media/1280_720_10M_30.h264";
    const char *INP_DIR_720_60_264 = "/data/test/media/1280_720_10M_60.h264";
    const char *INP_DIR_1080_30_264 = "/data/test/media/1920_1080_20M_30.h264";
    const char *INP_DIR_1080_60_264 = "/data/test/media/1920_1080_20M_60.h264";
    const char *INP_DIR_2160_30_264 = "/data/test/media/3840_2160_30M_30.h264";
    const char *INP_DIR_2160_60_264 = "/data/test/media/3840_2160_30M_60.h264";

    const char *INP_DIR_720_30_265 = "/data/test/media/1280_720_10M_30.h265";
    const char *INP_DIR_720_60_265 = "/data/test/media/1280_720_10M_60.h265";
    const char *INP_DIR_1080_30_265 = "/data/test/media/1920_1080_20M_30.h265";
    const char *INP_DIR_1080_60_265 = "/data/test/media/1920_1080_20M_60.h265";
    const char *INP_DIR_2160_30_265 = "/data/test/media/3840_2160_30M_30.h265";
    const char *INP_DIR_2160_60_265 = "/data/test/media/3840_2160_30M_60.h265";
    int64_t NANOS_IN_SECOND = 1000000000L;
    int64_t NANOS_IN_MICRO = 1000L;
};
} // namespace Media
} // namespace OHOS
void HwdecPerfNdkTest::SetUpTestCase()
{
    cap = OH_AVCodec_GetCapabilityByCategory(OH_AVCODEC_MIMETYPE_VIDEO_AVC, false, HARDWARE);
    g_codecName = OH_AVCapability_GetName(cap);
    cap_hevc = OH_AVCodec_GetCapabilityByCategory(OH_AVCODEC_MIMETYPE_VIDEO_HEVC, false, HARDWARE);
    g_codecNameHEVC = OH_AVCapability_GetName(cap_hevc);
}
void HwdecPerfNdkTest::TearDownTestCase() {}
void HwdecPerfNdkTest::SetUp() {}

void HwdecPerfNdkTest::TearDown() {}

int64_t HwdecPerfNdkTest::GetSystemTimeUs()
{
    struct timespec now;
    (void)clock_gettime(CLOCK_BOOTTIME, &now);
    int64_t nanoTime = (int64_t)now.tv_sec * NANOS_IN_SECOND + now.tv_nsec;
    return nanoTime / NANOS_IN_MICRO;
}

namespace {
/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0100
 * @tc.name      : test surface mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0100, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = true;
    vDecSample->INP_DIR = INP_DIR_720_30_264;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0200
 * @tc.name      : test surface mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0200, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = true;
    vDecSample->INP_DIR = INP_DIR_720_60_264;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0300
 * @tc.name      : test surface mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0300, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = true;
    vDecSample->INP_DIR = INP_DIR_1080_30_264;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0400
 * @tc.name      : test surface mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0400, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = true;
    vDecSample->INP_DIR = INP_DIR_1080_60_264;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0500
 * @tc.name      : test surface mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0500, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = true;
    vDecSample->INP_DIR = INP_DIR_2160_30_264;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0600
 * @tc.name      : test surface mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0600, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = true;
    vDecSample->INP_DIR = INP_DIR_2160_60_264;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0700
 * @tc.name      : test surface mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0700, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = true;
    vDecSample->INP_DIR = INP_DIR_720_30_265;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecNameHEVC));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0800
 * @tc.name      : test surface mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0800, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = true;
    vDecSample->INP_DIR = INP_DIR_720_60_265;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecNameHEVC));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0900
 * @tc.name      : test surface mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_0900, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = true;
    vDecSample->INP_DIR = INP_DIR_1080_30_265;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecNameHEVC));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_1000
 * @tc.name      : test surface mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_1000, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = true;
    vDecSample->INP_DIR = INP_DIR_1080_60_265;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecNameHEVC));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_1100
 * @tc.name      : test surface mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_1100, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = true;
    vDecSample->INP_DIR = INP_DIR_2160_30_265;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecNameHEVC));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_1200
 * @tc.name      : test surface mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_SURFACE_1200, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = true;
    vDecSample->INP_DIR = INP_DIR_2160_60_265;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecNameHEVC));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_0100
 * @tc.name      : test buffer mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_0100, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = false;
    vDecSample->INP_DIR = INP_DIR_720_30_264;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_0200
 * @tc.name      : test buffer mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_0200, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = false;
    vDecSample->INP_DIR = INP_DIR_720_60_264;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_0300
 * @tc.name      : test buffer mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_0300, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = false;
    vDecSample->INP_DIR = INP_DIR_1080_30_264;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_0400
 * @tc.name      : test buffer mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_0400, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = false;
    vDecSample->INP_DIR = INP_DIR_1080_60_264;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_0500
 * @tc.name      : test buffer mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_0500, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = false;
    vDecSample->INP_DIR = INP_DIR_2160_30_264;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_0600
 * @tc.name      : test buffer mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_0600, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = false;
    vDecSample->INP_DIR = INP_DIR_2160_60_264;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_0700
 * @tc.name      : test buffer mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_0700, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = false;
    vDecSample->INP_DIR = INP_DIR_720_30_265;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecNameHEVC));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_0800
 * @tc.name      : test buffer mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_0800, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = false;
    vDecSample->INP_DIR = INP_DIR_720_60_265;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecNameHEVC));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_0900
 * @tc.name      : test buffer mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_0900, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = false;
    vDecSample->INP_DIR = INP_DIR_1080_30_265;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecNameHEVC));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_1000
 * @tc.name      : test buffer mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_1000, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = false;
    vDecSample->INP_DIR = INP_DIR_1080_60_265;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecNameHEVC));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_1100
 * @tc.name      : test buffer mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_1100, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = false;
    vDecSample->INP_DIR = INP_DIR_2160_30_265;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecNameHEVC));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MEMORY_1200
 * @tc.name      : test buffer mode memory performance
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MEMORY_1200, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = false;
    vDecSample->INP_DIR = INP_DIR_2160_60_265;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecNameHEVC));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_0300
 * @tc.name      : surface API time test
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_0300, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_1080_60;
    vDecSample->OUT_DIR = "/data/test/media/1920_1080_60_out.rgba";
    vDecSample->SF_OUTPUT = true;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_0400
 * @tc.name      : create 1 decoder(1920*1080 60fps)+2 decoder(1280*720 60fps)
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_0400, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_1080_60;
    vDecSample->SF_OUTPUT = false;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());

    shared_ptr<VDecNdkSample> vDecSample1 = make_shared<VDecNdkSample>();
    vDecSample1->INP_DIR = INP_DIR_720_60;
    vDecSample1->SF_OUTPUT = false;
    vDecSample1->DEFAULT_WIDTH = 1280;
    vDecSample1->DEFAULT_HEIGHT = 720;
    vDecSample1->DEFAULT_FRAME_RATE = 60;
    vDecSample1->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample1->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample1->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->StartVideoDecoder());

    shared_ptr<VDecNdkSample> vDecSample2 = make_shared<VDecNdkSample>();
    vDecSample2->SF_OUTPUT = false;
    vDecSample2->INP_DIR = INP_DIR_720_30;
    vDecSample2->DEFAULT_WIDTH = 1280;
    vDecSample2->DEFAULT_HEIGHT = 720;
    vDecSample2->DEFAULT_FRAME_RATE = 30;
    vDecSample2->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample2->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample2->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample2->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample2->StartVideoDecoder());

    vDecSample->WaitForEOS();
    vDecSample1->WaitForEOS();
    vDecSample2->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
    ASSERT_EQ(AV_ERR_OK, vDecSample1->errCount);
    ASSERT_EQ(AV_ERR_OK, vDecSample2->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_0500
 * @tc.name      : decode YUV time 1280*720 30fps 10M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_0500, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->SF_OUTPUT = false;
    vDecSample->INP_DIR = INP_DIR_720_30;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_0600
 * @tc.name      : decode Surface time 1280*720 30fps 10M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_0600, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_720_30;
    vDecSample->SF_OUTPUT = true;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_0700
 * @tc.name      : decode YUV time 1280*720 60fps 10M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_0700, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_720_60;
    vDecSample->SF_OUTPUT = false;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_0800
 * @tc.name      : decode Surface time 1280*720 60fps 10M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_0800, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_720_60;
    vDecSample->SF_OUTPUT = true;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_0900
 * @tc.name      : decode YUV time 1920*1080 30fps 20M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_0900, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_1080_30;
    vDecSample->SF_OUTPUT = false;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_1000
 * @tc.name      : decode Surface time 1920*1080 30fps 20M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_1000, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_1080_30;
    vDecSample->SF_OUTPUT = true;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_1100
 * @tc.name      : decode YUV time 1920*1080 60fps 20M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_1100, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_1080_60;
    vDecSample->SF_OUTPUT = false;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_1200
 * @tc.name      : decode Surface time 1920*1080 60fps 20M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_1200, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_1080_60;
    vDecSample->SF_OUTPUT = true;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_1300
 * @tc.name      : decode YUV time 3840*2160 30fps 50M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_1300, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_2160_30;
    vDecSample->SF_OUTPUT = false;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_1400
 * @tc.name      : decode Surface time 3840*2160 30fps 50M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_1400, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_2160_30;
    vDecSample->SF_OUTPUT = true;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_1500
 * @tc.name      : decode YUV time 3840*2160 60fps 50M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_1500, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_2160_60;
    vDecSample->SF_OUTPUT = false;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_1600
 * @tc.name      : decode Surface time 3840*2160 60fps 50M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_1600, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_2160_60;
    vDecSample->SF_OUTPUT = true;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));
    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MORE_0500
 * @tc.name      : decode YUV time 1280*720 30fps 10M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MORE_0500, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_720_30_1;
    vDecSample->SF_OUTPUT = false;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());

    shared_ptr<VDecNdkSample> vDecSample1 = make_shared<VDecNdkSample>();
    vDecSample1->INP_DIR = INP_DIR_720_30;
    vDecSample1->SF_OUTPUT = false;
    vDecSample1->DEFAULT_WIDTH = 1280;
    vDecSample1->DEFAULT_HEIGHT = 720;
    vDecSample1->DEFAULT_FRAME_RATE = 30;
    vDecSample1->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample1->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample1->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->StartVideoDecoder());

    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
    vDecSample1->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample1->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MORE_0600
 * @tc.name      : decode Surface time 1280*720 30fps 10M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MORE_0600, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_720_30_1;
    vDecSample->SF_OUTPUT = true;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));

    shared_ptr<VDecNdkSample> vDecSample1 = make_shared<VDecNdkSample>();
    vDecSample1->INP_DIR = INP_DIR_720_30;
    vDecSample1->SF_OUTPUT = true;
    vDecSample1->DEFAULT_WIDTH = 1280;
    vDecSample1->DEFAULT_HEIGHT = 720;
    vDecSample1->DEFAULT_FRAME_RATE = 30;
    vDecSample1->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample1->RunVideoDec_Surface(g_codecName));

    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
    vDecSample1->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample1->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MORE_0700
 * @tc.name      : decode YUV time 1280*720 60fps 10M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MORE_0700, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_720_60_1;
    vDecSample->SF_OUTPUT = false;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());

    shared_ptr<VDecNdkSample> vDecSample1 = make_shared<VDecNdkSample>();
    vDecSample1->INP_DIR = INP_DIR_720_60;
    vDecSample1->SF_OUTPUT = false;
    vDecSample1->DEFAULT_WIDTH = 1280;
    vDecSample1->DEFAULT_HEIGHT = 720;
    vDecSample1->DEFAULT_FRAME_RATE = 60;
    vDecSample1->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample1->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample1->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->StartVideoDecoder());

    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
    vDecSample1->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample1->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MORE_0800
 * @tc.name      : decode Surface time 1280*720 60fps 10M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MORE_0800, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_720_60;
    vDecSample->SF_OUTPUT = true;
    vDecSample->DEFAULT_WIDTH = 1280;
    vDecSample->DEFAULT_HEIGHT = 720;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));

    shared_ptr<VDecNdkSample> vDecSample1 = make_shared<VDecNdkSample>();
    vDecSample1->INP_DIR = INP_DIR_720_60_1;
    vDecSample1->SF_OUTPUT = true;
    vDecSample1->DEFAULT_WIDTH = 1280;
    vDecSample1->DEFAULT_HEIGHT = 720;
    vDecSample1->DEFAULT_FRAME_RATE = 60;
    vDecSample1->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample1->RunVideoDec_Surface(g_codecName));

    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
    vDecSample1->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample1->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MORE_0900
 * @tc.name      : decode YUV time 1920*1080 30fps 20M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MORE_0900, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_1080_30_1;
    vDecSample->SF_OUTPUT = false;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());

    shared_ptr<VDecNdkSample> vDecSample1 = make_shared<VDecNdkSample>();
    vDecSample1->INP_DIR = INP_DIR_1080_30;
    vDecSample1->SF_OUTPUT = false;
    vDecSample1->DEFAULT_WIDTH = 1920;
    vDecSample1->DEFAULT_HEIGHT = 1080;
    vDecSample1->DEFAULT_FRAME_RATE = 30;
    vDecSample1->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample1->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample1->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->StartVideoDecoder());

    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
    vDecSample1->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample1->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MORE_1000
 * @tc.name      : decode Surface time 1920*1080 30fps 20M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MORE_1000, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_1080_30;
    vDecSample->SF_OUTPUT = true;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));

    shared_ptr<VDecNdkSample> vDecSample1 = make_shared<VDecNdkSample>();
    vDecSample1->INP_DIR = INP_DIR_1080_30_1;
    vDecSample1->SF_OUTPUT = true;
    vDecSample1->DEFAULT_WIDTH = 1920;
    vDecSample1->DEFAULT_HEIGHT = 1080;
    vDecSample1->DEFAULT_FRAME_RATE = 30;
    vDecSample1->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample1->RunVideoDec_Surface(g_codecName));

    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
    vDecSample1->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample1->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MORE_1100
 * @tc.name      : decode YUV time 1920*1080 60fps 20M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MORE_1100, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_1080_60_1;
    vDecSample->SF_OUTPUT = false;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());

    shared_ptr<VDecNdkSample> vDecSample1 = make_shared<VDecNdkSample>();
    vDecSample1->INP_DIR = INP_DIR_1080_60;
    vDecSample1->SF_OUTPUT = false;
    vDecSample1->DEFAULT_WIDTH = 1920;
    vDecSample1->DEFAULT_HEIGHT = 1080;
    vDecSample1->DEFAULT_FRAME_RATE = 60;
    vDecSample1->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample1->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample1->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->StartVideoDecoder());

    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
    vDecSample1->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample1->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MORE_1200
 * @tc.name      : decode Surface time 1920*1080 60fps 20M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MORE_1200, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_1080_60;
    vDecSample->SF_OUTPUT = true;
    vDecSample->DEFAULT_WIDTH = 1920;
    vDecSample->DEFAULT_HEIGHT = 1080;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));

    shared_ptr<VDecNdkSample> vDecSample1 = make_shared<VDecNdkSample>();
    vDecSample1->INP_DIR = INP_DIR_1080_60_1;
    vDecSample1->SF_OUTPUT = true;
    vDecSample1->DEFAULT_WIDTH = 1920;
    vDecSample1->DEFAULT_HEIGHT = 1080;
    vDecSample1->DEFAULT_FRAME_RATE = 60;
    vDecSample1->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample1->RunVideoDec_Surface(g_codecName));

    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
    vDecSample1->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample1->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MORE_1300
 * @tc.name      : decode YUV time 3840*2160 30fps 50M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MORE_1300, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_2160_30_1;
    vDecSample->SF_OUTPUT = false;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());

    shared_ptr<VDecNdkSample> vDecSample1 = make_shared<VDecNdkSample>();
    vDecSample1->INP_DIR = INP_DIR_2160_30;
    vDecSample1->SF_OUTPUT = false;
    vDecSample1->DEFAULT_WIDTH = 3840;
    vDecSample1->DEFAULT_HEIGHT = 2160;
    vDecSample1->DEFAULT_FRAME_RATE = 30;
    vDecSample1->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample1->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample1->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->StartVideoDecoder());

    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
    vDecSample1->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample1->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MORE_1400
 * @tc.name      : decode Surface time 3840*2160 30fps 50M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MORE_1400, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_2160_30;
    vDecSample->SF_OUTPUT = true;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 30;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));

    shared_ptr<VDecNdkSample> vDecSample1 = make_shared<VDecNdkSample>();
    vDecSample1->INP_DIR = INP_DIR_2160_30_1;
    vDecSample1->SF_OUTPUT = true;
    vDecSample1->DEFAULT_WIDTH = 3840;
    vDecSample1->DEFAULT_HEIGHT = 2160;
    vDecSample1->DEFAULT_FRAME_RATE = 30;
    vDecSample1->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample1->RunVideoDec_Surface(g_codecName));

    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
    vDecSample1->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample1->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MORE_1500
 * @tc.name      : decode YUV time 3840*2160 60fps 50M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MORE_1500, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_2160_60_1;
    vDecSample->SF_OUTPUT = false;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample->StartVideoDecoder());

    shared_ptr<VDecNdkSample> vDecSample1 = make_shared<VDecNdkSample>();
    vDecSample1->INP_DIR = INP_DIR_2160_60;
    vDecSample1->SF_OUTPUT = false;
    vDecSample1->DEFAULT_WIDTH = 3840;
    vDecSample1->DEFAULT_HEIGHT = 2160;
    vDecSample1->DEFAULT_FRAME_RATE = 60;
    vDecSample1->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample1->CreateVideoDecoder(g_codecName));
    ASSERT_EQ(AV_ERR_OK, vDecSample1->ConfigureVideoDecoder());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->SetVideoDecoderCallback());
    ASSERT_EQ(AV_ERR_OK, vDecSample1->StartVideoDecoder());

    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
    vDecSample1->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample1->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_PERFORMANCE_MORE_1600
 * @tc.name      : decode Surface time 3840*2160 60fps 50M
 * @tc.desc      : perf test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_PERFORMANCE_MORE_1600, TestSize.Level3)
{
    shared_ptr<VDecNdkSample> vDecSample = make_shared<VDecNdkSample>();
    vDecSample->INP_DIR = INP_DIR_2160_60_1;
    vDecSample->SF_OUTPUT = true;
    vDecSample->DEFAULT_WIDTH = 3840;
    vDecSample->DEFAULT_HEIGHT = 2160;
    vDecSample->DEFAULT_FRAME_RATE = 60;
    vDecSample->sleepOnFPS = false;
    ASSERT_EQ(AV_ERR_OK, vDecSample->RunVideoDec_Surface(g_codecName));

    shared_ptr<VDecNdkSample> vDecSample1 = make_shared<VDecNdkSample>();
    vDecSample1->INP_DIR = INP_DIR_2160_60;
    vDecSample1->SF_OUTPUT = true;
    vDecSample1->DEFAULT_WIDTH = 3840;
    vDecSample1->DEFAULT_HEIGHT = 2160;
    vDecSample1->DEFAULT_FRAME_RATE = 60;
    vDecSample1->sleepOnFPS = true;
    ASSERT_EQ(AV_ERR_OK, vDecSample1->RunVideoDec_Surface(g_codecName));

    vDecSample->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample->errCount);
    vDecSample1->WaitForEOS();
    ASSERT_EQ(AV_ERR_OK, vDecSample1->errCount);
}

/**
 * @tc.number    : VIDEO_HWDEC_MULTIINSTANCE_0100
 * @tc.name      : create 16 decoder (320*240)
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_MULTIINSTANCE_0100, TestSize.Level3)
{
    vector<shared_ptr<VDecNdkSample>> decVec;
    for (int i = 0; i < MAX_THREAD; i++) {
        auto vDecSample = make_shared<VDecNdkSample>();
        decVec.push_back(vDecSample);
        vDecSample->INP_DIR = INP_DIR_1080_30;
        vDecSample->DEFAULT_WIDTH = 1920;
        vDecSample->DEFAULT_HEIGHT = 1080;
        vDecSample->DEFAULT_FRAME_RATE = 30;
        vDecSample->SF_OUTPUT = false;
        ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
        cout << "count=" << i << endl;
        ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
        ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
        ASSERT_EQ(AV_ERR_OK, vDecSample->Start());
    }
}
/**
 * @tc.number    : VIDEO_HWDEC_MULTIINSTANCE_0100
 * @tc.name      : create 17 decoder
 * @tc.desc      : function test
 */
HWTEST_F(HwdecPerfNdkTest, VIDEO_HWDEC_MULTIINSTANCE_0200, TestSize.Level3)
{
    vector<shared_ptr<VDecNdkSample>> decVec;
    for (int i = 0; i < MAX_THREAD + 1; i++) {
        auto vDecSample = make_shared<VDecNdkSample>();
        decVec.push_back(vDecSample);
        vDecSample->INP_DIR = INP_DIR_1080_30;
        vDecSample->DEFAULT_WIDTH = 1920;
        vDecSample->DEFAULT_HEIGHT = 1080;
        vDecSample->DEFAULT_FRAME_RATE = 30;
        vDecSample->SF_OUTPUT = false;
        if (i < MAX_THREAD) {
            ASSERT_EQ(AV_ERR_OK, vDecSample->CreateVideoDecoder(g_codecName));
            ASSERT_EQ(AV_ERR_OK, vDecSample->ConfigureVideoDecoder());
            ASSERT_EQ(AV_ERR_OK, vDecSample->SetVideoDecoderCallback());
            ASSERT_EQ(AV_ERR_OK, vDecSample->Start());
        } else {
            ASSERT_EQ(AV_ERR_UNKNOWN, vDecSample->CreateVideoDecoder(g_codecName));
        }
        cout << "count=" << i << endl;
    }
}
} // namespace