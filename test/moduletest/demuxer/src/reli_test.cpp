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

#include "gtest/gtest.h"

#include "native_avcodec_base.h"
#include "native_avdemuxer.h"
#include "native_avformat.h"
#include "native_avsource.h"

#include <iostream>
#include <cstdio>
#include <string>
#include <fcntl.h>
#include <thread>
#include <securec.h>

namespace OHOS {
namespace Media {
class DemuxerReliNdkTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
};

static OH_AVMemory *memory = nullptr;
static OH_AVSource *source = nullptr;
static OH_AVDemuxer *demuxer = nullptr;
static OH_AVFormat *sourceFormat = nullptr;
static OH_AVFormat *trackFormat = nullptr;
const char *URI2 = "http://192.168.3.11:8080/share/audio/AAC_48000_1.aac";
const char *URI1 = "http://192.168.3.11:8080/share/audio/MP3_48000_1.mp3";
static int32_t g_trackCount;
static int32_t g_width = 3840;
static int32_t g_height = 2160;
static int32_t g_maxThread = 16;
OH_AVSource *source_list[16] = {};
OH_AVMemory *memory_list[16] = {};
OH_AVDemuxer *demuxer_list[16] = {};
int g_fdList[16] = {};
int32_t g_track = 2;

void DemuxerReliNdkTest::SetUpTestCase() {}
void DemuxerReliNdkTest::TearDownTestCase() {}
void DemuxerReliNdkTest::SetUp()
{
    memory = OH_AVMemory_Create(g_width * g_height);
    g_trackCount = 0;
}
void DemuxerReliNdkTest::TearDown()
{
    if (trackFormat != nullptr) {
        OH_AVFormat_Destroy(trackFormat);
        trackFormat = nullptr;
    }

    if (sourceFormat != nullptr) {
        OH_AVFormat_Destroy(sourceFormat);
        sourceFormat = nullptr;
    }

    if (memory != nullptr) {
        OH_AVMemory_Destroy(memory);
        memory = nullptr;
    }
    if (source != nullptr) {
        OH_AVSource_Destroy(source);
        source = nullptr;
    }
    if (demuxer != nullptr) {
        OH_AVDemuxer_Destroy(demuxer);
        demuxer = nullptr;
    }

    for (int i = 0; i < g_maxThread; i++) {
        if (demuxer_list[i] != nullptr) {
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_Destroy(demuxer_list[i]));
            demuxer_list[i] = nullptr;
        }

        if (source_list[i] != nullptr) {
            ASSERT_EQ(AV_ERR_OK, OH_AVSource_Destroy(source_list[i]));
            source_list[i] = nullptr;
        }
        if (memory_list[i] != nullptr) {
            ASSERT_EQ(AV_ERR_OK, OH_AVMemory_Destroy(memory_list[i]));
            memory_list[i] = nullptr;
        }
        std::cout << i << "            finish Destroy!!!!" << std::endl;

        close(g_fdList[i]);
    }
}
} // namespace Media
} // namespace OHOS

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;
using namespace testing::ext;
namespace {
static int64_t GetFileSize(const char *fileName)
{
    int64_t fileSize = 0;
    if (fileName != nullptr) {
        struct stat fileStatus {};
        if (stat(fileName, &fileStatus) == 0) {
            fileSize = static_cast<int64_t>(fileStatus.st_size);
        }
    }
    return fileSize;
}

void DemuxFunc(int i, int loop)
{
    bool audioIsEnd = false;
    bool videoIsEnd = false;
    OH_AVCodecBufferAttr attr;
    ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer_list[i], 0));
    ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer_list[i], 1));
    while (!audioIsEnd || !videoIsEnd) {
        for (int32_t index = 0; index < g_track; index++) {
            if ((audioIsEnd && (index == 0)) || (videoIsEnd && (index == 1))) {
                continue;
            }
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer_list[i], index, memory_list[i], &attr));

            if ((index == 0) && (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS)) {
                audioIsEnd = true;
            }
            if ((index == 1) && (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS)) {
                videoIsEnd = true;
            }
        }
    }
}

static void SetAudioValue(OH_AVCodecBufferAttr attr, bool &audioIsEnd, int &audioFrame)
{
    if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
        audioIsEnd = true;
        cout << audioFrame << "    audio is end !!!!!!!!!!!!!!!" << endl;
    } else {
        audioFrame++;
    }
}

