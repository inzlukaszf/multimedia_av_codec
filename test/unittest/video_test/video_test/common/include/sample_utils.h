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

#ifndef AVCODEC_SAMPLE_SAMPLE_UTILS_H
#define AVCODEC_SAMPLE_SAMPLE_UTILS_H

#include <cstdint>

namespace OHOS {
namespace MediaAVCodec {
namespace Sample {
void ThreadSleep(bool isValid, int32_t interval);
int32_t ToGraphicPixelFormat(int32_t avPixelFormat, int32_t profile);
} // Sample
} // MediaAVCodec
} // OHOS

#endif // AVCODEC_SAMPLE_SAMPLE_UTILS_H