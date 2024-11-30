/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#ifndef FILTERS_AUDIO_CAPTURE_FILTER_H
#define FILTERS_AUDIO_CAPTURE_FILTER_H

#include "filter/filter.h"
#include "common/status.h"
#include "osal/task/task.h"
#include "audio_capturer.h"

namespace OHOS {
namespace Media {
namespace AudioCaptureModule {
class AudioCaptureModule;
}
namespace Pipeline {

class AudioCaptureFilter : public Filter, public std::enable_shared_from_this<AudioCaptureFilter> {
public:
    explicit AudioCaptureFilter(std::string name, FilterType type);
    ~AudioCaptureFilter() override;
    void Init(const std::shared_ptr<EventReceiver> &receiver,
        const std::shared_ptr<FilterCallback> &callback) override;
    void SetLogTag(std::string logTag);
    Status Prepare() override;
    Status Start() override;
    Status Pause() override;
    Status Resume() override;
    Status Stop() override;
    Status Flush() override;
    Status Release() override;
    void SetParameter(const std::shared_ptr<Meta> &meta) override;
    void GetParameter(std::shared_ptr<Meta> &meta) override;
    Status LinkNext(const std::shared_ptr<Filter> &nextFilter, StreamType outType) override;
    Status UpdateNext(const std::shared_ptr<Filter> &nextFilter, StreamType outType) override;
    Status UnLinkNext(const std::shared_ptr<Filter> &nextFilter, StreamType outType) override;
    Status SendEos();
    FilterType GetFilterType();
    void SetAudioSource(int32_t source);
    void OnLinkedResult(const sptr<AVBufferQueueProducer> &queue, std::shared_ptr<Meta> &meta);
    Status OnLinked(StreamType inType, const std::shared_ptr<Meta> &meta,
        const std::shared_ptr<FilterLinkCallback> &callback) override;
    Status OnUpdated(StreamType inType, const std::shared_ptr<Meta> &meta,
        const std::shared_ptr<FilterLinkCallback> &callback) override;
    Status OnUnLinked(StreamType inType, const std::shared_ptr<FilterLinkCallback> &callback) override;
    void OnUnlinkedResult(const std::shared_ptr<Meta> &meta);
    void OnUpdatedResult(const std::shared_ptr<Meta> &meta);
    Status SetAudioCaptureChangeCallback(
        const std::shared_ptr<AudioStandard::AudioCapturerInfoChangeCallback> &callback);
    Status GetCurrentCapturerChangeInfo(AudioStandard::AudioCapturerChangeInfo &changeInfo);
    int32_t GetMaxAmplitude();
private:
    void ReadLoop();
    Status PrepareAudioCapture();
    std::shared_ptr<Task> taskPtr_{nullptr};
    std::shared_ptr<AudioCaptureModule::AudioCaptureModule> audioCaptureModule_{nullptr};
    sptr<AVBufferQueueProducer> outputBufferQueue_;
    AudioStandard::SourceType sourceType_;
    std::shared_ptr<Meta> audioCaptureConfig_;

    std::shared_ptr<EventReceiver> receiver_;
    std::shared_ptr<FilterCallback> callback_;

    std::shared_ptr<Filter> nextFilter_;
    std::atomic<FilterState> state_{FilterState::CREATED};
    std::atomic<bool> eos_{false};

    std::string logTag_ = "";
};
} // namespace Pipeline
} // namespace Media
} // namespace OHOS
#endif // FILTERS_AUDIO_CAPTURE_FILTER_H

