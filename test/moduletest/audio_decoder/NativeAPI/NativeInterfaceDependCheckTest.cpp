/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "AudioDecoderDemoCommon.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::MediaAVCodec;

constexpr uint32_t CHANNEL_COUNT = 2;
constexpr uint32_t SAMPLE_RATE = 44100;
constexpr uint32_t CODED_SAMPLE = 4;
constexpr uint32_t KEY_BITRATE = 169000;
constexpr uint32_t SIZE_INFO = 100;

namespace {
class NativeInterfaceDependCheckTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void NativeInterfaceDependCheckTest::SetUpTestCase() {}
void NativeInterfaceDependCheckTest::TearDownTestCase() {}
void NativeInterfaceDependCheckTest::SetUp() {}
void NativeInterfaceDependCheckTest::TearDown() {}

OH_AVCodec *Create(AudioDecoderDemo *decoderDemo)
{
    return decoderDemo->NativeCreateByName("OH.Media.Codec.Decoder.Audio.Mpeg");
}

OH_AVErrCode Destroy(AudioDecoderDemo *decoderDemo, OH_AVCodec *handle)
{
    return decoderDemo->NativeDestroy(handle);
}

OH_AVErrCode SetCallback(AudioDecoderDemo *decoderDemo, OH_AVCodec *handle)
{
    struct OH_AVCodecAsyncCallback cb = {&OnError, &OnOutputFormatChanged, &OnInputBufferAvailable,
                                         &OnOutputBufferAvailable};
    return decoderDemo->NativeSetCallback(handle, cb);
}

OH_AVErrCode Configure(AudioDecoderDemo *decoderDemo, OH_AVCodec *handle)
{
    OH_AVFormat *format = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_AUD_CHANNEL_COUNT, CHANNEL_COUNT);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_AUD_SAMPLE_RATE, SAMPLE_RATE);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_BITS_PER_CODED_SAMPLE, CODED_SAMPLE);
    OH_AVFormat_SetLongValue(format, OH_MD_KEY_BITRATE, KEY_BITRATE);

    OH_AVErrCode ret = decoderDemo->NativeConfigure(handle, format);

    OH_AVFormat_Destroy(format);
    return ret;
}

OH_AVErrCode Prepare(AudioDecoderDemo *decoderDemo, OH_AVCodec *handle)
{
    return decoderDemo->NativePrepare(handle);
}

OH_AVErrCode Start(AudioDecoderDemo *decoderDemo, OH_AVCodec *handle, uint32_t &index, uint8_t *data)
{
    OH_AVErrCode ret = decoderDemo->NativeStart(handle);
    if (ret != AV_ERR_OK) {
        return ret;
    }
    sleep(1);
    index = decoderDemo->NativeGetInputIndex();
    data = decoderDemo->NativeGetInputBuf();

    return ret;
}

OH_AVErrCode Stop(AudioDecoderDemo *decoderDemo, OH_AVCodec *handle)
{
    return decoderDemo->NativeStop(handle);
}

OH_AVErrCode Flush(AudioDecoderDemo *decoderDemo, OH_AVCodec *handle)
{
    return decoderDemo->NativeFlush(handle);
}

OH_AVErrCode Reset(AudioDecoderDemo *decoderDemo, OH_AVCodec *handle)
{
    return decoderDemo->NativeReset(handle);
}

OH_AVErrCode PushInputData(AudioDecoderDemo *decoderDemo, OH_AVCodec *handle, uint32_t index)
{
    OH_AVCodecBufferAttr info;
    info.size = SIZE_INFO;
    info.offset = 0;
    info.pts = 0;
    info.flags = AVCODEC_BUFFER_FLAGS_NONE;

    return decoderDemo->NativePushInputData(handle, index, info);
}

