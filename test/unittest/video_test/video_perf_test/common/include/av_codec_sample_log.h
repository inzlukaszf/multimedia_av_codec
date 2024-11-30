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

#ifndef AVCODEC_SAMPLE_LOG_H
#define AVCODEC_SAMPLE_LOG_H

#include "avcodec_log.h"

namespace OHOS {
namespace MediaAVCodec {
namespace Sample {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B66

extern const bool VERBOSE_LOG;

#define AVCODEC_LOGV(fmt, ...)              \
    if (VERBOSE_LOG) {                      \
        AVCODEC_LOGI(fmt, ##__VA_ARGS__);   \
    }
} // Sample
} // MediaAVCodec
} // OHOS
#endif // AVCODEC_SAMPLE_LOG_H