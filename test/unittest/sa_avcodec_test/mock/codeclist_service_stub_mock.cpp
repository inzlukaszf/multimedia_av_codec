/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <mutex>
#include "avcodec_log.h"
#include "codeclist_service_stub.h"
#define PRINT_HILOG
#include "unittest_log.h"
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN_TEST, "CodecListServiceStub"};
std::mutex g_mutex;
std::weak_ptr<OHOS::MediaAVCodec::CodecListServiceStubMock> g_mockObject;
} // namespace

namespace OHOS {
namespace MediaAVCodec {
void CodecListServiceStub::RegisterMock(std::shared_ptr<CodecListServiceStubMock> &mock)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    UNITTEST_INFO_LOG("CodecListServiceStub:0x%" PRIXPTR, FAKE_POINTER(mock.get()));
    g_mockObject = mock;
}

sptr<CodecListServiceStub> CodecListServiceStub::Create()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    UNITTEST_INFO_LOG("");
    auto mock = g_mockObject.lock();
    UNITTEST_CHECK_AND_RETURN_RET_LOG(mock != nullptr, nullptr, "mock object is nullptr");
    return mock->Create();
}

CodecListServiceStub::CodecListServiceStub()
{
    UNITTEST_INFO_LOG("");
}

CodecListServiceStub::~CodecListServiceStub()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    UNITTEST_INFO_LOG("");
    auto mock = g_mockObject.lock();
    UNITTEST_CHECK_AND_RETURN_LOG(mock != nullptr, "mock object is nullptr");
    mock->CodecListServiceStubDtor();
}

int32_t CodecListServiceStub::DestroyStub()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    UNITTEST_INFO_LOG("");
    auto mock = g_mockObject.lock();
    UNITTEST_CHECK_AND_RETURN_RET_LOG(mock != nullptr, AVCS_ERR_UNKNOWN, "mock object is nullptr");
    return mock->DestroyStub();
}
} // namespace MediaAVCodec
} // namespace OHOS