static void SetVideoValue(OH_AVCodecBufferAttr attr, bool &videoIsEnd, int &videoFrame)
{
    if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
        videoIsEnd = true;
        cout << videoFrame << "   video is end !!!!!!!!!!!!!!!" << endl;
    } else {
        videoFrame++;
        cout << "video track !!!!!" << endl;
    }
}

/**
 * @tc.number    : DEMUXER_RELI_1000
 * @tc.name      : create 16 instances create-destory
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_1000, TestSize.Level3)
{
    int len = 256;
    int num = 0;
    vector<std::thread> vecThread;
    for (int i = 0; i < g_maxThread; i++) {
        memory_list[i] = OH_AVMemory_Create(g_width * g_height);
        char file[256] = {};
        sprintf_s(file, len, "/data/test/media/16/%d_video_audio.mp4", i);
        g_fdList[i] = open(file, O_RDONLY);
        int64_t size = GetFileSize(file);
        cout << file << "----------------------" << g_fdList[i] << "---------" << size << endl;

        source_list[i] = OH_AVSource_CreateWithFD(g_fdList[i], 0, size);
        ASSERT_NE(source_list[i], nullptr);

        demuxer_list[i] = OH_AVDemuxer_CreateWithSource(source_list[i]);
        ASSERT_NE(demuxer_list[i], nullptr);
        vecThread.emplace_back(DemuxFunc, i, num);
    }
    for (auto &val : vecThread) {
        val.join();
    }
}

/**
 * @tc.number    : DEMUXER_RELI_0200
 * @tc.name      : create 16 instances repeat create-destory
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_0200, TestSize.Level3)
{
    int num = 0;
    int len = 256;
    while (num < 10) {
        num++;
        vector<std::thread> vecThread;
        for (int i = 0; i < g_maxThread; i++) {
            memory_list[i] = OH_AVMemory_Create(g_width * g_height);
            char file[256] = {};
            sprintf_s(file, len, "/data/test/media/16/%d_video_audio.mp4", i);
            g_fdList[i] = open(file, O_RDONLY);
            int64_t size = GetFileSize(file);
            cout << file << "----------------------" << g_fdList[i] << "---------" << size << endl;

            source_list[i] = OH_AVSource_CreateWithFD(g_fdList[i], 0, size);
            ASSERT_NE(source_list[i], nullptr);

            demuxer_list[i] = OH_AVDemuxer_CreateWithSource(source_list[i]);
            ASSERT_NE(demuxer_list[i], nullptr);
            vecThread.emplace_back(DemuxFunc, i, num);
        }
        for (auto &val : vecThread) {
            val.join();
        }

        for (int i = 0; i < g_maxThread; i++) {
            if (demuxer_list[i] != nullptr) {
                ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_Destroy(demuxer_list[i]));
                demuxer_list[i] = nullptr;
            }

            if (source_list[i] != nullptr) {
                ASSERT_EQ(AV_ERR_OK, OH_AVSource_Destroy(source_list[i]));
                source_list[i] = nullptr;
            }
            if (memory_list[i] != nullptr) {
                ASSERT_EQ(AV_ERR_OK, OH_AVMemory_Destroy(memory_list[i]));
                memory_list[i] = nullptr;
            }
            std::cout << i << "            finish Destroy!!!!" << std::endl;

            close(g_fdList[i]);
        }
        cout << "num: " << num << endl;
    }
}

/**
 * @tc.number    : DEMUXER_RELI_0300
 * @tc.name      : create 17 instances,17 failed
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_0300, TestSize.Level3)
{
    int num = 0;
    int len = 256;
    int64_t size = 0;
    vector<std::thread> vecThread;
    for (int i = 0; i < g_maxThread; i++) {
        memory_list[i] = OH_AVMemory_Create(g_width * g_height);
        char file[256] = {};
        sprintf_s(file, len, "/data/test/media/16/%d_video_audio.mp4", i);
        g_fdList[i] = open(file, O_RDONLY);
        size = GetFileSize(file);
        cout << file << "----------------------" << g_fdList[i] << "---------" << size << endl;

        source_list[i] = OH_AVSource_CreateWithFD(g_fdList[i], 0, size);
        ASSERT_NE(source_list[i], nullptr);

        demuxer_list[i] = OH_AVDemuxer_CreateWithSource(source_list[i]);
        ASSERT_NE(demuxer_list[i], nullptr);
        vecThread.emplace_back(DemuxFunc, i, num);
    }
    for (auto &val : vecThread) {
        val.join();
    }

    source = OH_AVSource_CreateWithFD(g_fdList[15], 0, size);
    ASSERT_EQ(source, nullptr);
    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_EQ(demuxer, nullptr);
}

/**
 * @tc.number    : DEMUXER_RELI_0400
 * @tc.name      : one instance repeat create destory
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_0400, TestSize.Level0)
{
    int num = 0;
    OH_AVCodecBufferAttr attr;

    const char *file = "/data/test/media/01_video_audio.mp4";
    bool audioIsEnd = false;
    bool videoIsEnd = false;

    int fd = open(file, O_RDONLY);
    int64_t size = GetFileSize(file);
    cout << file << "----------------------" << fd << "---------" << size << endl;
    num++;
    cout << num << endl;
    source = OH_AVSource_CreateWithFD(fd, 0, size);
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    for (int32_t index = 0; index < 2; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }
    while (!audioIsEnd || !videoIsEnd) {
        for (int32_t index = 0; index < 2; index++) {

            if ((audioIsEnd && (index == 0)) || (videoIsEnd && (index == 1))) {
                continue;
            }
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));

            if ((index == 0) && (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS)) {
                audioIsEnd = true;
                cout << "    audio is end !!!!!!!!!!!!!!!" << endl;
            }
            if ((index == 1) && (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS)) {
                videoIsEnd = true;
                cout << "   video is end !!!!!!!!!!!!!!!" << endl;
            }
        }
    }
    if (source != nullptr) {
        OH_AVSource_Destroy(source);
        source = nullptr;
    }
    if (demuxer != nullptr) {
        OH_AVDemuxer_Destroy(demuxer);
        demuxer = nullptr;
    }
    close(fd);
}

/**
 * @tc.number    : DEMUXER_RELI_0500
 * @tc.name      : one instance demux long file
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_0500, TestSize.Level3)
{
    int num = 0;
    OH_AVCodecBufferAttr attr;

    const char *file = "/data/test/media/long.mp4";

    bool audioIsEnd = false;
    bool videoIsEnd = false;
    int audioFrame = 0;
    int videoFrame = 0;

    int fd = open(file, O_RDONLY);
    int64_t size = GetFileSize(file);
    cout << file << "----------------------" << fd << "---------" << size << endl;
    num++;
    cout << num << endl;
    source = OH_AVSource_CreateWithFD(fd, 0, size);
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    for (int32_t index = 0; index < 2; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }
    while (!audioIsEnd || !videoIsEnd) {
        for (int32_t index = 0; index < 2; index++) {
            if ((audioIsEnd && (index == 0)) || (videoIsEnd && (index == 1))) {
                continue;
            }
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));

            if (index == 0) {
                SetAudioValue(attr, audioIsEnd, audioFrame);
            } else if (index == 1) {
                SetVideoValue(attr, videoIsEnd, videoFrame);
            }
        }
    }
    close(fd);
}

/**
 * @tc.number    : DEMUXER_RELI_0100
 * @tc.name      : OH_AVSource_CreateWithURI Repeat Call
 * @tc.desc      : api test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_0100, TestSize.Level2)
{
    OH_AVSource *source1 = OH_AVSource_CreateWithURI(const_cast<char *>(URI1));
    cout << URI1 << "-----------------------" << endl;
    ASSERT_NE(source1, nullptr);
    OH_AVSource *source2 = OH_AVSource_CreateWithURI(const_cast<char *>(URI2));
    cout << URI2 << "-----------------------" << endl;
    ASSERT_NE(source2, nullptr);
    ASSERT_EQ(AV_ERR_OK, OH_AVSource_Destroy(source1));
    source1 = nullptr;
    ASSERT_EQ(AV_ERR_OK, OH_AVSource_Destroy(source2));
    source2 = nullptr;
}

/**
 * @tc.number    : DEMUXER_RELI_4400
 * @tc.name      : OH_AVSource_CreateWithURI with unSupported rtsp uri
 * @tc.desc      : api test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_4400, TestSize.Level2)
{
    const char *uri = "rtp://192.168.3.11:12345";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_EQ(source, nullptr);
}

/**
 * @tc.number    : DEMUXER_RELI_4500
 * @tc.name      : OH_AVSource_CreateWithURI with unSupported rtp uri
 * @tc.desc      : api test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_4500, TestSize.Level2)
{
    const char *uri = "rtp://192.168.3.11:12345";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_EQ(source, nullptr);
}

/**
 * @tc.number    : DEMUXER_RELI_4600
 * @tc.name      : OH_AVSource_CreateWithURI with invalid uri
 * @tc.desc      : api test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_4600, TestSize.Level2)
{
    const char *uri = "https://media.w3.org/2010/05/sinte/trailer.mp4";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_EQ(source, nullptr);
}

/**
 * @tc.number    : DEMUXER_RELI_4700
 * @tc.name      : OH_AVSource_CreateWithURI with https
 * @tc.desc      : api test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_4700, TestSize.Level2)
{
    OH_AVCodecBufferAttr attr;
    bool audioIsEnd = false;
    bool videoIsEnd = false;
    int audioFrame = 0;
    int videoFrame = 0;
    int tarckType = 0;
    const char *uri = "https://media.w3.org/2010/05/sintel/trailer.mp4";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));
    ASSERT_EQ(2, g_trackCount);
    for (int32_t index = 0; index < g_trackCount; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }
    while (!audioIsEnd || !videoIsEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {

            trackFormat = OH_AVSource_GetTrackFormat(source, index);
            ASSERT_NE(trackFormat, nullptr);
            ASSERT_TRUE(OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_TRACK_TYPE, &tarckType));

            if ((audioIsEnd && (tarckType == 0)) || (videoIsEnd && (tarckType == 1))) {
                continue;
            }
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));

            if (tarckType == 0) {
                SetAudioValue(attr, audioIsEnd, audioFrame);
            } else if (tarckType == 1) {
                SetVideoValue(attr, videoIsEnd, videoFrame);
            }
        }
    }
}

/**
 * @tc.number    : DEMUXER_RELI_4800
 * @tc.name      : OH_AVSource_CreateWithURI with not supported track
 * @tc.desc      : api test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_4800, TestSize.Level2)
{
    const char *uri = "http://clips.vorwaerts-gmbh.de/big_buck_bunny.mp4";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));
    ASSERT_EQ(4, g_trackCount);
    ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, 0));
    ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, 1));
    ASSERT_NE(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, 2));
    ASSERT_NE(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, 3));
}

/**
 * @tc.number    : DEMUXER_RELI_4900
 * @tc.name      : create source with uri
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_4900, TestSize.Level0)
{
    OH_AVCodecBufferAttr attr;
    bool audioIsEnd = false;
    int audioFrame = 0;

    const char *uri = "http://192.168.3.11:8080/share/audio/MP3_48000_1.mp3";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));
    ASSERT_EQ(1, g_trackCount);

    for (int32_t index = 0; index < g_trackCount; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }
    int keyCount = 0;
    while (!audioIsEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
                audioIsEnd = true;
                cout << audioFrame << "    audio is end !!!!!!!!!!!!!!!" << endl;
                continue;
            }

            audioFrame++;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_SYNC_FRAME) {
                keyCount++;
            }
        }
    }
    ASSERT_EQ(audioFrame, 9150);
    ASSERT_EQ(keyCount, 9150);
}

/**
 * @tc.number    : DEMUXER_RELI_0500
 * @tc.name      : create source with uri,aac
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_5000, TestSize.Level0)
{
    OH_AVCodecBufferAttr attr;
    bool isEnd = false;

    const char *uri = "http://192.168.3.11:8080/share/audio/AAC_48000_1.aac";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));

    for (int32_t index = 0; index < g_trackCount; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }

    int audioFrame = 0;
    int keyCount = 0;
    while (!isEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));
            cout << attr.size << "size---------------pts:" << attr.pts << endl;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
                isEnd = true;
                cout << "isend !!!!!!!!!!!!!!!" << endl;
                continue;
            }

            audioFrame++;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_SYNC_FRAME) {
                keyCount++;
            }
        }
    }
    ASSERT_EQ(audioFrame, 9457);
    ASSERT_EQ(keyCount, 9457);
}

/**
 * @tc.number    : DEMUXER_RELI_5100
 * @tc.name      : create source with uri,flac
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_5100, TestSize.Level0)
{
    OH_AVCodecBufferAttr attr;
    bool isEnd = false;

    const char *uri = "http://192.168.3.11:8080/share/audio/FLAC_48000_1.flac";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));

    for (int32_t index = 0; index < g_trackCount; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }

    int audioFrame = 0;
    int keyCount = 0;
    while (!isEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));
            cout << attr.size << "size---------------pts:" << attr.pts << endl;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
                isEnd = true;
                cout << "isend !!!!!!!!!!!!!!!" << endl;
                continue;
            }

            audioFrame++;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_SYNC_FRAME) {
                keyCount++;
            }
        }
    }
    ASSERT_EQ(audioFrame, 2288);
    ASSERT_EQ(keyCount, 2288);
}

/**
 * @tc.number    : DEMUXER_RELI_5200
 * @tc.name      : create source with uri,m4a
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_5200, TestSize.Level0)
{
    OH_AVCodecBufferAttr attr;
    bool isEnd = false;

    const char *uri = "http://192.168.3.11:8080/share/audio/M4A_48000_1.m4a";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));

    for (int32_t index = 0; index < g_trackCount; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }

    int audioFrame = 0;
    int keyCount = 0;
    while (!isEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));
            cout << attr.size << "size---------------pts:" << attr.pts << endl;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
                isEnd = true;
                cout << "isend !!!!!!!!!!!!!!!" << endl;
                continue;
            }

            audioFrame++;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_SYNC_FRAME) {
                keyCount++;
            }
        }
    }
    ASSERT_EQ(audioFrame, 10293);
    ASSERT_EQ(keyCount, 10293);
}

/**
 * @tc.number    : DEMUXER_RELI_5300
 * @tc.name      : create source with uri,mp3
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_5300, TestSize.Level0)
{
    OH_AVCodecBufferAttr attr;
    bool isEnd = false;

    const char *uri = "http://192.168.3.11:8080/share/audio/MP3_48000_1.mp3";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));

    for (int32_t index = 0; index < g_trackCount; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }

    int audioFrame = 0;
    int keyCount = 0;
    while (!isEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));
            cout << attr.size << "size---------------pts:" << attr.pts << endl;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
                isEnd = true;
                cout << "isend !!!!!!!!!!!!!!!" << endl;
                continue;
            }

            audioFrame++;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_SYNC_FRAME) {
                keyCount++;
            }
        }
    }
    ASSERT_EQ(audioFrame, 9150);
    ASSERT_EQ(keyCount, 9150);
}

/**
 * @tc.number    : DEMUXER_RELI_5400
 * @tc.name      : create source with uri,ogg
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_5400, TestSize.Level0)
{
    OH_AVCodecBufferAttr attr;
    bool isEnd = false;

    const char *uri = "http://192.168.3.11:8080/share/audio/OGG_48000_1.ogg";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));

    for (int32_t index = 0; index < g_trackCount; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }

    int audioFrame = 0;
    int keyCount = 0;
    while (!isEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));
            cout << attr.size << "size---------------pts:" << attr.pts << endl;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
                isEnd = true;
                cout << "isend !!!!!!!!!!!!!!!" << endl;
                continue;
            }

            audioFrame++;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_SYNC_FRAME) {
                keyCount++;
            }
        }
    }
    ASSERT_EQ(audioFrame, 11439);
    ASSERT_EQ(keyCount, 11439);
}

/**
 * @tc.number    : DEMUXER_RELI_5500
 * @tc.name      : create source with uri,wav
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_5500, TestSize.Level0)
{
    OH_AVCodecBufferAttr attr;
    bool isEnd = false;

    const char *uri = "http://192.168.3.11:8080/share/audio/wav_48000_1.wav";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));

    for (int32_t index = 0; index < g_trackCount; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }

    int audioFrame = 0;
    int keyCount = 0;
    while (!isEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));
            cout << attr.size << "size---------------pts:" << attr.pts << endl;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
                isEnd = true;
                cout << "isend !!!!!!!!!!!!!!!" << endl;
                continue;
            }

            audioFrame++;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_SYNC_FRAME) {
                keyCount++;
            }
        }
    }
    ASSERT_EQ(audioFrame, 5146);
    ASSERT_EQ(keyCount, 5146);
}

/**
 * @tc.number    : DEMUXER_RELI_5600
 * @tc.name      : create source with uri,mp4
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_5600, TestSize.Level0)
{
    OH_AVCodecBufferAttr attr;
    bool audioIsEnd = false;
    bool videoIsEnd = false;
    int audioFrame = 0;
    int videoFrame = 0;
    int tarckType = 0;
    const char *uri = "http://192.168.3.11:8080/share/01_video_audio.mp4";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));
    ASSERT_EQ(2, g_trackCount);
    for (int32_t index = 0; index < g_trackCount; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }
    while (!audioIsEnd || !videoIsEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {

            trackFormat = OH_AVSource_GetTrackFormat(source, index);
            ASSERT_NE(trackFormat, nullptr);
            ASSERT_TRUE(OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_TRACK_TYPE, &tarckType));

            if ((audioIsEnd && (tarckType == 0)) || (videoIsEnd && (tarckType == 1))) {
                continue;
            }
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));

            if (tarckType == 0) {
                SetAudioValue(attr, audioIsEnd, audioFrame);
            } else if (tarckType == 1) {
                SetVideoValue(attr, videoIsEnd, videoFrame);
            }
        }
    }
}

void CreateDemuxer(char *uri)
{
    int two = 2;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));
    ASSERT_EQ(two, g_trackCount);

    for (int32_t index = 0; index < g_trackCount; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }
}

/**
 * @tc.number    : DEMUXER_RELI_5700
 * @tc.name      : create source with uri,hvcc mp4
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_5700, TestSize.Level0)
{
    int tarckType = 0;
    OH_AVCodecBufferAttr attr;
    bool audioIsEnd = false;
    bool videoIsEnd = false;
    int audioFrame = 0;
    int videoFrame = 0;
    const char *uri = "http://192.168.3.11:8080/share/hvcc_1920x1080_60.mp4";
    cout << uri << "------" << endl;
    CreateDemuxer(const_cast<char *>(uri));
    int aKeyCount = 0;
    int vKeyCount = 0;
    while (!audioIsEnd || !videoIsEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {

            trackFormat = OH_AVSource_GetTrackFormat(source, index);
            ASSERT_NE(trackFormat, nullptr);
            ASSERT_TRUE(OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_TRACK_TYPE, &tarckType));

            if ((audioIsEnd && (tarckType == 0)) || (videoIsEnd && (tarckType == 1))) {
                continue;
            }
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));

            if (tarckType == 0) {
                SetAudioValue(attr, audioIsEnd, audioFrame);
            } else if (tarckType == 1) {
                SetVideoValue(attr, videoIsEnd, videoFrame);
            }

            if (tarckType == 0 && attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_SYNC_FRAME) {
                aKeyCount++;
            } else if (tarckType == 1 && attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_SYNC_FRAME) {
                vKeyCount++;
            }
        }
    }
    ASSERT_EQ(audioFrame, 433);
    ASSERT_EQ(videoFrame, 602);
    ASSERT_EQ(aKeyCount, 433);
    ASSERT_EQ(vKeyCount, 3);
}

/**
 * @tc.number    : DEMUXER_RELI_5800
 * @tc.name      : create source with uri,wav
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_5800, TestSize.Level0)
{
    int tarckType = 0;
    OH_AVCodecBufferAttr attr;
    bool audioIsEnd = false;
    bool videoIsEnd = false;
    int audioFrame = 0;
    int videoFrame = 0;

    const char *uri = "http://192.168.3.11:8080/share/avcc_10sec.mp4";
    cout << uri << "------" << endl;
    CreateDemuxer(const_cast<char *>(uri));
    while (!audioIsEnd || !videoIsEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {

            trackFormat = OH_AVSource_GetTrackFormat(source, index);
            ASSERT_NE(trackFormat, nullptr);
            ASSERT_TRUE(OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_TRACK_TYPE, &tarckType));

            if ((audioIsEnd && (tarckType == 0)) || (videoIsEnd && (tarckType == 1))) {
                continue;
            }
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));

            if (tarckType == 0) {
                SetAudioValue(attr, audioIsEnd, audioFrame);
            } else if (tarckType == 1) {
                SetVideoValue(attr, videoIsEnd, videoFrame);
            }
        }
    }
    ASSERT_EQ(audioFrame, 431);
    ASSERT_EQ(videoFrame, 600);
}

/**
 * @tc.number    : DEMUXER_RELI_5900
 * @tc.name      : create source with uri,wav
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_5900, TestSize.Level0)
{
    int tarckType = 0;
    OH_AVCodecBufferAttr attr;
    bool audioIsEnd = false;
    bool videoIsEnd = false;
    int audioFrame = 0;
    int videoFrame = 0;
    const char *uri = "http://192.168.3.11:8080/share/ts_video.ts";
    cout << uri << "------" << endl;
    CreateDemuxer(const_cast<char *>(uri));
    while (!audioIsEnd || !videoIsEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {

            trackFormat = OH_AVSource_GetTrackFormat(source, index);
            ASSERT_NE(trackFormat, nullptr);
            ASSERT_TRUE(OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_TRACK_TYPE, &tarckType));

            if ((audioIsEnd && (tarckType == 0)) || (videoIsEnd && (tarckType == 1))) {
                continue;
            }
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));

            if (tarckType == 0) {
                SetAudioValue(attr, audioIsEnd, audioFrame);
            } else if (tarckType == 1) {
                SetVideoValue(attr, videoIsEnd, videoFrame);
            }
        }
    }
    ASSERT_EQ(audioFrame, 384);
    ASSERT_EQ(videoFrame, 602);
}

/**
 * @tc.number    : DEMUXER_RELI_6000
 * @tc.name      : create source with uri,wav
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_6000, TestSize.Level0)
{
    const char *uri = "http://192.168.3.11:8080/share/zero_track.mp4";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);

    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));
    ASSERT_EQ(g_trackCount, 0);

    ASSERT_EQ(AV_ERR_INVALID_VAL, OH_AVDemuxer_SelectTrackByID(demuxer, 0));
}

/**
 * @tc.number    : DEMUXER_RELI_6100
 * @tc.name      : create source with uri
 * @tc.desc      : reliable test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_6100, TestSize.Level0)
{
    int num = 0;
    OH_AVCodecBufferAttr attr;
    const char *uri = "http://192.168.3.11:8080/share/avcc_10sec.mp4";
    cout << uri << "------" << endl;
    cout<<"num: "<<num<<endl;
    int tarckType = 0;
    bool audioIsEnd = false;
    bool videoIsEnd = false;
    int audioFrame = 0;
    int videoFrame = 0;
    CreateDemuxer(const_cast<char *>(uri));
    while (!audioIsEnd || !videoIsEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {
            trackFormat = OH_AVSource_GetTrackFormat(source, index);
            ASSERT_NE(trackFormat, nullptr);
            ASSERT_TRUE(OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_TRACK_TYPE, &tarckType));

            if ((audioIsEnd && (tarckType == 0)) || (videoIsEnd && (tarckType == 1))) {
                continue;
            }
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));
            
            if (tarckType == 0) {
                SetAudioValue(attr, audioIsEnd, audioFrame);
            } else if (tarckType == 1) {
                SetVideoValue(attr, videoIsEnd, videoFrame);
            }
        }
    }
    ASSERT_EQ(audioFrame, 431);
    ASSERT_EQ(videoFrame, 600);
    num++;
}

/**
 * @tc.number    : DEMUXER_RELI_6200
 * @tc.name      : create source with uri
 * @tc.desc      : reliable test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_6200, TestSize.Level3)
{
    int num = 0;
    OH_AVCodecBufferAttr attr;
    const char *uri = "https://media.w3.org/2010/05/sintel/trailer.mp4";
    cout << uri << "------" << endl;
    cout<<"num: "<<num<<endl;
    int tarckType = 0;
    bool audioIsEnd = false;
    bool videoIsEnd = false;
    int audioFrame = 0;
    int videoFrame = 0;
    CreateDemuxer(const_cast<char *>(uri));
    while (!audioIsEnd || !videoIsEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {

            trackFormat = OH_AVSource_GetTrackFormat(source, index);
            ASSERT_NE(trackFormat, nullptr);
            ASSERT_TRUE(OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_TRACK_TYPE, &tarckType));

            if ((audioIsEnd && (tarckType == 0)) || (videoIsEnd && (tarckType == 1))) {
                continue;
            }
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));

            if (tarckType == 0) {
                SetAudioValue(attr, audioIsEnd, audioFrame);
            } else if (tarckType == 1) {
                SetVideoValue(attr, videoIsEnd, videoFrame);
            }
        }
    }
    num++;
}

/**
 * @tc.number    : DEMUXER_RELI_6300
 * @tc.name      : create 16 instances create-destory
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_6300, TestSize.Level3)
{
    int num = 0;
    while (num < 5) {
        num++;
        vector<std::thread> vecThread;
        // "https://media.w3.org/2010/05/sintel/trailer.mp4"
        const char *uri = "http://192.168.3.11:8080/share/avcc_10sec.mp4";
        for (int i = 0; i < g_maxThread; i++) {
            memory_list[i] = OH_AVMemory_Create(g_width * g_height);
            cout << i << "  uri:  " << uri << endl;
            source_list[i] = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
            ASSERT_NE(source_list[i], nullptr);
            demuxer_list[i] = OH_AVDemuxer_CreateWithSource(source_list[i]);
            ASSERT_NE(demuxer_list[i], nullptr);
            vecThread.emplace_back(DemuxFunc, i, num);
        }
        for (auto &val : vecThread) {
            val.join();
        }

        for (int i = 0; i < g_maxThread; i++) {
            if (demuxer_list[i] != nullptr) {
                ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_Destroy(demuxer_list[i]));
                demuxer_list[i] = nullptr;
            }

            if (source_list[i] != nullptr) {
                ASSERT_EQ(AV_ERR_OK, OH_AVSource_Destroy(source_list[i]));
                source_list[i] = nullptr;
            }
            if (memory_list[i] != nullptr) {
                ASSERT_EQ(AV_ERR_OK, OH_AVMemory_Destroy(memory_list[i]));
                memory_list[i] = nullptr;
            }
            std::cout << i << "            finish Destroy!!!!" << std::endl;
        }
        cout << "num: " << num << endl;
    }
}

/**
 * @tc.number    : DEMUXER_RELI_6400
 * @tc.name      : create source with uri,amr_nb
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_6400, TestSize.Level0)
{
    OH_AVCodecBufferAttr attr;
    bool isEnd = false;
    const char *uri = "http://192.168.3.11:8080/share/amr_nb_8000_1.amr";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);
    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);
    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));
    for (int32_t index = 0; index < g_trackCount; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }
    int audioFrame = 0;
    int keyCount = 0;
    while (!isEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));
            cout << attr.size << "size---------------pts:" << attr.pts << endl;
            if (attr.flags == OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
                isEnd = true;
                cout << "isend !!!!!!!!!!!!!!!" << endl;
                continue;
            }
            audioFrame++;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_SYNC_FRAME) {
                keyCount++;
            }
        }
    }
    cout << "audioFrame:" << audioFrame << "   keyCount:" << keyCount << endl;
    ASSERT_EQ(audioFrame, 1501);
    ASSERT_EQ(keyCount, 1501);
}

/**
 * @tc.number    : DEMUXER_RELI_6500
 * @tc.name      : create source with uri,flac
 * @tc.desc      : function test
 */
