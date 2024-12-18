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

#ifndef AVCODEC_AUDIO_ENCODER_INNER_DEMO_H
#define AVCODEC_AUDIO_ENCODER_INNER_DEMO_H

#include <atomic>
#include <fstream>
#include <thread>
#include <queue>
#include <string>
#include "avcodec_common.h"
#include "avcodec_audio_encoder.h"
#include "nocopyable.h"

namespace OHOS {
namespace MediaAVCodec {
namespace InnerAudioDemo {
class AEnSignal {
public:
    std::mutex inMutex_;
    std::mutex outMutex_;
    std::condition_variable inCond_;
    std::condition_variable outCond_;
    std::queue<uint32_t> inQueue_;
    std::queue<std::shared_ptr<AVSharedMemory>> inBufferQueue_;
    std::queue<std::shared_ptr<AVSharedMemory>> outBufferQueue_;
    std::queue<uint32_t> outQueue_;
    std::queue<AVCodecBufferInfo> sizeQueue_;
};

class AEnDemoCallback : public AVCodecCallback, public NoCopyable {
public:
    explicit AEnDemoCallback(std::shared_ptr<AEnSignal> signal);
    virtual ~AEnDemoCallback() = default;

    void OnError(AVCodecErrorType errorType, int32_t errorCode) override;
    void OnOutputFormatChanged(const Format &format) override;
    void OnInputBufferAvailable(uint32_t index, std::shared_ptr<AVSharedMemory> buffer) override;
    void OnOutputBufferAvailable(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag,
                                 std::shared_ptr<AVSharedMemory> buffer) override;

private:
    std::shared_ptr<AEnSignal> signal_;
};

class AEnInnerDemo : public NoCopyable {
public:
    AEnInnerDemo() = default;
    virtual ~AEnInnerDemo() = default;
    void RunCase();

private:
    int32_t CreateDec();
    int32_t Configure(const Format &format);
    int32_t Start();
    int32_t Stop();
    int32_t Flush();
    int32_t Reset();
    int32_t Release();
    void InputFunc();
    void OutputFunc();

    std::atomic<bool> isRunning_ = false;
    std::unique_ptr<std::ifstream> testFile_;
    std::unique_ptr<std::thread> inputLoop_;
    std::unique_ptr<std::thread> outputLoop_;
    std::shared_ptr<AVCodecAudioEncoder> audioEn_;
    std::shared_ptr<AEnSignal> signal_;
    std::shared_ptr<AEnDemoCallback> cb_;
};
} // namespace InnerAudioDemo
} // namespace MediaAVCodec
} // namespace OHOS
#endif // AVCODEC_AUDIO_DECODER_INNER_DEMO_H
