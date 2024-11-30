/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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
 
#ifndef HISTREAMER_HLS_MEDIA_DOWNLOADER_H
#define HISTREAMER_HLS_MEDIA_DOWNLOADER_H

#include <mutex>
#include <thread>
#include "playlist_downloader.h"
#include "media_downloader.h"
#include "osal/utils/ring_buffer.h"
#include "osal/utils/steady_clock.h"
#include "openssl/aes.h"
#include "osal/task/task.h"
#include "common/media_source.h"
#include <unistd.h>
#include "common/media_core.h"

namespace OHOS {
namespace Media {
namespace Plugins {
namespace HttpPlugin {

enum BufferingTimes : int32_t {
    FIRST_TIMES = 1,
    SECOND_TIMES = 2,
};

enum SLEEP_TIME : int32_t {
    REQUEST_SLEEP_TIME = 5, // 5ms
    BUFFERING_SLEEP_TIME = 10, // 10ms
    CACHE_DATA_SLEEP_TIME = 100, // 100ms
    BUFFERING_TIME_OUT = 1000, // 100ms
};

class HlsMediaDownloader : public MediaDownloader, public PlayListChangeCallback {
public:
    HlsMediaDownloader() noexcept;
    explicit HlsMediaDownloader(int expectBufferDuration);
    explicit HlsMediaDownloader(std::string mimeType);
    ~HlsMediaDownloader() override;
    bool Open(const std::string& url, const std::map<std::string, std::string>& httpHeader) override;
    void Close(bool isAsync) override;
    void Pause() override;
    void Resume() override;
    Status Read(unsigned char* buff, ReadDataInfo& readDataInfo) override;
    bool SeekToTime(int64_t seekTime, SeekMode mode) override;

    size_t GetContentLength() const override;
    int64_t GetDuration() const override;
    Seekable GetSeekable() const override;
    void SetCallback(Callback* cb) override;
    void OnPlayListChanged(const std::vector<PlayInfo>& playList) override;
    void SetStatusCallback(StatusCallbackFunc cb) override;
    bool GetStartedStatus() override;
    std::vector<uint32_t> GetBitRates() override;
    bool SelectBitRate(uint32_t bitRate) override;
    void OnSourceKeyChange(const uint8_t *key, size_t keyLen, const uint8_t *iv) override;
    void OnDrmInfoChanged(const std::multimap<std::string, std::vector<uint8_t>>& drmInfos) override;
    void SetIsTriggerAutoMode(bool isAuto) override;
    void SetReadBlockingFlag(bool isReadBlockingAllowed) override;
    void SeekToTs(uint64_t seekTime, SeekMode mode);
    void PutRequestIntoDownloader(const PlayInfo& playInfo);
    uint64_t RequestNewTs(uint64_t seekTime, SeekMode mode, double totalDuration,
        double hstTime, const PlayInfo& item);
    void UpdateDownloadFinished(const std::string &url, const std::string& location);
    void AutoSelectBitrate(uint32_t bitRate);
    size_t GetRingBufferSize();
    size_t GetTotalBufferSize();
    void SetInterruptState(bool isInterruptNeeded) override;
    void GetPlaybackInfo(PlaybackInfo& playbackInfo) override;
    void ReportBitrateStart(uint32_t bitRate);
    std::pair<int32_t, int32_t> GetDownloadRateAndSpeed();
    void GetDownloadInfo(DownloadInfo& downloadInfo) override;
    std::pair<int32_t, int32_t> GetDownloadInfo() override;
    void ReportVideoSizeChange();
    Status SetCurrentBitRate(int32_t bitRate, int32_t streamID) override;
private:
    void SaveHttpHeader(const std::map<std::string, std::string>& httpHeader);
    void SetDemuxerState(int32_t streamId) override;
    void SetDownloadErrorState() override;
    bool SaveData(uint8_t* data, uint32_t len);
    Status ReadDelegate(unsigned char* buff, ReadDataInfo& readDataInfo);
    bool SaveEncryptData(uint8_t* data, uint32_t len);
    void InitMediaDownloader();
    void DownloadRecordHistory(int64_t nowTime);
    void OnWriteRingBuffer(uint32_t len);
    void OnReadRingBuffer(uint32_t len);
    double GetAveDownSpeed();
    uint64_t GetMinBuffer();
    void DownloadReport();
    void ReportDownloadSpeed();
    bool CheckRiseBufferSize();
    bool CheckPulldownBufferSize();

    void RiseBufferSize();
    void DownBufferSize();
    void ActiveAutoBufferSize();
    void InActiveAutoBufferSize();
    uint64_t TransferSizeToBitRate(int width);
    void CacheData();
    bool HandleBuffering();
    bool HandleCache();
    bool CheckReadStatus();
    Status CheckPlaylist(unsigned char* buff, ReadDataInfo& readDataInfo);
    bool CheckReadTimeOut();
    bool CheckBreakCondition();
    uint32_t GetDecrptyRealLen(uint8_t* writeDataPoint, uint32_t waitLen, uint32_t writeLen);
    void ResetPlaylistCapacity(size_t size);
    void PlaylistBackup(const PlayInfo& fragment);
    void HandleCachedDuration();
    int32_t GetWaterLineAbove();
    void CaculateBitRate(size_t fragmentSize, double duration);
    double CalculateCurrentDownloadSpeed();
    void UpdateCachedPercent(BufferingInfoType infoType);
    bool CheckBufferingOneSeconds();

private:
    std::shared_ptr<RingBuffer> buffer_;
    size_t totalRingBufferSize_ {0};
    std::atomic<bool> usingExtraRingBuffer_ {false};
    std::shared_ptr<RingBuffer> tmpBuffer_;
    std::shared_ptr<Downloader> downloader_;
    std::shared_ptr<DownloadRequest> downloadRequest_;
    std::mutex mtxLock_;
    Callback* callback_ {nullptr};
    DataSaveFunc dataSave_;
    StatusCallbackFunc statusCallback_;
    bool startedPlayStatus_ {false};

