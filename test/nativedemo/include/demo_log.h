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

#ifndef DEMO_LOG_H
#define DEMO_LOG_H

#include <cstdio>

namespace OHOS {
#define LOG_MAX_SIZE 200
#define DEMO_CHECK_AND_RETURN_RET_LOG(cond, ret, fmt, ...)                                                             \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            char ch[LOG_MAX_SIZE];                                                                                     \
            (void)sprintf_s(ch, LOG_MAX_SIZE, fmt, ##__VA_ARGS__);                                                     \
            (void)printf("%s\n", ch);                                                                                  \
            return ret;                                                                                                \
        }                                                                                                              \
    } while (0)

#define DEMO_CHECK_AND_RETURN_LOG(cond, fmt, ...)                                                                      \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            char ch[LOG_MAX_SIZE];                                                                                     \
            (void)sprintf_s(ch, LOG_MAX_SIZE, fmt, ##__VA_ARGS__);                                                     \
            (void)printf("%s\n", ch);                                                                                  \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0)

#define DEMO_CHECK_AND_BREAK_LOG(cond, fmt, ...)                                                                       \
    if (!(cond)) {                                                                                                     \
        char ch[LOG_MAX_SIZE];                                                                                         \
        (void)sprintf_s(ch, LOG_MAX_SIZE, fmt, ##__VA_ARGS__);                                                         \
        (void)printf("%s\n", ch);                                                                                      \
        break;                                                                                                         \
    }

#define DEMO_CHECK_AND_CONTINUE_LOG(cond, fmt, ...)                                                                    \
    if (!(cond)) {                                                                                                     \
        char ch[LOG_MAX_SIZE];                                                                                         \
        (void)sprintf_s(ch, LOG_MAX_SIZE, fmt, ##__VA_ARGS__);                                                         \
        (void)printf("%s\n", ch);                                                                                      \
        continue;                                                                                                      \
    }
} // namespace OHOS
#endif // DEMO_LOG_H