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

#include "demuxer_inner_mock.h"
#include "avsource_inner_mock.h"

namespace OHOS {
namespace MediaAVCodec {
std::shared_ptr<DemuxerMock> AVDemuxerMockFactory::CreateDemuxer(std::shared_ptr<AVSourceMock> source)
{
    auto innerSource = std::static_pointer_cast<AVSourceInnerMock>(source);
    std::shared_ptr<AVSource> avSource = (innerSource != nullptr) ? innerSource->GetAVSource() : nullptr;
    std::shared_ptr<AVDemuxer> demuxer = AVDemuxerFactory::CreateWithSource(avSource);
    if (demuxer != nullptr) {
        return std::make_shared<DemuxerInnerMock>(demuxer);
    }
    return nullptr;
}
}
}