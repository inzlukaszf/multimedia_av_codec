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

#ifndef FILTERS_MUXER_FILTER_H
#define FILTERS_MUXER_FILTER_H

#include <cstring>
#include <map>

#include "filter/filter.h"
#include "common/status.h"
#include "meta/media_types.h"

namespace OHOS {
namespace Media {
class MediaMuxer;
namespace Pipeline {
using Plugins::OutputFormat;
class MuxerFilter : public Filter, public std::enable_shared_from_this<MuxerFilter> {
public:
    explicit MuxerFilter(std::string name, FilterType type);
    ~MuxerFilter() override;
    Status SetOutputParameter(int32_t appUid, int32_t appPid, int32_t fd, int32_t format);
    void Init(const std::shared_ptr<EventReceiver> &receiver, const std::shared_ptr<FilterCallback> &callback) override;
    void SetLogTag(std::string logTag);
    Status Prepare() override;
    Status Start() override;
    Status Pause() override;
    Status Resume() override;
    Status Stop() override;
    Status Flush() override;
    Status Release() override;
    void SetParameter(const std::shared_ptr<Meta> &parameter) override;
    void GetParameter(std::shared_ptr<Meta> &parameter) override;
    Status LinkNext(const std::shared_ptr<Filter> &nextFilter, StreamType outType) override;
    Status UpdateNext(const std::shared_ptr<Filter> &nextFilter, StreamType outType) override;
    Status UnLinkNext(const std::shared_ptr<Filter> &nextFilter, StreamType outType) override;
    FilterType GetFilterType();
    Status OnLinked(StreamType inType, const std::shared_ptr<Meta> &meta,
        const std::shared_ptr<FilterLinkCallback> &callback) override;
    Status OnUpdated(StreamType inType, const std::shared_ptr<Meta> &meta,
        const std::shared_ptr<FilterLinkCallback> &callback) override;
    Status OnUnLinked(StreamType inType, const std::shared_ptr<FilterLinkCallback>& callback) override;
    void OnBufferFilled(std::shared_ptr<AVBuffer> &inputBuffer, int32_t trackIndex,
        sptr<AVBufferQueueProducer> inputBufferQueue);

private:
    std::string name_;

    std::shared_ptr<EventReceiver> eventReceiver_;
    std::shared_ptr<FilterCallback> filterCallback_;

    std::shared_ptr<MediaMuxer> mediaMuxer_;

    int32_t preFilterCount_{0};
    int32_t startCount_{0};
    int32_t stopCount_{0};
    std::map<int32_t, int64_t> bufferPtsMap_;

    std::string logTag_ = "";
};
} // namespace Pipeline
} // namespace MEDIA
} // namespace OHOS
#endif // FILTERS_MUXER_FILTER_H
