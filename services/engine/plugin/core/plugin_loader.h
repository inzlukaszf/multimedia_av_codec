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

#ifndef PLUGIN_CORE_PLUGIN_LOADER_H
#define PLUGIN_CORE_PLUGIN_LOADER_H

#include "plugin_base.h"
#include "plugin_definition.h"

namespace OHOS {
namespace MediaAVCodec {
namespace Plugin {
class PluginLoader {
public:
    PluginLoader(void* handler, std::string name); // NOLINT: void*

    PluginLoader(const PluginLoader &) = delete;

    PluginLoader operator=(const PluginLoader &) = delete;

    ~PluginLoader();

    static std::shared_ptr<PluginLoader> Create(const std::string &name, const std::string &path);

    RegisterFunc FetchRegisterFunction() const;

    UnregisterFunc FetchUnregisterFunction() const;

private:
    static void* LoadPluginFile(const std::string &path);

    static std::shared_ptr<PluginLoader> CheckSymbol(void* handler, const std::string &name); // NOLINT: void*

    void UnLoadPluginFile();

private:
    const void *handler_;
    const std::string name_;
    RegisterFunc registerFunc_ {nullptr};
    UnregisterFunc unregisterFunc_ {nullptr};
};
} // namespace Plugin
} // namespace MediaAVCodec
} // namespace OHOS
#endif // PLUGIN_CORE_PLUGIN_LOADER_H
