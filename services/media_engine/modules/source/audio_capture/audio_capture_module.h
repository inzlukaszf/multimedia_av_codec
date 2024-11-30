/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_AUDIO_CAPTURE_MODULE_H
#define HISTREAMER_AUDIO_CAPTURE_MODULE_H

#include <string>
#include <memory>
#include "audio_capturer.h"
#include "common/status.h"
#include "meta/meta.h"
#include "buffer/avbuffer.h"
#include "osal/task/mutex.h"

namespace OHOS {
namespace Media {
namespace AudioCaptureModule {
using ValueType = Any;

class AudioCaptureModuleCallback {
public:
    virtual ~AudioCaptureModuleCallback() = default;

    virtual void OnInterrupt(const std::string &interruptInfo) = 0;
};

class AudioCaptureModule {
public:
    explicit AudioCaptureModule();
    ~AudioCaptureModule();
    Status Init();
    void SetLogTag(std::string logTag);
    Status Deinit();
    Status Prepare();
    Status Reset();
    Status Start();
    Status Stop();
    Status GetParameter(std::shared_ptr<Meta> &meta);
    Status SetParameter(const std::shared_ptr<Meta> &meta);
    Status Read(std::shared_ptr<AVBuffer> &buffer, size_t expectedLen);
    Status GetSize(uint64_t &size);
    Status SetAudioInterruptListener(const std::shared_ptr<AudioCaptureModuleCallback> &callback);
    Status SetAudioCapturerInfoChangeCallback(
        const std::shared_ptr<AudioStandard::AudioCapturerInfoChangeCallback> &callback);
    Status GetCurrentCapturerChangeInfo(AudioStandard::AudioCapturerChangeInfo &changeInfo);
    int32_t GetMaxAmplitude();
    void SetAudioSource(AudioStandard::SourceType source);
private:
    Status DoDeinit();
    bool AssignSampleRateIfSupported(const int32_t value);
    bool AssignChannelNumIfSupported(const int32_t value);
    bool AssignSampleFmtIfSupported(const Plugins::AudioSampleFormat value);

    void TrackMaxAmplitude(int16_t *data, int32_t size);

    Mutex captureMutex_ {};
    std::unique_ptr<OHOS::AudioStandard::AudioCapturer> audioCapturer_ {nullptr};
    std::shared_ptr<AudioStandard::AudioCapturerInfoChangeCallback> audioCapturerInfoChangeCallback_{nullptr};
    AudioStandard::AudioCapturerOptions options_{};
    std::shared_ptr<AudioCaptureModuleCallback> audioCaptureModuleCallback_ {nullptr};
    std::shared_ptr<AudioStandard::AudioCapturerCallback> audioInterruptCallback_ {nullptr};
    int64_t bitRate_ {0};
    int32_t appTokenId_ {0};
    int32_t appUid_ {0};
    int32_t appPid_ {0};
    int64_t appFullTokenId_ {0};
    size_t bufferSize_ {0};
    int32_t maxAmplitude_ {0};
    bool isTrackMaxAmplitude {false};

    std::string logTag_ = "";
};
} // namespace AudioCaptureModule
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_AUDIO_CAPTURE_MODULE_H