HWTEST_F(DemuxerReliNdkTest, DEMUXER_RELI_6500, TestSize.Level0)
{
    OH_AVCodecBufferAttr attr;
    bool isEnd = false;
    const char *uri = "http://192.168.3.11:8080/share/amr_wb_16000_1.amr";
    cout << uri << "------" << endl;
    source = OH_AVSource_CreateWithURI(const_cast<char *>(uri));
    ASSERT_NE(source, nullptr);
    demuxer = OH_AVDemuxer_CreateWithSource(source);
    ASSERT_NE(demuxer, nullptr);
    sourceFormat = OH_AVSource_GetSourceFormat(source);
    ASSERT_TRUE(OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &g_trackCount));
    for (int32_t index = 0; index < g_trackCount; index++) {
        ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_SelectTrackByID(demuxer, index));
    }
    int audioFrame = 0;
    int keyCount = 0;
    while (!isEnd) {
        for (int32_t index = 0; index < g_trackCount; index++) {
            ASSERT_EQ(AV_ERR_OK, OH_AVDemuxer_ReadSample(demuxer, index, memory, &attr));
            cout << attr.size << "size---------------pts:" << attr.pts << endl;
            if (attr.flags == OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_EOS) {
                isEnd = true;
                cout << "isend !!!!!!!!!!!!!!!" << endl;
                continue;
            }

            audioFrame++;
            if (attr.flags & OH_AVCodecBufferFlags::AVCODEC_BUFFER_FLAGS_SYNC_FRAME) {
                keyCount++;
            }
        }
    }
    cout << "audioFrame:" << audioFrame << "   keyCount:" << keyCount << endl;
    ASSERT_EQ(audioFrame, 1500);
    ASSERT_EQ(keyCount, 1500);
}
}