/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef DEMUXER_FACTORY_H
#define DEMUXER_FACTORY_H

#include <map>
#include <vector>
#include "plugin_info.h"
#include "demuxer.h"
#include "plugin_loader.h"
#include "plugin_definition.h"

namespace OHOS {
namespace MediaAVCodec {
namespace Plugin {
class DemuxerFactory {
public:
    DemuxerFactory(const DemuxerFactory&) = delete;
    DemuxerFactory operator=(const DemuxerFactory&) = delete;
    ~DemuxerFactory();
    static DemuxerFactory& Instance()
    {
        static DemuxerFactory impl;
        return impl;
    }

    std::shared_ptr<Demuxer> CreatePlugin(uintptr_t sourceAddr);
    DemuxerFactory();
private:
    void RegisterPlugins();
    void RegisterDynamicPlugins(const char* libDirPath);
    void UnregisterAllPlugins();

private:
    struct PluginRegInfo {
        std::shared_ptr<PackageDef> packageDef;
        std::shared_ptr<PluginInfo> info;
        DemuxerPluginCreatorFunc creator;
        DemuxerPluginSnifferFunc sniffer;
        std::shared_ptr<PluginLoader> loader;
    };
    struct RegisterData {
        std::vector<std::string> registerNames;
        std::map<std::string, std::shared_ptr<PluginRegInfo>> registerTable;
        bool IsPluginExist(const std::string& name);
    };
    struct RegisterImpl : PackageRegister {
        explicit RegisterImpl(std::shared_ptr<RegisterData> data, std::shared_ptr<PluginLoader> loader = nullptr)
            : pluginLoader(std::move(loader)), registerData(std::move(data)) {}
        
        ~RegisterImpl() override = default;
        
        Status AddPlugin(const PluginDefBase& def) override;
        Status AddPackage(const PackageDef& def) override;
        Status SetPackageDef(const PackageDef& def);
        bool Verification(const PluginDefBase& definition);
        bool VersionMatched(const PluginDefBase& definition);

        std::shared_ptr<PluginLoader> pluginLoader;
        std::shared_ptr<RegisterData> registerData;
        std::shared_ptr<PackageDef> packageDef {nullptr};
    };

    std::shared_ptr<RegisterData> registerData_ = std::make_shared<RegisterData>();
    std::vector<std::shared_ptr<PluginLoader>> registeredLoaders_;
};
} // namespace Plugin
} // namespace MediaAVCodec
} // namespace OHOS
#endif // DEMUXER_FACTORY_H
