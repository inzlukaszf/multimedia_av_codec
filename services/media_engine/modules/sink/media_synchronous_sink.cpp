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

#define HST_LOG_TAG "MediaSyncSink"
#include "media_synchronous_sink.h"
#include "common/log.h"
#include "plugin/plugin_time.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_DOMAIN_SYSTEM_PLAYER, "Histreamer" };
}

namespace OHOS {
namespace Media {
namespace Pipeline {
void MediaSynchronousSink::Init()
{
    auto syncCenter = syncCenter_.lock();
    if (syncCenter) {
        syncCenter->AddSynchronizer(this);
    }
}

MediaSynchronousSink::~MediaSynchronousSink()
{
    MEDIA_LOG_D("~MediaSynchronousSink enter .");
    auto syncCenter = syncCenter_.lock();
    if (syncCenter) {
        syncCenter->RemoveSynchronizer(this);
    }
}

void MediaSynchronousSink::WaitAllPrerolled(bool shouldWait)
{
    waitForPrerolled_ = shouldWait;
}

int8_t MediaSynchronousSink::GetPriority()
{
    return syncerPriority_;
}

void MediaSynchronousSink::ResetPrerollReported()
{
    hasReportedPreroll_ = false;
}

void MediaSynchronousSink::WriteToPluginRefTimeSync(const std::shared_ptr<OHOS::Media::AVBuffer>& buffer)
{
    if (!hasReportedPreroll_) {
        auto syncCenter = syncCenter_.lock();
        if (syncCenter) {
            syncCenter->ReportPrerolled(this);
        }
        hasReportedPreroll_ = true;
    }
    if (waitForPrerolled_) {
        OHOS::Media::AutoLock lock(prerollMutex_);
        if (!prerollCond_.WaitFor(lock, Plugins::HstTime2Ms(waitPrerolledTimeout_),
                                  [&] { return waitForPrerolled_.load(); })) {
            MEDIA_LOG_W("Wait for preroll timeout");
        }
        // no need to wait for preroll next time
        waitForPrerolled_ = false;
    }
    DoSyncWrite(buffer);
}

void MediaSynchronousSink::NotifyAllPrerolled()
{
    OHOS::Media::AutoLock lock(prerollMutex_);
    waitForPrerolled_ = false;
    prerollCond_.NotifyAll();
}

void MediaSynchronousSink::UpdateMediaTimeRange(const std::shared_ptr<Meta>& meta)
{
    FALSE_RETURN_MSG(meta != nullptr, "meta is null!");
    int64_t trackStartTime = 0;
    meta->GetData(Tag::MEDIA_START_TIME, trackStartTime);
    uint32_t trackId = 0;
    FALSE_LOG(meta->GetData(Tag::REGULAR_TRACK_ID, trackId));
    auto syncCenter = syncCenter_.lock();
    if (syncCenter) {
        syncCenter->SetMediaTimeRangeStart(trackStartTime, trackId, this);
    }
    int64_t trackDuration = 0;
    if (meta->GetData(Tag::MEDIA_DURATION, trackDuration)) {
        if (syncCenter) {
            syncCenter->SetMediaTimeRangeEnd(trackDuration + trackStartTime, trackId, this);
        }
    } else {
        MEDIA_LOG_W("Get duration failed");
        if (syncCenter) {
            syncCenter->SetMediaTimeRangeEnd(INT64_MAX, trackId, this);
        }
    }
}
} // namespace Pipeline
} // namespace Media
} // namespace OHOS