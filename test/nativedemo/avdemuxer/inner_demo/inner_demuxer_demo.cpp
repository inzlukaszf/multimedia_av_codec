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

#include <cinttypes>
#include "native_avmemory.h"
#include "avdemuxer.h"
#include "inner_demuxer_demo.h"

namespace OHOS {
namespace MediaAVCodec {
InnerDemuxerDemo::InnerDemuxerDemo()
{
    printf("InnerDemuxerDemo is called\n");
}

InnerDemuxerDemo::~InnerDemuxerDemo()
{
    printf("~InnerDemuxerDemo is called\n");
}

int32_t InnerDemuxerDemo::CreateWithSource(std::shared_ptr<AVSource> source)
{
    this->demuxer_ = AVDemuxerFactory::CreateWithSource(source);
    if (!demuxer_) {
        printf("AVDemuxerFactory::CreateWithSource is failed\n");
        return -1;
    }
    return 0;
}
void InnerDemuxerDemo::Destroy()
{
    printf("InnerDemuxerDemo::Destroy\n");
}

int32_t InnerDemuxerDemo::SelectTrackByID(uint32_t trackIndex)
{
    if (this->demuxer_ == nullptr) {
        printf("SelectTrackByID is failed\n");
        return -1;
    }
    int32_t ret = this->demuxer_->SelectTrackByID(trackIndex);
    if (ret != 0) {
        printf("SelectTrackByID is failed\n");
    }
    return ret;
}

int32_t InnerDemuxerDemo::UnselectTrackByID(uint32_t trackIndex)
{
    if (this->demuxer_ == nullptr) {
        printf("UnselectTrackByID is failed\n");
        return -1;
    }
    int32_t ret = this->demuxer_->UnselectTrackByID(trackIndex);
    if (ret != 0) {
        printf("SelectTrackByID is failed\n");
    }
    return ret;
}

int32_t InnerDemuxerDemo::PrintInfo(int32_t tracks)
{
    for (int32_t i = 0; i < tracks; i++) {
        printf("streams[%d]==>total frames=%" PRId64 ",KeyFrames=%" PRId64 "\n", i,
            frames_[i] + key_frames_[i], key_frames_[i]);
    }
    return 0;
}

bool InnerDemuxerDemo::isEOS(std::map<uint32_t, bool>& countFlag)
{
    for (auto iter = countFlag.begin(); iter != countFlag.end(); ++iter) {
        if (!iter->second) {
            return false;
        }
    }
    return true;
}

int32_t InnerDemuxerDemo::StartReferenceParser(int64_t startTimeMs)
{
    if (this->demuxer_ == nullptr) {
        printf("StartReferenceParser is failed\n");
        return -1;
    }
    return this->demuxer_->StartReferenceParser(startTimeMs);
}

int32_t InnerDemuxerDemo::GetFrameLayerInfo(std::shared_ptr<AVBuffer> videoSample, FrameLayerInfo &frameLayerInfo)
{
    if (this->demuxer_ == nullptr) {
        printf("GetFrameLayerInfo is failed\n");
        return -1;
    }
    return this->demuxer_->GetFrameLayerInfo(videoSample, frameLayerInfo);
}

int32_t InnerDemuxerDemo::GetGopLayerInfo(uint32_t gopId, GopLayerInfo &gopLayerInfo)
{
    if (this->demuxer_ == nullptr) {
        printf("GetGopLayerInfo is failed\n");
        return -1;
    }
    return this->demuxer_->GetGopLayerInfo(gopId, gopLayerInfo);
}

int32_t InnerDemuxerDemo::ReadAllSamples(std::shared_ptr<AVSharedMemory> SampleMem, int32_t tracks)
{
    uint32_t bufferFlag = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_NONE;
    std::map<uint32_t, bool> eosFlag;
    for (int i = 0;i < tracks; i++) {
        frames_[i] = 0;
        key_frames_[i] = 0;
        eosFlag[i] = false;
    }
    int32_t ret = -1;
    while (!isEOS(eosFlag)) {
        for (int32_t i = 0; i < tracks; i++) {
            ret = ReadSample(i, SampleMem, sampleInfo, bufferFlag);
            if (ret == 0 && (bufferFlag & AVCODEC_BUFFER_FLAG_EOS)) {
                eosFlag[i] = true;
                continue;
            }
            if (ret == 0 && (bufferFlag & AVCODEC_BUFFER_FLAG_SYNC_FRAME)) {
                key_frames_[i]++;
            } else if (ret == 0 && (bufferFlag & AVCODEC_BUFFER_FLAG_NONE) == 0) {
                frames_[i]++;
            } else {
                printf("the flags is error ret=%d\n", ret);
                printf("the bufferFlag=%d, sampleInfo.size=%d, sampleInfo.pts=%" PRId64 "\n",
                bufferFlag, sampleInfo.size, sampleInfo.presentationTimeUs);
                return ret;
            }
        }
    }
    PrintInfo(tracks);
    return ret;
}

int32_t InnerDemuxerDemo::ReadSample(uint32_t trackIndex, std::shared_ptr<AVSharedMemory> mem,
                                     AVCodecBufferInfo &bufInfo, uint32_t &bufferFlag)
{
    if (this->demuxer_ == nullptr) {
        printf("ReadSample is failed\n");
        return -1;
    }
    int32_t ret = this->demuxer_->ReadSample(trackIndex, mem, bufInfo, bufferFlag);
    if (ret != 0) {
        return ret;
    }
    return ret;
}

int32_t InnerDemuxerDemo::ReadSampleBuffer(uint32_t trackIndex, std::shared_ptr<AVBuffer> sample)
{
    if (this->demuxer_ == nullptr) {
        printf("ReadSampleBuffer is failed\n");
        return -1;
    }
    int32_t ret = this->demuxer_->ReadSampleBuffer(trackIndex, sample);
    if (ret != 0) {
        return ret;
    }
    return ret;
}

int32_t InnerDemuxerDemo::SeekToTime(int64_t millisecond, Media::SeekMode mode)
{
    if (demuxer_ == nullptr) {
        printf("SeekToTime is failed\n");
        return -1;
    }
    int32_t ret = demuxer_->SeekToTime(millisecond, mode);
    if (ret != 0) {
        printf("SeekToTime is failed\n");
    }
    return ret;
}

int32_t InnerDemuxerDemo::GetIndexByRelativePresentationTimeUs(const uint32_t trackIndex,
    const uint64_t relativePresentationTimeUs, uint32_t &index)
{
    if (demuxer_ == nullptr) {
        printf("GetIndexByRelativePresentationTimeUs is failed\n");
        return -1;
    }
    int32_t ret = demuxer_->GetIndexByRelativePresentationTimeUs(trackIndex, relativePresentationTimeUs, index);
    if (ret != 0) {
        printf("GetIndexByRelativePresentationTimeUs is failed\n");
    }
    return ret;
}

int32_t InnerDemuxerDemo::GetRelativePresentationTimeUsByIndex(const uint32_t trackIndex,
    const uint32_t index, uint64_t &relativePresentationTimeUs)
{
    if (demuxer_ == nullptr) {
        printf("GetRelativePresentationTimeUsByIndex is failed\n");
        return -1;
    }
    int32_t ret = demuxer_->GetRelativePresentationTimeUsByIndex(trackIndex, index, relativePresentationTimeUs);
    if (ret != 0) {
        printf("GetRelativePresentationTimeUsByIndex is failed\n");
    }
    return ret;
}
}
}
