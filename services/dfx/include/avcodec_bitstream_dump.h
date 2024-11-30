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

#ifndef AVCODEC_BITSTREAM_DUMP_H
#define AVCODEC_BITSTREAM_DUMP_H

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace OHOS {
namespace MediaAVCodec {
enum class BitStreamDumpType {
    BIT_STREAM_DUMP_TYPE_DEFAULT,
    BIT_STREAM_DUMP_TYPE_VCODEC,
    BIT_STREAM_DUMP_TYPE_ACODEC,
    BIT_STREAM_DUMP_TYPE_MUXER,
    BIT_STREAM_DUMP_TYPE_DEMUXER,
    BIT_STREAM_DUMP_TYPE_SOURCE
};

class __attribute__((visibility("default"))) AVCodecBitStreamDumper {
public:
    static AVCodecBitStreamDumper &GetInstance();
    void SaveBitStream(BitStreamDumpType type, const std::string &name,
        const uint32_t index, const uint8_t* buffer, const uint32_t size);
    bool SwitchEnable();

private:
    AVCodecBitStreamDumper();
    ~AVCodecBitStreamDumper();
    int32_t fileCount_ = 0;
    int32_t bitStreamCount_ = 0;
    std::unique_ptr<std::thread> thread_;
    void TaskProcessor();
    std::mutex mutex_;
    std::condition_variable cond_;
    std::string bitstreamString_;
    bool isDump_ = false;
    bool isExit_ = false;
    bool isEnable_ = false;
    bool isNewFile_ = true;
};

// AVCodec bitstream dump macro interface
#ifdef  BITSTREAM_DUMP_ENABLE
#define AVCODEC_BITSTREAM_DUMP(type, name, index, buffer, size)                     \
    do {                                                                            \
        (void)OHOS::MediaAVCodec::AVCodecBitStreamDumper::GetInstance().SaveBitStream(     \
            type, name, index, buffer, size);                                       \
    } while (0)
#else
#define AVCODEC_BITSTREAM_DUMP(type, name, index, buffer, size)
#endif
} // namespace MediaAVCodec
} // namespace OHOS

#endif // AVCODEC_LOG_DUMP_H