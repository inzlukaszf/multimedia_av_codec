/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_AUDIO_MP3_ENCODER_PLUGIN_H
#define HISTREAMER_AUDIO_MP3_ENCODER_PLUGIN_H

#include <functional>
#include <mutex>
#include <vector>
#include "buffer/avbuffer.h"
#include "meta/meta.h"
#include "nocopyable.h"
#include "plugin/codec_plugin.h"
#include "plugin/plugin_definition.h"

namespace OHOS {
namespace Media {
namespace Plugins {
namespace Mp3 {
class AudioMp3EncoderPlugin : public CodecPlugin {
public:
    explicit AudioMp3EncoderPlugin(const std::string& name);

    ~AudioMp3EncoderPlugin();

     Status Init() override;

    Status Prepare() override;

    Status Reset() override;

    Status Start() override;

    Status Stop() override;

    Status SetParameter(const std::shared_ptr<Meta> &parameter) override;

    Status GetParameter(std::shared_ptr<Meta> &parameter) override;

    Status QueueInputBuffer(const std::shared_ptr<AVBuffer> &inputBuffer) override;

    Status QueueOutputBuffer(std::shared_ptr<AVBuffer> &outputBuffer) override;

    Status GetInputBuffers(std::vector<std::shared_ptr<AVBuffer>> &inputBuffers) override;

    Status GetOutputBuffers(std::vector<std::shared_ptr<AVBuffer>> &outputBuffers) override;

    Status Flush() override;

    Status Release() override;

    Status SetDataCallback(DataCallback* dataCallback) override
    {
        dataCallback_ = dataCallback;
        return Status::OK;
    }

private:
    bool CheckFormat();
    Meta audioParameter_ ;
    mutable std::mutex avMutex_ {};

    DataCallback* dataCallback_ {nullptr};
    AudioSampleFormat audioSampleFormat_;

    struct LameInfo;
    std::unique_ptr<LameInfo> lameInfo;

    int32_t lameInitFlag;
    int32_t channels_;
    int64_t bitrate_;
    int64_t pts_;

    int32_t sampleRate_;
    int32_t maxInputSize_;
    int32_t maxOutputSize_;
    int32_t outputSize_;
    std::unique_ptr<unsigned char[]> lameMp3Buffer;
};
} // namespace Mp3
} // namespace Plugins
} // namespace Media
} // namespace OHOS

#endif // HISTREAMER_AUDIO_MP3_ENCODER_PLUGIN_H