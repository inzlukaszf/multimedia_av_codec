/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef AVCODEC_SAMPLE_SAMPLE_BASE_H
#define AVCODEC_SAMPLE_SAMPLE_BASE_H

#include <fstream>
#include <thread>
#include "sample_info.h"

namespace OHOS {
namespace MediaAVCodec {
namespace Sample {
class SampleBase {
public:
    virtual ~SampleBase() {};

    virtual int32_t Create(SampleInfo sampleInfo) = 0;
    virtual int32_t Start() = 0;
    virtual int32_t WaitForDone();

protected:
    std::mutex mutex_;
    std::condition_variable doneCond_;
};

class SampleFactory {
public:
    static std::shared_ptr<SampleBase> CreateSample(const SampleInfo &info);
};
} // Sample
} // MediaAVCodec
} // OHOS
#endif // AVCODEC_SAMPLE_SAMPLE_BASE_H