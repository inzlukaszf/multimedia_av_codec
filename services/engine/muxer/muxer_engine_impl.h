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

#ifndef MUXER_ENGINE_IMPL_H
#define MUXER_ENGINE_IMPL_H

#include <map>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "i_muxer_engine.h"
#include "muxer.h"
#include "block_queue.h"
#include "muxer_buffer_pool.h"

namespace OHOS {
namespace MediaAVCodec {
class MuxerEngineImpl : public IMuxerEngine {
public:
    MuxerEngineImpl(int32_t appUid, int32_t appPid, int32_t fd, OutputFormat format);
    ~MuxerEngineImpl() override;
    int32_t Init();
    int32_t SetRotation(int32_t rotation) override;
    int32_t AddTrack(int32_t &trackIndex, const MediaDescription &trackDesc) override;
    int32_t Start() override;
    int32_t WriteSample(uint32_t trackIndex, std::shared_ptr<AVSharedMemory> sample,
        AVCodecBufferInfo info, AVCodecBufferFlag flag) override;
    int32_t Stop() override;
    int32_t DumpInfo(int32_t fd) override;

    enum class State {
        UNINITIALIZED,
        INITIALIZED,
        STARTED,
        STOPPED
    };

    enum TrackMimeType {
        TRACK_MIME_TYPE_AUDIO = 0,
        TRACK_MIME_TYPE_VIDEO,
        TRACK_MIME_TYPE_IMAGE
    };

private:
    int32_t StartThread(const std::string &name);
    int32_t StopThread() noexcept;
    void ThreadProcessor();
    bool CanAddTrack(const std::string &mimeType);
    bool CheckKeys(const std::string &mimeType, const MediaDescription &trackDesc);
    std::string ConvertStateToString(State state);
    int32_t TranslatePluginStatus(Plugin::Status error);
    TrackMimeType GetTrackMimeType(const std::string &mime);

    struct BlockBuffer {
        uint32_t trackIndex_;
        std::shared_ptr<uint8_t> buffer_;
        AVCodecBufferInfo info_;
        AVCodecBufferFlag flag_;
    };

    int32_t appUid_ = -1;
    int32_t appPid_ = -1;
    int64_t fd_ = -1;
    OutputFormat format_;
    std::atomic<State> state_ = State::UNINITIALIZED;
    std::shared_ptr<Plugin::Muxer> muxer_ = nullptr;
    std::map<int32_t, MediaDescription> tracksDesc_;
    BlockQueue<std::shared_ptr<BlockBuffer>> que_;
    MuxerBufferPool pool_;
    std::string threadName_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::unique_ptr<std::thread> thread_ = nullptr;
    bool isThreadExit_ = true;
};
} // namespace MediaAVCodec
} // namespace OHOS
#endif // MUXER_ENGINE_IMPL_H