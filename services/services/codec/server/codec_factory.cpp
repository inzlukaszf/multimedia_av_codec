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

#include "codec_factory.h"
#include <cinttypes>
#include <dlfcn.h>
#include <limits>
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "codec_ability_singleton.h"
#include "codeclist_core.h"
#include "meta/format.h"
#ifdef CLIENT_SUPPORT_CODEC
#include "audio_codec.h"
#include "audio_codec_adapter.h"
#else
#include "fcodec_loader.h"
#include "hevc_decoder_loader.h"
#include "hcodec_loader.h"
#endif

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN_FRAMEWORK, "CodecFactory"};
} // namespace

namespace OHOS {
namespace MediaAVCodec {
CodecFactory &CodecFactory::Instance()
{
    static CodecFactory inst;
    return inst;
}

CodecFactory::~CodecFactory() {}

std::vector<std::string> CodecFactory::GetCodecNameArrayByMime(const std::string &mime, const bool isEncoder)
{
    auto codecListCore = std::make_shared<CodecListCore>();
    auto nameArray = codecListCore->FindCodecNameArray(mime, isEncoder);
#ifndef CLIENT_SUPPORT_CODEC
    auto checkFunc = [](const std::string &str) { return str.find("secure") != std::string::npos; };
    nameArray.erase(std::remove_if(nameArray.begin(), nameArray.end(), checkFunc), nameArray.end());
#endif
    return nameArray;
}

std::shared_ptr<CodecBase> CodecFactory::CreateCodecByName(const std::string &name, API_VERSION apiVersion)
{
    std::shared_ptr<CodecListCore> codecListCore = std::make_shared<CodecListCore>();
    CodecType codecType = codecListCore->FindCodecType(name);
    std::shared_ptr<CodecBase> codec = nullptr;
    switch (codecType) {
#ifndef CLIENT_SUPPORT_CODEC
        case CodecType::AVCODEC_HCODEC:
            codec = HCodecLoader::CreateByName(name);
            break;
        case CodecType::AVCODEC_VIDEO_CODEC:
            codec = FCodecLoader::CreateByName(name);
            break;
        case CodecType::AVCODEC_VIDEO_HEVC_DECODER:
            codec = HevcDecoderLoader::CreateByName(name);
            break;
#else
        case CodecType::AVCODEC_AUDIO_CODEC:
            if (apiVersion == API_VERSION::API_VERSION_10) {
                codec = std::make_shared<AudioCodecAdapter>(name);
            } else {
                codec = std::make_shared<AudioCodec>();
                auto ret = codec->CreateCodecByName(name);
                CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Create codec by name:%{public}s failed",
                                         name.c_str());
            }
            break;
#endif
        default:
            AVCODEC_LOGE("Create codec %{public}s failed", name.c_str());
            return codec;
    }
    (void)apiVersion;
    EXPECT_AND_LOGI(codec != nullptr, "Create codec %{public}s successful", name.c_str());
    return codec;
}
} // namespace MediaAVCodec
} // namespace OHOS