OH_AVErrCode PushInputDataEOS(AudioDecoderDemo *decoderDemo, OH_AVCodec *handle, uint32_t index)
{
    OH_AVCodecBufferAttr info;
    info.size = 0;
    info.offset = 0;
    info.pts = 0;
    info.flags = AVCODEC_BUFFER_FLAGS_EOS;

    return decoderDemo->NativePushInputData(handle, index, info);
}
} // namespace

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_001
 * @tc.name      : Create -> SetCallback
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_001, TestSize.Level2)
{
    OH_AVErrCode ret;
    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_002
 * @tc.name      : Create -> SetCallback -> SetCallback
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_002, TestSize.Level2)
{
    OH_AVErrCode ret;
    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_003
 * @tc.name      : Create -> Configure -> SetCallback
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_003, TestSize.Level2)
{
    OH_AVErrCode ret;
    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_004
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> SetCallback
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_004, TestSize.Level2)
{
    OH_AVErrCode ret;
    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_005
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> SetCallback
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_005, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    cout << "handle is " << handle << endl;
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    cout << "SetCallback ret is " << ret << endl;
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    cout << "Configure ret is " << ret << endl;
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    cout << "Prepare ret is " << ret << endl;
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    cout << "Start ret is " << ret << endl;
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = SetCallback(decoderDemo, handle);
    cout << "SetCallback ret is " << ret << endl;
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_006
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> PushInputData -> SetCallback
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_006, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_007
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> PushInputData[EOS] -> SetCallback
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_007, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputDataEOS(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_008
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Flush -> SetCallback
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_008, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_009
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Stop -> SetCallback
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_009, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_010
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Reset -> SetCallback
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_010, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);
    cout << "handle is " << handle << endl;

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    cout << "SetCallback ret is " << ret << endl;

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    cout << "Configure ret is " << ret << endl;

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    cout << "Prepare ret is " << ret << endl;

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);
    cout << "Start ret is " << ret << endl;

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    cout << "Stop ret is " << ret << endl;

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    cout << "Reset ret is " << ret << endl;

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    cout << "SetCallback ret is " << ret << endl;

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_011
 * @tc.name      : Create -> SetCallback -> Configure
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_011, TestSize.Level2)
{
    OH_AVErrCode ret;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_012
 * @tc.name      : Create -> SetCallback -> Configure -> Configure
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_012, TestSize.Level2)
{
    OH_AVErrCode ret;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_013
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Configure
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_013, TestSize.Level2)
{
    OH_AVErrCode ret;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_014
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Configure
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_014, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_015
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> PushInputData -> Configure
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_015, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_016
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> PushInputData[EOS] -> Configure
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_016, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputDataEOS(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_017
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Flush -> Configure
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_017, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_018
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Configure
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_018, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_019
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Reset -> Configure
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_019, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_020
 * @tc.name      : Create -> Start
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_020, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_021
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_021, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_022
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Start
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_022, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_023
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> PushInputData -> Start
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_023, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_024
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> PushInputData[EOS] -> Start
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_024, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputDataEOS(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_025
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Flush -> Start
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_025, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_026
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Start
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_026, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_027
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Reset -> Start
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_027, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_028
 * @tc.name      : Create -> PushInputData
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_028, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_029
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> PushInputData
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_029, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_030
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> PushInputData
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_030, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    cout << "index is " << trackId << endl;
    printf("data is %p\n", data);
    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_031
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> PushInputData -> PushInputData
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_031, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_UNKNOWN, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_032
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> PushInputData[EOS] -> PushInputData
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_032, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputDataEOS(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_033
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Flush -> PushInputData
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_033, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_034
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Flush -> Start -> PushInputData
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_034, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_035
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Stop -> PushInputData
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_035, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_036
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Reset -> PushInputData
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_036, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_037
 * @tc.name      : Create -> Flush
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_037, TestSize.Level2)
{
    OH_AVErrCode ret;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_038
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Flush
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_038, TestSize.Level2)
{
    OH_AVErrCode ret;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_039
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Flush
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_039, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_040
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> PushInputData -> Flush
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_040, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_041
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> PushInputData[EOS] -> Flush
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_041, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputDataEOS(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_042
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Flush -> Flush
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_042, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_043
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Flush
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_043, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_044
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Reset -> Flush
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_044, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_045
 * @tc.name      : Create -> Stop
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_045, TestSize.Level2)
{
    OH_AVErrCode ret;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_046
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Stop
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_046, TestSize.Level2)
{
    OH_AVErrCode ret;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_047
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Stop
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_047, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_048
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> PushInputData -> Stop
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_048, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_049
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> PushInputData[EOS] -> Stop
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_049, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputDataEOS(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_050
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Flush -> Stop
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_050, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_051
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Stop
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_051, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_052
 * @tc.name      : Create -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Reset -> Stop
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_052, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_INVALID_STATE, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_053
 * @tc.name      : Creat -> Reset
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_053, TestSize.Level2)
{
    OH_AVErrCode ret;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_054
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Reset
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_054, TestSize.Level2)
{
    OH_AVErrCode ret;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_055
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Start -> Reset
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_055, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_056
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Start -> PushInputData -> Reset
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_056, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_057
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Start -> PushInputData[EOS] -> Reset
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_057, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputDataEOS(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_058
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Start -> Flush -> Reset
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_058, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_059
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Reset
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_059, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_060
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Reset -> Reset
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_060, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    Destroy(decoderDemo, handle);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_061
 * @tc.name      : Creat -> Destory
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_061, TestSize.Level2)
{
    OH_AVErrCode ret;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = Destroy(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_062
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Destory
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_062, TestSize.Level2)
{
    OH_AVErrCode ret;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Destroy(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_063
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Start -> Destory
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_063, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Destroy(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_064
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Start -> PushInputData -> Destory
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_064, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputData(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Destroy(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_065
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Start -> PushInputData[EOS] -> Destory
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_065, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = PushInputDataEOS(decoderDemo, handle, trackId);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Destroy(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_066
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Start -> Flush -> Destory
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_066, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Flush(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Destroy(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_067
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Destory
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_067, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Destroy(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    delete decoderDemo;
}

/**
 * @tc.number    : SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_068
 * @tc.name      : Creat -> SetCallback -> Configure -> Prepare -> Start -> Stop -> Reset -> Destory
 * @tc.desc      : interface depend check
 */
HWTEST_F(NativeInterfaceDependCheckTest, SUB_MULTIMEDIA_AUDIO_DECODER_INTERFACE_DEPEND_CHECK_068, TestSize.Level2)
{
    OH_AVErrCode ret;
    uint32_t trackId = -1;
    uint8_t *data = nullptr;

    AudioDecoderDemo *decoderDemo = new AudioDecoderDemo();
    OH_AVCodec *handle = Create(decoderDemo);
    ASSERT_NE(nullptr, handle);

    ret = SetCallback(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Configure(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Prepare(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Start(decoderDemo, handle, trackId, data);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Stop(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Reset(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);

    ret = Destroy(decoderDemo, handle);
    ASSERT_EQ(AV_ERR_OK, ret);
    delete decoderDemo;
}