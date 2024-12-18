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

#ifndef CODECLIST_SERVICE_PROXY_H
#define CODECLIST_SERVICE_PROXY_H

#include "i_standard_codeclist_service.h"
#include "nocopyable.h"

namespace OHOS {
namespace MediaAVCodec {
class CodecListServiceProxy : public IRemoteProxy<IStandardCodecListService>, public NoCopyable {
public:
    explicit CodecListServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~CodecListServiceProxy();

    std::string FindDecoder(const Media::Format &format) override;
    std::string FindEncoder(const Media::Format &format) override;
    int32_t GetCapability(CapabilityData &capabilityData, const std::string &mime, const bool isEncoder,
                          const AVCodecCategory &category) override;
    int32_t DestroyStub() override;

private:
    static inline BrokerDelegator<CodecListServiceProxy> delegator_;
};
} // namespace MediaAVCodec
} // namespace OHOS
#endif // CODECLIST_SERVICE_PROXY_H