    std::shared_ptr<PlayListDownloader> playListDownloader_;

    std::shared_ptr<BlockingQueue<PlayInfo>> playList_;
    std::map<std::string, bool> fragmentDownloadStart;
    std::map<std::string, bool> fragmentPushed;
    std::deque<PlayInfo> backPlayList_;
    bool isSelectingBitrate_ {false};
    bool isDownloadStarted_ {false};
    static constexpr uint64_t DECRYPT_UNIT_LEN = 16;
    static constexpr uint64_t RING_BUFFER_SIZE = 5 * 1024 * 1024;
    uint8_t afterAlignRemainedBuffer_[DECRYPT_UNIT_LEN] {0};
    uint64_t afterAlignRemainedLength_ = 0;
    uint64_t totalLen_ = 0;
    std::string curUrl_;
    uint8_t key_[16] = {0};
    size_t keyLen_ {0};
    uint8_t iv_[16] = {0};
    AES_KEY aesKey_;
    uint8_t decryptCache_[RING_BUFFER_SIZE] {0};
    uint8_t decryptBuffer_[RING_BUFFER_SIZE] {0};
    int havePlayedTsNum_ = 0;
    bool isAutoSelectBitrate_ {true};
    uint64_t seekTime_ = 0;

    uint64_t readTime_ {0};
    bool isReadFrame_ {false};
    bool isTimeOut_ {false};
    bool downloadErrorState_ {false};
    uint64_t bufferedDuration_ {0};
    uint64_t currentBitrate_ {1 * 1024 * 1024}; // bps
    bool userDefinedBufferDuration_ {false};
    uint64_t expectDuration_ {0};
    bool autoBufferSize_ {true}; // 默认为false
    uint64_t lastCheckTime_ {0};
    uint32_t recordCount_ {0};
    uint64_t lastRecordTime_ {0};
    std::atomic<bool> isInterruptNeeded_{false};

    struct BufferDownRecord {
        /* data */
        uint64_t dataBits {0};
        uint64_t timeoff {0};
        BufferDownRecord* next {nullptr};
    };
    // std::unique_ptr<BufferDownRecord> bufferDownRecord_;
    BufferDownRecord* bufferDownRecord_ {nullptr};
    // buffer least
    struct BufferLeastRecord {
        uint64_t minDuration {0};
        BufferLeastRecord* next {nullptr};
    };
    // std::unique_ptr<BufferLeastRecord> bufferLeastRecord_;
    BufferLeastRecord* bufferLeastRecord_ {nullptr};
    uint64_t lastWriteTime_ {0};
    uint64_t lastReadTime_ {0};
    uint64_t lastWriteBit_ {0};
    SteadyClock steadyClock_;

    uint64_t totalBits_ {0};        // 总下载量

    uint64_t lastBits_ {0};         // 上一统计周期的总下载量

    uint64_t downloadDuringTime_ {0};    // 累计有效下载时长 ms

    uint64_t downloadBits_ {0};          // 累计有效时间内下载数据量 bit
    uint32_t changeBitRateCount_ {0};  // 设置码率次数
    int32_t seekFailedCount_ {0};   // seek失败次数
    int64_t openTime_ {0};
    int64_t playDelayTime_ {0};
    int64_t startDownloadTime_ {0};
    int32_t avgDownloadSpeed_ {0};
    bool isDownloadFinish_ {false};
    double avgSpeedSum_ {0};
    uint32_t recordSpeedCount_ {0};

    int64_t lastReportUsageTime_ {0};
    uint64_t dataUsage_ {0};

    struct RecordData {
        double downloadRate {0};
        uint64_t bufferDuring {0};
        std::shared_ptr<RecordData> next {nullptr};
    };
    std::shared_ptr<RecordData> recordData_ {nullptr};
    std::map<std::string, std::string> httpHeader_ {};
    std::atomic<bool> isStopped = false;
    std::string mimeType_;
    size_t waterLineAbove_ {0};
    bool isInterrupt_ {false};
    bool isBuffering_ {false};
    bool isFirstFrameArrived_ {false};
    std::atomic<bool> isSeekingFlag {false};
    Mutex switchMutex_ {};
    bool isLastDecryptWriteError_ {false};
    uint32_t lastRealLen_ {0};

    uint64_t lastReadCheckTime_ = 0;
    uint64_t readTotalBytes_ = 0;
    uint64_t readRecordDuringTime_ = 0;
    uint64_t totalDownloadDuringTime_ {0};
    int32_t currentBitRate_ {0};
    int32_t fragmentBitRate_ {0};
    uint64_t lastDurationReacord_ {0};
    int32_t lastCachedSize_ {0};
};
}
}
}
}
#endif