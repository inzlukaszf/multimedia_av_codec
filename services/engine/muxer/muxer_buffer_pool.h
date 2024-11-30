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

#ifndef MUXER_BUFFER_POOL_H
#define MUXER_BUFFER_POOL_H

#include <list>
#include <string>
#include <memory>
#include <mutex>

namespace OHOS {
namespace MediaAVCodec {
class MuxerBufferPool {
public:
    explicit MuxerBufferPool(std::string name, size_t capacity = defaultPoolSize);
    ~MuxerBufferPool();
    std::shared_ptr<uint8_t> AcquireBuffer(int32_t size);
    void ReleaseBuffer(std::shared_ptr<uint8_t> buffer);
    void Reset();
    std::string GetName();
private:
    using MuxerBuffer = std::pair<std::shared_ptr<uint8_t>, int32_t>;
    std::list<MuxerBuffer> idleList_;
    std::list<MuxerBuffer> busyList_;
    std::string name_;
    const size_t capacity_;
    std::mutex mutex_;
    static const size_t defaultPoolSize = 16;
};
}
}
#endif