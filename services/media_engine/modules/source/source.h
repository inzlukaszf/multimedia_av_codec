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

#ifndef MEDIA_SOURCE_H
#define MEDIA_SOURCE_H

#include <memory>
#include <string>

#include "osal/task/task.h"
#include "osal/utils/util.h"
#include "common/media_source.h"
#include "plugin/plugin_buffer.h"
#include "plugin/plugin_info.h"
#include "plugin/plugin_base.h"
#include "plugin/plugin_manager.h"
#include "plugin/plugin_event.h"
#include "plugin/source_plugin.h"
#include "media_demuxer.h"

namespace OHOS {
namespace Media {
using SourceType = OHOS::Media::Plugins::SourceType;
using MediaSource = OHOS::Media::Plugins::MediaSource;

class CallbackImpl : public Plugins::Callback {
public:
    void OnEvent(const Plugins::PluginEvent &event) override
    {
        callbackWrap_->OnEvent(event);
    }

    void SetCallbackWrap(Callback* callbackWrap)
    {
        callbackWrap_ = callbackWrap;
    }

private:
    Callback* callbackWrap_ {nullptr};
};

class Source : public Plugins::Callback {
public:
    explicit Source();
    ~Source() override;

    Status PullData(uint64_t offset, int64_t seekTime, size_t size, std::shared_ptr<Plugins::Buffer>& data);
    virtual Status SetSource(const std::shared_ptr<MediaSource>& source);
    Status Prepare();
    Status Start();
    Status Stop();
    Status Pause();
    Status Resume();
    Status SetReadBlockingFlag(bool isReadBlockingAllowed);
    Plugins::Seekable GetSeekable();

    Status GetSize(uint64_t &fileSize);

    void OnEvent(const Plugins::PluginEvent &event) override;
    bool IsSeekToTimeSupported();
    Status SetPushData(const std::shared_ptr<PushDataImpl>& data);
    int64_t GetDuration();
    Status SeekToTime(int64_t seekTime);
    Status GetBitRates(std::vector<uint32_t>& bitRates);
    Status SelectBitRate(uint32_t bitRate);
    void SetCallback(Callback* callback);
    bool IsNeedPreDownload();
private:
    void ActivateMode();
    Status InitPlugin(const std::shared_ptr<MediaSource>& source);
    static std::string GetUriSuffix(const std::string& uri);
    void ReadLoop();
    bool GetProtocolByUri();
    bool ParseProtocol(const std::shared_ptr<MediaSource>& source);
    Status CreatePlugin(const std::shared_ptr<Plugins::PluginInfo>& info, const std::string& name,
        Plugins::PluginManager& manager);
    Status FindPlugin(const std::shared_ptr<MediaSource>& source);

    void ClearData();

    std::shared_ptr<Task> taskPtr_;
    std::string protocol_;
    bool isHls_{false};
    std::string uri_;
    Plugins::Seekable seekable_;
    uint64_t position_;
    int64_t mediaOffset_ {0}; // offset used in push mode
    int32_t retryTimes_ {0};

    std::shared_ptr<Plugins::SourcePlugin> plugin_;

    std::shared_ptr<Plugins::PluginInfo> pluginInfo_{};
    bool isPluginReady_ {false};
    bool isAboveWaterline_ {false};

    std::shared_ptr<PushDataImpl> pushData_;
    std::shared_ptr<CallbackImpl> mediaDemuxerCallback_;
};
} // namespace Media
} // namespace OHOS
#endif