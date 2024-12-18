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

#include <fcntl.h>
#include <iostream>
#include <memory>
#include <vector>
#include "meta/meta.h"
#include "avcodec_errors.h"
#include "avcodec_codec_name.h"
#include "unittest_log.h"
#include "drm_decryptor_coverage_unit_test.h"

using namespace OHOS;
using namespace OHOS::MediaAVCodec;
using namespace OHOS::Media;
using namespace testing;
using namespace testing::ext;

namespace {
const uint32_t KEY_ID_LEN = 16;
const uint32_t IV_LEN = 16;

// for H264
const uint8_t H264_KEY_ID[] = { 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53,
                                0x54, 0x5f, 0x43, 0x49, 0x44, 0x5f, 0x30, 0x37};
const uint8_t H264_IV[] = { 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53,
                            0x54, 0x5f, 0x43, 0x49, 0x44, 0x5f, 0x30, 0x37};
uint32_t H264_ENCRYPTED_BUFFER_SIZE = 418;
const uint8_t H264_SM4C_ENCRYPTED_BUFFER[] = {
0x00, 0x00, 0x00, 0x01, 0x09, 0x10, 0x00, 0x00, 0x00, 0x01, 0x67, 0x64, 0x00, 0x1f, 0xac, 0xd3,
0x01, 0xe0, 0x08, 0x9f, 0x97, 0x01, 0x10, 0x00, 0x00, 0x03, 0x00, 0x10, 0x00, 0x00, 0x03, 0x03,
0xce, 0x64, 0x00, 0x01, 0x31, 0x2d, 0x00, 0x00, 0x2f, 0xaf, 0x0a, 0x27, 0x28, 0x07, 0x8c, 0x18,
0x9c, 0x00, 0x00, 0x00, 0x01, 0x68, 0xea, 0xac, 0xb2, 0x2c, 0x00, 0x00, 0x01, 0x06, 0x00, 0x05,
0x98, 0xb8, 0x00, 0xaf, 0xd4, 0x80, 0x00, 0x00, 0x01, 0x06, 0x05, 0x26, 0xdc, 0x45, 0xe9, 0xbd,
0xe6, 0xd9, 0x48, 0xb7, 0x96, 0x2c, 0xd8, 0x20, 0xd9, 0x23, 0xee, 0xef, 0x73, 0x75, 0x6d, 0x61,
0x20, 0x76, 0x69, 0x64, 0x65, 0x6f, 0x2c, 0x76, 0x32, 0x30, 0x31, 0x36, 0x20, 0x2d, 0x31, 0x34,
0x38, 0x00, 0x80, 0x00, 0x00, 0x01, 0x06, 0x01, 0x02, 0x00, 0x0a, 0x80, 0x00, 0x00, 0x01, 0x06,
0x05, 0xc8, 0x70, 0xc1, 0xdb, 0x9f, 0x66, 0xae, 0x41, 0x27, 0xbf, 0xc0, 0xbb, 0x19, 0x81, 0x69,
0x4b, 0x66, 0xf0, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44,
0x5f, 0x30, 0x37, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44,
0x5f, 0x30, 0x37, 0x10, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49,
0x44, 0x5f, 0x30, 0x37, 0xc0, 0x84, 0x43, 0x00, 0x00, 0x03, 0x00, 0x00, 0x82, 0x70, 0x73, 0x73,
0x68, 0x00, 0x00, 0x03, 0x00, 0x00, 0x3d, 0x5e, 0x6d, 0x35, 0x9b, 0x9a, 0x41, 0xe8, 0xb8, 0x43,
0xdd, 0x3c, 0x6e, 0x72, 0xc4, 0x2c, 0x00, 0x00, 0x03, 0x00, 0x62, 0x7b, 0x22, 0x76, 0x65, 0x72,
0x73, 0x69, 0x6f, 0x6e, 0x22, 0x3a, 0x22, 0x56, 0x31, 0x2e, 0x30, 0x22, 0x2c, 0x22, 0x63, 0x6f,
0x6e, 0x74, 0x65, 0x6e, 0x74, 0x49, 0x44, 0x22, 0x3a, 0x22, 0x61, 0x44, 0x49, 0x32, 0x4e, 0x43,
0x31, 0x7a, 0x62, 0x54, 0x52, 0x6a, 0x4c, 0x58, 0x52, 0x7a, 0x4c, 0x54, 0x41, 0x3d, 0x22, 0x2c,
0x22, 0x6b, 0x69, 0x64, 0x73, 0x22, 0x3a, 0x5b, 0x5d, 0x2c, 0x22, 0x65, 0x6e, 0x73, 0x63, 0x68,
0x65, 0x6d, 0x61, 0x22, 0x3a, 0x22, 0x73, 0x6d, 0x34, 0x63, 0x22, 0x2c, 0x22, 0x65, 0x78, 0x74,
0x73, 0x22, 0x3a, 0x22, 0x74, 0x73, 0x2f, 0x68, 0x32, 0x36, 0x34, 0x22, 0x7d, 0x80, 0x00, 0x00,
0x00, 0x01, 0x65, 0x88, 0x84, 0x00, 0xff, 0x9f, 0x97, 0x12, 0x77, 0x06, 0xff, 0x81, 0x48, 0xf0,
0x84, 0xc3, 0xfb, 0x35, 0x95, 0x4d, 0x40, 0x94, 0x6e, 0x1a, 0xd3, 0x6f, 0xb0, 0xc3, 0xae, 0x35,
0x01, 0x6f, 0x46, 0x26, 0xae, 0x2b, 0x46, 0x6b, 0x81, 0x9f, 0xc1, 0xf9, 0x46, 0x86, 0x5b, 0x62,
0xaa, 0x0d, 0x2a, 0x39, 0xc0, 0xae, 0x2d, 0x52, 0xef, 0x97, 0x6e, 0xd5, 0x78, 0xec, 0xa4, 0x25,
0xee, 0xa7, 0xa6, 0xae, 0xed, 0xf3, 0xe7, 0xdf, 0xef, 0xd8, 0x84, 0xb4, 0x45, 0xd4, 0xfb, 0x67,
0x8c, 0x6c}; //418
const uint8_t H264_SM4S_ENCRYPTED_BUFFER[] = {
0x00, 0x00, 0x00, 0x01, 0x09, 0x10, 0x00, 0x00, 0x00, 0x01, 0x67, 0x64, 0x00, 0x1f, 0xac, 0xd3,
0x01, 0xe0, 0x08, 0x9f, 0x97, 0x01, 0x10, 0x00, 0x00, 0x03, 0x00, 0x10, 0x00, 0x00, 0x03, 0x03,
0xce, 0x64, 0x00, 0x01, 0x31, 0x2d, 0x00, 0x00, 0x2f, 0xaf, 0x0a, 0x27, 0x28, 0x07, 0x8c, 0x18,
0x9c, 0x00, 0x00, 0x00, 0x01, 0x68, 0xea, 0xac, 0xb2, 0x2c, 0x00, 0x00, 0x01, 0x06, 0x00, 0x05,
0x98, 0xb8, 0x00, 0xaf, 0xd4, 0x80, 0x00, 0x00, 0x01, 0x06, 0x05, 0x26, 0xdc, 0x45, 0xe9, 0xbd,
0xe6, 0xd9, 0x48, 0xb7, 0x96, 0x2c, 0xd8, 0x20, 0xd9, 0x23, 0xee, 0xef, 0x73, 0x75, 0x6d, 0x61,
0x20, 0x76, 0x69, 0x64, 0x65, 0x6f, 0x2c, 0x76, 0x32, 0x30, 0x31, 0x36, 0x20, 0x2d, 0x31, 0x34,
0x38, 0x00, 0x80, 0x00, 0x00, 0x01, 0x06, 0x01, 0x02, 0x00, 0x0a, 0x80, 0x00, 0x00, 0x01, 0x06,
0x05, 0xc8, 0x70, 0xc1, 0xdb, 0x9f, 0x66, 0xae, 0x41, 0x27, 0xbf, 0xc0, 0xbb, 0x19, 0x81, 0x69,
0x4b, 0x66, 0xf0, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44,
0x5f, 0x30, 0x37, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44,
0x5f, 0x30, 0x37, 0x10, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49,
0x44, 0x5f, 0x30, 0x37, 0xc0, 0x84, 0x41, 0x00, 0x00, 0x03, 0x00, 0x00, 0x82, 0x70, 0x73, 0x73,
0x68, 0x00, 0x00, 0x03, 0x00, 0x00, 0x3d, 0x5e, 0x6d, 0x35, 0x9b, 0x9a, 0x41, 0xe8, 0xb8, 0x43,
0xdd, 0x3c, 0x6e, 0x72, 0xc4, 0x2c, 0x00, 0x00, 0x03, 0x00, 0x62, 0x7b, 0x22, 0x76, 0x65, 0x72,
0x73, 0x69, 0x6f, 0x6e, 0x22, 0x3a, 0x22, 0x56, 0x31, 0x2e, 0x30, 0x22, 0x2c, 0x22, 0x63, 0x6f,
0x6e, 0x74, 0x65, 0x6e, 0x74, 0x49, 0x44, 0x22, 0x3a, 0x22, 0x61, 0x44, 0x49, 0x32, 0x4e, 0x43,
0x31, 0x7a, 0x62, 0x54, 0x52, 0x6a, 0x4c, 0x58, 0x52, 0x7a, 0x4c, 0x54, 0x41, 0x3d, 0x22, 0x2c,
0x22, 0x6b, 0x69, 0x64, 0x73, 0x22, 0x3a, 0x5b, 0x5d, 0x2c, 0x22, 0x65, 0x6e, 0x73, 0x63, 0x68,
0x65, 0x6d, 0x61, 0x22, 0x3a, 0x22, 0x73, 0x6d, 0x34, 0x63, 0x22, 0x2c, 0x22, 0x65, 0x78, 0x74,
0x73, 0x22, 0x3a, 0x22, 0x74, 0x73, 0x2f, 0x68, 0x32, 0x36, 0x34, 0x22, 0x7d, 0x80, 0x00, 0x00,
0x00, 0x01, 0x65, 0x88, 0x84, 0x00, 0xff, 0x9f, 0x97, 0x12, 0x77, 0x06, 0xff, 0x81, 0x48, 0xf0,
0x84, 0xc3, 0xfb, 0x35, 0x95, 0x4d, 0x40, 0x94, 0x6e, 0x1a, 0xd3, 0x6f, 0xb0, 0xc3, 0xae, 0x35,
0x01, 0x6f, 0x46, 0x26, 0xae, 0x2b, 0x46, 0x6b, 0x81, 0x9f, 0xc1, 0xf9, 0x46, 0x86, 0x5b, 0x62,
0xaa, 0x0d, 0x2a, 0x39, 0xc0, 0xae, 0x2d, 0x52, 0xef, 0x97, 0x6e, 0xd5, 0x78, 0xec, 0xa4, 0x25,
0xee, 0xa7, 0xa6, 0xae, 0xed, 0xf3, 0xe7, 0xdf, 0xef, 0xd8, 0x84, 0xb4, 0x45, 0xd4, 0xfb, 0x67,
0x8c, 0x6c}; //418
const uint8_t H264_CBC1_ENCRYPTED_BUFFER[] = {
0x00, 0x00, 0x00, 0x01, 0x09, 0x10, 0x00, 0x00, 0x00, 0x01, 0x67, 0x64, 0x00, 0x1f, 0xac, 0xd3,
0x01, 0xe0, 0x08, 0x9f, 0x97, 0x01, 0x10, 0x00, 0x00, 0x03, 0x00, 0x10, 0x00, 0x00, 0x03, 0x03,
0xce, 0x64, 0x00, 0x01, 0x31, 0x2d, 0x00, 0x00, 0x2f, 0xaf, 0x0a, 0x27, 0x28, 0x07, 0x8c, 0x18,
0x9c, 0x00, 0x00, 0x00, 0x01, 0x68, 0xea, 0xac, 0xb2, 0x2c, 0x00, 0x00, 0x01, 0x06, 0x00, 0x05,
0x98, 0xb8, 0x00, 0xaf, 0xd4, 0x80, 0x00, 0x00, 0x01, 0x06, 0x05, 0x26, 0xdc, 0x45, 0xe9, 0xbd,
0xe6, 0xd9, 0x48, 0xb7, 0x96, 0x2c, 0xd8, 0x20, 0xd9, 0x23, 0xee, 0xef, 0x73, 0x75, 0x6d, 0x61,
0x20, 0x76, 0x69, 0x64, 0x65, 0x6f, 0x2c, 0x76, 0x32, 0x30, 0x31, 0x36, 0x20, 0x2d, 0x31, 0x34,
0x38, 0x00, 0x80, 0x00, 0x00, 0x01, 0x06, 0x01, 0x02, 0x00, 0x0a, 0x80, 0x00, 0x00, 0x01, 0x06,
0x05, 0xc8, 0x70, 0xc1, 0xdb, 0x9f, 0x66, 0xae, 0x41, 0x27, 0xbf, 0xc0, 0xbb, 0x19, 0x81, 0x69,
0x4b, 0x66, 0xf0, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44,
0x5f, 0x30, 0x37, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44,
0x5f, 0x30, 0x37, 0x10, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49,
0x44, 0x5f, 0x30, 0x37, 0xc0, 0x84, 0x45, 0x00, 0x00, 0x03, 0x00, 0x00, 0x82, 0x70, 0x73, 0x73,
0x68, 0x00, 0x00, 0x03, 0x00, 0x00, 0x3d, 0x5e, 0x6d, 0x35, 0x9b, 0x9a, 0x41, 0xe8, 0xb8, 0x43,
0xdd, 0x3c, 0x6e, 0x72, 0xc4, 0x2c, 0x00, 0x00, 0x03, 0x00, 0x62, 0x7b, 0x22, 0x76, 0x65, 0x72,
0x73, 0x69, 0x6f, 0x6e, 0x22, 0x3a, 0x22, 0x56, 0x31, 0x2e, 0x30, 0x22, 0x2c, 0x22, 0x63, 0x6f,
0x6e, 0x74, 0x65, 0x6e, 0x74, 0x49, 0x44, 0x22, 0x3a, 0x22, 0x61, 0x44, 0x49, 0x32, 0x4e, 0x43,
0x31, 0x7a, 0x62, 0x54, 0x52, 0x6a, 0x4c, 0x58, 0x52, 0x7a, 0x4c, 0x54, 0x41, 0x3d, 0x22, 0x2c,
0x22, 0x6b, 0x69, 0x64, 0x73, 0x22, 0x3a, 0x5b, 0x5d, 0x2c, 0x22, 0x65, 0x6e, 0x73, 0x63, 0x68,
0x65, 0x6d, 0x61, 0x22, 0x3a, 0x22, 0x73, 0x6d, 0x34, 0x63, 0x22, 0x2c, 0x22, 0x65, 0x78, 0x74,
0x73, 0x22, 0x3a, 0x22, 0x74, 0x73, 0x2f, 0x68, 0x32, 0x36, 0x34, 0x22, 0x7d, 0x80, 0x00, 0x00,
0x00, 0x01, 0x65, 0x88, 0x84, 0x00, 0xff, 0x9f, 0x97, 0x12, 0x77, 0x06, 0xff, 0x81, 0x48, 0xf0,
0x84, 0xc3, 0xfb, 0x35, 0x95, 0x4d, 0x40, 0x94, 0x6e, 0x1a, 0xd3, 0x6f, 0xb0, 0xc3, 0xae, 0x35,
0x01, 0x6f, 0x46, 0x26, 0xae, 0x2b, 0x46, 0x6b, 0x81, 0x9f, 0xc1, 0xf9, 0x46, 0x86, 0x5b, 0x62,
0xaa, 0x0d, 0x2a, 0x39, 0xc0, 0xae, 0x2d, 0x52, 0xef, 0x97, 0x6e, 0xd5, 0x78, 0xec, 0xa4, 0x25,
0xee, 0xa7, 0xa6, 0xae, 0xed, 0xf3, 0xe7, 0xdf, 0xef, 0xd8, 0x84, 0xb4, 0x45, 0xd4, 0xfb, 0x67,
0x8c, 0x6c}; //418
const uint8_t H264_CBCS_ENCRYPTED_BUFFER[] = {
0x00, 0x00, 0x00, 0x01, 0x09, 0x10, 0x00, 0x00, 0x00, 0x01, 0x67, 0x64, 0x00, 0x1f, 0xac, 0xd3,
0x01, 0xe0, 0x08, 0x9f, 0x97, 0x01, 0x10, 0x00, 0x00, 0x03, 0x00, 0x10, 0x00, 0x00, 0x03, 0x03,
0xce, 0x64, 0x00, 0x01, 0x31, 0x2d, 0x00, 0x00, 0x2f, 0xaf, 0x0a, 0x27, 0x28, 0x07, 0x8c, 0x18,
0x9c, 0x00, 0x00, 0x00, 0x01, 0x68, 0xea, 0xac, 0xb2, 0x2c, 0x00, 0x00, 0x01, 0x06, 0x00, 0x05,
0x98, 0xb8, 0x00, 0xaf, 0xd4, 0x80, 0x00, 0x00, 0x01, 0x06, 0x05, 0x26, 0xdc, 0x45, 0xe9, 0xbd,
0xe6, 0xd9, 0x48, 0xb7, 0x96, 0x2c, 0xd8, 0x20, 0xd9, 0x23, 0xee, 0xef, 0x73, 0x75, 0x6d, 0x61,
0x20, 0x76, 0x69, 0x64, 0x65, 0x6f, 0x2c, 0x76, 0x32, 0x30, 0x31, 0x36, 0x20, 0x2d, 0x31, 0x34,
0x38, 0x00, 0x80, 0x00, 0x00, 0x01, 0x06, 0x01, 0x02, 0x00, 0x0a, 0x80, 0x00, 0x00, 0x01, 0x06,
0x05, 0xc8, 0x70, 0xc1, 0xdb, 0x9f, 0x66, 0xae, 0x41, 0x27, 0xbf, 0xc0, 0xbb, 0x19, 0x81, 0x69,
0x4b, 0x66, 0xf0, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44,
0x5f, 0x30, 0x37, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44,
0x5f, 0x30, 0x37, 0x10, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49,
0x44, 0x5f, 0x30, 0x37, 0xc0, 0x84, 0x42, 0x00, 0x00, 0x03, 0x00, 0x00, 0x82, 0x70, 0x73, 0x73,
0x68, 0x00, 0x00, 0x03, 0x00, 0x00, 0x3d, 0x5e, 0x6d, 0x35, 0x9b, 0x9a, 0x41, 0xe8, 0xb8, 0x43,
0xdd, 0x3c, 0x6e, 0x72, 0xc4, 0x2c, 0x00, 0x00, 0x03, 0x00, 0x62, 0x7b, 0x22, 0x76, 0x65, 0x72,
0x73, 0x69, 0x6f, 0x6e, 0x22, 0x3a, 0x22, 0x56, 0x31, 0x2e, 0x30, 0x22, 0x2c, 0x22, 0x63, 0x6f,
0x6e, 0x74, 0x65, 0x6e, 0x74, 0x49, 0x44, 0x22, 0x3a, 0x22, 0x61, 0x44, 0x49, 0x32, 0x4e, 0x43,
0x31, 0x7a, 0x62, 0x54, 0x52, 0x6a, 0x4c, 0x58, 0x52, 0x7a, 0x4c, 0x54, 0x41, 0x3d, 0x22, 0x2c,
0x22, 0x6b, 0x69, 0x64, 0x73, 0x22, 0x3a, 0x5b, 0x5d, 0x2c, 0x22, 0x65, 0x6e, 0x73, 0x63, 0x68,
0x65, 0x6d, 0x61, 0x22, 0x3a, 0x22, 0x73, 0x6d, 0x34, 0x63, 0x22, 0x2c, 0x22, 0x65, 0x78, 0x74,
0x73, 0x22, 0x3a, 0x22, 0x74, 0x73, 0x2f, 0x68, 0x32, 0x36, 0x34, 0x22, 0x7d, 0x80, 0x00, 0x00,
0x00, 0x01, 0x65, 0x88, 0x84, 0x00, 0xff, 0x9f, 0x97, 0x12, 0x77, 0x06, 0xff, 0x81, 0x48, 0xf0,
0x84, 0xc3, 0xfb, 0x35, 0x95, 0x4d, 0x40, 0x94, 0x6e, 0x1a, 0xd3, 0x6f, 0xb0, 0xc3, 0xae, 0x35,
0x01, 0x6f, 0x46, 0x26, 0xae, 0x2b, 0x46, 0x6b, 0x81, 0x9f, 0xc1, 0xf9, 0x46, 0x86, 0x5b, 0x62,
0xaa, 0x0d, 0x2a, 0x39, 0xc0, 0xae, 0x2d, 0x52, 0xef, 0x97, 0x6e, 0xd5, 0x78, 0xec, 0xa4, 0x25,
0xee, 0xa7, 0xa6, 0xae, 0xed, 0xf3, 0xe7, 0xdf, 0xef, 0xd8, 0x84, 0xb4, 0x45, 0xd4, 0xfb, 0x67,
0x8c, 0x6c}; //418
const uint8_t H264_NONE_ENCRYPTED_BUFFER[] = {
0x00, 0x00, 0x00, 0x01, 0x09, 0x10, 0x00, 0x00, 0x00, 0x01, 0x67, 0x64, 0x00, 0x1f, 0xac, 0xd3,
0x01, 0xe0, 0x08, 0x9f, 0x97, 0x01, 0x10, 0x00, 0x00, 0x03, 0x00, 0x10, 0x00, 0x00, 0x03, 0x03,
0xce, 0x64, 0x00, 0x01, 0x31, 0x2d, 0x00, 0x00, 0x2f, 0xaf, 0x0a, 0x27, 0x28, 0x07, 0x8c, 0x18,
0x9c, 0x00, 0x00, 0x00, 0x01, 0x68, 0xea, 0xac, 0xb2, 0x2c, 0x00, 0x00, 0x01, 0x06, 0x00, 0x05,
0x98, 0xb8, 0x00, 0xaf, 0xd4, 0x80, 0x00, 0x00, 0x01, 0x06, 0x05, 0x26, 0xdc, 0x45, 0xe9, 0xbd,
0xe6, 0xd9, 0x48, 0xb7, 0x96, 0x2c, 0xd8, 0x20, 0xd9, 0x23, 0xee, 0xef, 0x73, 0x75, 0x6d, 0x61,
0x20, 0x76, 0x69, 0x64, 0x65, 0x6f, 0x2c, 0x76, 0x32, 0x30, 0x31, 0x36, 0x20, 0x2d, 0x31, 0x34,
0x38, 0x00, 0x80, 0x00, 0x00, 0x01, 0x06, 0x01, 0x02, 0x00, 0x0a, 0x80, 0x00, 0x00, 0x01, 0x06,
0x05, 0xc8, 0x70, 0xc1, 0xdb, 0x9f, 0x66, 0xae, 0x41, 0x27, 0xbf, 0xc0, 0xbb, 0x19, 0x81, 0x69,
0x4b, 0x66, 0xf0, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44,
0x5f, 0x30, 0x37, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44,
0x5f, 0x30, 0x37, 0x10, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49,
0x44, 0x5f, 0x30, 0x37, 0xc0, 0x84, 0x40, 0x00, 0x00, 0x03, 0x00, 0x00, 0x82, 0x70, 0x73, 0x73,
0x68, 0x00, 0x00, 0x03, 0x00, 0x00, 0x3d, 0x5e, 0x6d, 0x35, 0x9b, 0x9a, 0x41, 0xe8, 0xb8, 0x43,
0xdd, 0x3c, 0x6e, 0x72, 0xc4, 0x2c, 0x00, 0x00, 0x03, 0x00, 0x62, 0x7b, 0x22, 0x76, 0x65, 0x72,
0x73, 0x69, 0x6f, 0x6e, 0x22, 0x3a, 0x22, 0x56, 0x31, 0x2e, 0x30, 0x22, 0x2c, 0x22, 0x63, 0x6f,
0x6e, 0x74, 0x65, 0x6e, 0x74, 0x49, 0x44, 0x22, 0x3a, 0x22, 0x61, 0x44, 0x49, 0x32, 0x4e, 0x43,
0x31, 0x7a, 0x62, 0x54, 0x52, 0x6a, 0x4c, 0x58, 0x52, 0x7a, 0x4c, 0x54, 0x41, 0x3d, 0x22, 0x2c,
0x22, 0x6b, 0x69, 0x64, 0x73, 0x22, 0x3a, 0x5b, 0x5d, 0x2c, 0x22, 0x65, 0x6e, 0x73, 0x63, 0x68,
0x65, 0x6d, 0x61, 0x22, 0x3a, 0x22, 0x73, 0x6d, 0x34, 0x63, 0x22, 0x2c, 0x22, 0x65, 0x78, 0x74,
0x73, 0x22, 0x3a, 0x22, 0x74, 0x73, 0x2f, 0x68, 0x32, 0x36, 0x34, 0x22, 0x7d, 0x80, 0x00, 0x00,
0x00, 0x01, 0x65, 0x88, 0x84, 0x00, 0xff, 0x9f, 0x97, 0x12, 0x77, 0x06, 0xff, 0x81, 0x48, 0xf0,
0x84, 0xc3, 0xfb, 0x35, 0x95, 0x4d, 0x40, 0x94, 0x6e, 0x1a, 0xd3, 0x6f, 0xb0, 0xc3, 0xae, 0x35,
0x01, 0x6f, 0x46, 0x26, 0xae, 0x2b, 0x46, 0x6b, 0x81, 0x9f, 0xc1, 0xf9, 0x46, 0x86, 0x5b, 0x62,
0xaa, 0x0d, 0x2a, 0x39, 0xc0, 0xae, 0x2d, 0x52, 0xef, 0x97, 0x6e, 0xd5, 0x78, 0xec, 0xa4, 0x25,
0xee, 0xa7, 0xa6, 0xae, 0xed, 0xf3, 0xe7, 0xdf, 0xef, 0xd8, 0x84, 0xb4, 0x45, 0xd4, 0xfb, 0x67,
0x8c, 0x6c}; //418
uint32_t H264_CLEAR_BUFFER_SIZE = 215;
const uint8_t H264_CLEAR_BUFFER[] = {
0x00, 0x00, 0x00, 0x01, 0x09, 0x10, 0x00, 0x00, 0x00, 0x01, 0x67, 0x64, 0x00, 0x1f, 0xac, 0xd3,
0x01, 0xe0, 0x08, 0x9f, 0x97, 0x01, 0x10, 0x00, 0x00, 0x03, 0x00, 0x10, 0x00, 0x00, 0x03, 0x03,
0xce, 0x64, 0x00, 0x01, 0x31, 0x2d, 0x00, 0x00, 0x2f, 0xaf, 0x0a, 0x27, 0x28, 0x07, 0x8c, 0x18,
0x9c, 0x00, 0x00, 0x00, 0x01, 0x68, 0xea, 0xac, 0xb2, 0x2c, 0x00, 0x00, 0x01, 0x06, 0x00, 0x05,
0x98, 0xb8, 0x00, 0xaf, 0xd4, 0x80, 0x00, 0x00, 0x01, 0x06, 0x05, 0x26, 0xdc, 0x45, 0xe9, 0xbd,
0xe6, 0xd9, 0x48, 0xb7, 0x96, 0x2c, 0xd8, 0x20, 0xd9, 0x23, 0xee, 0xef, 0x73, 0x75, 0x6d, 0x61,
0x20, 0x76, 0x69, 0x64, 0x65, 0x6f, 0x2c, 0x76, 0x32, 0x30, 0x31, 0x36, 0x20, 0x2d, 0x31, 0x34,
0x38, 0x00, 0x80, 0x00, 0x00, 0x01, 0x06, 0x01, 0x02, 0x00, 0x0a, 0x80, 0x00, 0x00, 0x01, 0x06,
0x05, 0xc8, 0x70, 0xc1, 0xdb, 0x9f, 0x66, 0xae, 0x41, 0x27, 0xbf, 0xc0, 0xbb, 0x19, 0x81, 0x69,
0x4b, 0x66, 0x10, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44,
0x5f, 0x30, 0x37, 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44,
0x5f, 0x30, 0x37, 0x10, 0x00, 0x00, 0x01, 0x65, 0x88, 0x84, 0x00, 0xff, 0x9f, 0x97, 0x12, 0x77,
0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44, 0x5f, 0x30, 0x32,
0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45}; //215

// for H265
const uint8_t H265_SM4C_KEY_ID[] = { 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53,
                                     0x54, 0x5f, 0x43, 0x49, 0x44, 0x5f, 0x30, 0x32};
const uint8_t H265_SM4C_IV[] = { 0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53,
                                 0x54, 0x5f, 0x43, 0x49, 0x44, 0x5f, 0x30, 0x32};
uint32_t H265_SM4C_ENCRYPTED_BUFFER_SIZE = 351;
const uint8_t H265_SM4C_ENCRYPTED_BUFFER[] = {
0x00, 0x00, 0x00, 0x01, 0x46, 0x01, 0x50, 0x00, 0x00, 0x01, 0x4e, 0x01, 0x05, 0xc8, 0x70, 0xc1,
0xdb, 0x9f, 0x66, 0xae, 0x41, 0x27, 0xbf, 0xc0, 0xbb, 0x19, 0x81, 0x69, 0x4b, 0x66, 0xf0, 0x55,
0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44, 0x5f, 0x30, 0x32, 0x55,
0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44, 0x5f, 0x30, 0x32, 0x10,
0x55, 0x44, 0x52, 0x4d, 0x5f, 0x54, 0x45, 0x53, 0x54, 0x5f, 0x43, 0x49, 0x44, 0x5f, 0x30, 0x32,
0xc0, 0x84, 0x33, 0x00, 0x00, 0x03, 0x00, 0x00, 0x82, 0x70, 0x73, 0x73, 0x68, 0x00, 0x00, 0x03,
0x00, 0x00, 0x3d, 0x5e, 0x6d, 0x35, 0x9b, 0x9a, 0x41, 0xe8, 0xb8, 0x43, 0xdd, 0x3c, 0x6e, 0x72,
0xc4, 0x2c, 0x00, 0x00, 0x03, 0x00, 0x62, 0x7b, 0x22, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e,
0x22, 0x3a, 0x22, 0x56, 0x31, 0x2e, 0x30, 0x22, 0x2c, 0x22, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e,
0x74, 0x49, 0x44, 0x22, 0x3a, 0x22, 0x61, 0x44, 0x49, 0x32, 0x4e, 0x53, 0x31, 0x7a, 0x62, 0x54,
0x52, 0x6a, 0x4c, 0x58, 0x52, 0x7a, 0x4c, 0x54, 0x41, 0x3d, 0x22, 0x2c, 0x22, 0x6b, 0x69, 0x64,
0x73, 0x22, 0x3a, 0x5b, 0x5d, 0x2c, 0x22, 0x65, 0x6e, 0x73, 0x63, 0x68, 0x65, 0x6d, 0x61, 0x22,
0x3a, 0x22, 0x73, 0x6d, 0x34, 0x63, 0x22, 0x2c, 0x22, 0x65, 0x78, 0x74, 0x73, 0x22, 0x3a, 0x22,
0x74, 0x73, 0x2f, 0x68, 0x32, 0x36, 0x35, 0x22, 0x7d, 0x80, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
0xe0, 0x24, 0xbe, 0x08, 0x32, 0x96, 0x00, 0x00, 0x03, 0x02, 0x85, 0xda, 0x3c, 0xd7, 0x82, 0x78,
0x1e, 0xb7, 0x2e, 0xad, 0x4c, 0x59, 0xb2, 0x2b, 0x72, 0x1f, 0xbe, 0xff, 0x1e, 0xbe, 0xf3, 0x1d,
0xab, 0x9c, 0x2a, 0x50, 0xfd, 0xdd, 0x3c, 0x00, 0xe9, 0xd6, 0xa3, 0x04, 0x0a, 0x7e, 0x9a, 0x02,
0x7f, 0x3e, 0x3c, 0x00, 0x2c, 0xf7, 0x86, 0xe4, 0x03, 0x5b, 0x6d, 0xe5, 0xfe, 0x5c, 0xe7, 0x88,
0x6d, 0x44, 0x67, 0x16, 0x38, 0x95, 0x02, 0x3a, 0x34, 0x31, 0x63, 0x0e, 0x9c, 0xe0, 0xe3, 0x70,
0xcb, 0xc3, 0x4c, 0x5d, 0x21, 0xaa, 0x9e, 0x0d, 0x45, 0x38, 0x56, 0xb9, 0xc9, 0x2b, 0xf7, 0xf5,
0xe1, 0xd9, 0x13, 0xa0, 0x46, 0xc3, 0xe8, 0x16, 0xe0, 0x78, 0x75, 0xe8, 0x69, 0x5d, 0xc1, 0xe3,
0x1d, 0xaa, 0x8c, 0xf4, 0x59, 0x75, 0xc2, 0xe1, 0x36, 0x98, 0x4f, 0x83, 0x2e, 0x87, 0x51,
}; // 351

// for AVS
const uint8_t AVS_SM4C_KEY_ID[] = { 0xd4, 0xb2, 0x01, 0xe4, 0x61, 0xc8, 0x98, 0x96,
                                    0xcf, 0x05, 0x22, 0x39, 0x8d, 0x09, 0xe6, 0x28 };
const uint8_t AVS_SM4C_IV[] = { 0xbf, 0x77, 0xed, 0x51, 0x81, 0xde, 0x36, 0x3e,
                                0x52, 0xf7, 0x20, 0x4f, 0x72, 0x14, 0xa3, 0x95};
uint32_t AVS_SM4C_ENCRYPTED_BUFFER_SIZE = 238;
const uint8_t AVS_SM4C_ENCRYPTED_BUFFER[] = {
0x00, 0x00, 0x01, 0xB5, 0xDF, 0x80, 0xD4, 0xB2, 0x01, 0xE4, 0x61, 0xC8, 0x98, 0x96, 0xCF, 0x05,
0x22, 0x39, 0x8D, 0x09, 0xE6, 0x28, 0x10, 0xBF, 0x77, 0xED, 0x51, 0x81, 0xDE, 0x36, 0x3E, 0x52,
0xF7, 0x20, 0x4F, 0x72, 0x14, 0xA3, 0x95, 0x00, 0x00, 0x01, 0xb6, 0xff, 0xff, 0xff, 0xff, 0xce,
0x14, 0x92, 0x10, 0x20, 0x82, 0xb5, 0xc0, 0x5e, 0x7b, 0xb4, 0x5a, 0x00, 0x00, 0x01, 0x00, 0xf6,
0x41, 0x55, 0xf7, 0x4f, 0xbb, 0x05, 0x15, 0xec, 0x11, 0xba, 0x98, 0x92, 0xdc, 0x21, 0x72, 0x75,
0x52, 0xfd, 0x9b, 0xff, 0x6c, 0x13, 0x75, 0x32, 0x11, 0x9a, 0x1c, 0xd4, 0xac, 0x58, 0x2c, 0x9a,
0x27, 0x4a, 0xab, 0x5c, 0x5b, 0x5e, 0xe1, 0xe8, 0xb9, 0x80, 0x9b, 0x0c, 0x63, 0xd8, 0xd1, 0xd8,
0xee, 0x8f, 0xce, 0x56, 0x14, 0xb2, 0xad, 0x30, 0x56, 0x7a, 0x22, 0x72, 0x85, 0x21, 0x1e, 0x47,
0x75, 0xd1, 0x30, 0xb1, 0x74, 0x5c, 0x0d, 0x75, 0xc4, 0x33, 0xcb, 0xfa, 0x14, 0x2d, 0x36, 0x32,
0x30, 0xd2, 0x2e, 0x9b, 0xc7, 0xd1, 0x5d, 0x3e, 0xd5, 0xaf, 0xed, 0x52, 0xbb, 0xa3, 0x73, 0xe7,
0x32, 0xfa, 0x4c, 0x5b, 0x4e, 0x73, 0xbd, 0x75, 0x94, 0x4a, 0x12, 0xfd, 0x73, 0x58, 0x95, 0x0c,
0x50, 0x6b, 0x91, 0xd7, 0x60, 0x52, 0xed, 0xb4, 0x33, 0xea, 0xee, 0x48, 0xbe, 0xde, 0x22, 0xe4,
0xa6, 0x48, 0xb8, 0xd5, 0xf7, 0x90, 0x1b, 0x2e, 0xbe, 0x69, 0xad, 0x86, 0xe9, 0x88, 0x23, 0x70,
0x31, 0x61, 0x5d, 0xe7, 0x60, 0x75, 0x27, 0x29, 0x5e, 0xf5, 0xe5, 0x86, 0x76, 0xb8, 0x42, 0x63,
0x42, 0x84, 0xbb, 0xb4, 0x48, 0x6c, 0x39, 0xc0, 0x40, 0x80, 0x00, 0x00, 0x01, 0x8f,
};

// for Audio
const uint8_t DRM_AUDIO_KEY_ID[] = {
    0xf3, 0xc5, 0xe0, 0x36, 0x1e, 0x66, 0x54, 0xb2,
    0x8f, 0x80, 0x49, 0xc7, 0x78, 0xb2, 0x39, 0x46};
const uint8_t DRM_AUDIO_IV[] = {
    0xa4, 0x63, 0x1a, 0x15, 0x3a, 0x44, 0x3d, 0xf9,
    0xee, 0xd0, 0x59, 0x30, 0x43, 0xdb, 0x75, 0x19};
uint32_t DRM_AUDIO_ENCRYPTED_BUFFER_SIZE = 65;
const uint8_t DRM_AUDIO_ENCRYPTED_BUFFER[] = {
    0x41, 0x55, 0xf7, 0x4f, 0xbb, 0x05, 0x15, 0xec, 0x11, 0xba, 0x98, 0x92, 0xdc, 0x21, 0x72, 0x75,
    0x52, 0xfd, 0x9b, 0xff, 0x6c, 0x13, 0x75, 0x32, 0x11, 0x9a, 0x1c, 0xd4, 0xac, 0x58, 0x2c, 0x9a,
    0x27, 0x4a, 0xab, 0x5c, 0x5b, 0x5e, 0xe1, 0xe8, 0xb9, 0x80, 0x9b, 0x0c, 0x63, 0xd8, 0xd1, 0xd8,
    0xee, 0x8f, 0xce, 0x56, 0x14, 0xb2, 0xad, 0x30, 0x56, 0x7a, 0x22, 0x72, 0x85, 0x21, 0x1e, 0x47,
    0x75}; // 65
}

namespace OHOS {
namespace Media {

void CodecDrmDecryptorUnitTest::SetUpTestCase(void)
{
}

void CodecDrmDecryptorUnitTest::TearDownTestCase(void)
{
}

void CodecDrmDecryptorUnitTest::SetUp(void)
{}

void CodecDrmDecryptorUnitTest::TearDown(void)
{}

void CreateH264MediaCencInfo(MetaDrmCencInfo &cencInfo, uint32_t method)
{
    UNITTEST_INFO_LOG("CreateH264MediaCencInfo begin");
    cencInfo.algo = MetaDrmCencAlgorithm::META_DRM_ALG_CENC_SM4_CBC;
    memcpy_s(cencInfo.keyId, sizeof(cencInfo.keyId), H264_KEY_ID, sizeof(cencInfo.keyId));
    cencInfo.keyIdLen = KEY_ID_LEN;
    memcpy_s(cencInfo.iv, sizeof(cencInfo.iv), H264_IV, sizeof(cencInfo.iv));
    cencInfo.ivLen = IV_LEN;
    cencInfo.encryptBlocks = 0;
    cencInfo.skipBlocks = 0;
    cencInfo.firstEncryptOffset = 0;
    cencInfo.subSampleNum = 1;
    cencInfo.mode = MetaDrmCencInfoMode::META_DRM_CENC_INFO_KEY_IV_SUBSAMPLES_NOT_SET;

    if (method == 0xff) {
        cencInfo.subSamples[0].clearHeaderLen = H264_CLEAR_BUFFER_SIZE;
    } else {
        cencInfo.subSamples[0].clearHeaderLen = H264_ENCRYPTED_BUFFER_SIZE;
    }
    cencInfo.subSamples[0].payLoadLen = 0;
    UNITTEST_INFO_LOG("CreateH264MediaCencInfo done");
}

void SetH264MediaData(std::shared_ptr<AVBuffer> drmInBuf, MetaDrmCencInfo &cencInfo, uint32_t method)
{
    UNITTEST_INFO_LOG("SetH264MediaData begin");
    int32_t drmRes = 0;
    switch (method) {
        case 0x1: // 0x1:SM4-SAMPL SM4S
            cencInfo.algo = MetaDrmCencAlgorithm::META_DRM_ALG_CENC_SM4_CBC;
            drmRes = memcpy_s(drmInBuf->memory_->GetAddr(), H264_ENCRYPTED_BUFFER_SIZE,
                H264_SM4S_ENCRYPTED_BUFFER, H264_ENCRYPTED_BUFFER_SIZE);
            break;
        case 0x2: // 0x2:AES CBCS
            cencInfo.algo = MetaDrmCencAlgorithm::META_DRM_ALG_CENC_AES_CBC;
            drmRes = memcpy_s(drmInBuf->memory_->GetAddr(), H264_ENCRYPTED_BUFFER_SIZE,
                H264_CBCS_ENCRYPTED_BUFFER, H264_ENCRYPTED_BUFFER_SIZE);
            break;
        case 0x5: // 0x5:AES CBC1
            cencInfo.algo = MetaDrmCencAlgorithm::META_DRM_ALG_CENC_AES_CBC;
            drmRes = memcpy_s(drmInBuf->memory_->GetAddr(), H264_ENCRYPTED_BUFFER_SIZE,
                H264_CBC1_ENCRYPTED_BUFFER, H264_ENCRYPTED_BUFFER_SIZE);
            break;
        case 0x3: // 0x3:SM4-CBC SM4C
            cencInfo.algo = MetaDrmCencAlgorithm::META_DRM_ALG_CENC_SM4_CBC;
            drmRes = memcpy_s(drmInBuf->memory_->GetAddr(), H264_ENCRYPTED_BUFFER_SIZE,
                H264_SM4C_ENCRYPTED_BUFFER, H264_ENCRYPTED_BUFFER_SIZE);
            break;
        case 0x0: // 0x0:NONE
            cencInfo.algo = MetaDrmCencAlgorithm::META_DRM_ALG_CENC_UNENCRYPTED;
            drmRes = memcpy_s(drmInBuf->memory_->GetAddr(), H264_ENCRYPTED_BUFFER_SIZE,
                H264_NONE_ENCRYPTED_BUFFER, H264_ENCRYPTED_BUFFER_SIZE);
            break;
        case 0xff: // 0xff:CLEAR
            cencInfo.algo = MetaDrmCencAlgorithm::META_DRM_ALG_CENC_UNENCRYPTED;
            drmRes = memcpy_s(drmInBuf->memory_->GetAddr(), H264_CLEAR_BUFFER_SIZE,
                H264_CLEAR_BUFFER, H264_CLEAR_BUFFER_SIZE);
            break;
        default:
            break;
    }
    EXPECT_EQ(drmRes, 0);
    UNITTEST_INFO_LOG("SetH264MediaData done");
}

void H264MediaCencDecrypt(std::shared_ptr<AVBuffer> drmInBuf, std::shared_ptr<AVBuffer> drmOutBuf,
    std::shared_ptr<MediaAVCodec::CodecDrmDecryptorMock> decryptorMock, uint32_t method, int32_t flag)
{
    UNITTEST_INFO_LOG("H264MediaCencDecrypt begin");
    MetaDrmCencInfo cencInfo;
    CreateH264MediaCencInfo(cencInfo, method);
    SetH264MediaData(drmInBuf, cencInfo, method);
    if (flag == 1) {
        std::vector<uint8_t> drmCencVec(reinterpret_cast<uint8_t *>(&cencInfo),
            (reinterpret_cast<uint8_t *>(&cencInfo)) + sizeof(MetaDrmCencInfo));
        drmInBuf->meta_->SetData(Media::Tag::DRM_CENC_INFO, std::move(drmCencVec));
    }
    if (method == 0xff) {
        decryptorMock->DrmVideoCencDecrypt(drmInBuf, drmOutBuf, H264_CLEAR_BUFFER_SIZE);
    } else {
        decryptorMock->DrmVideoCencDecrypt(drmInBuf, drmOutBuf, H264_ENCRYPTED_BUFFER_SIZE);
    }
    UNITTEST_INFO_LOG("H264MediaCencDecrypt done");
}

void CreateH265MediaCencInfo(MetaDrmCencInfo &cencInfo)
{
    UNITTEST_INFO_LOG("CreateH265MediaCencInfo begin");
    cencInfo.algo = MetaDrmCencAlgorithm::META_DRM_ALG_CENC_SM4_CBC;
    memcpy_s(cencInfo.keyId, sizeof(cencInfo.keyId), H265_SM4C_KEY_ID, sizeof(cencInfo.keyId));
    cencInfo.keyIdLen = KEY_ID_LEN;
    memcpy_s(cencInfo.iv, sizeof(cencInfo.iv), H265_SM4C_IV, sizeof(cencInfo.iv));
    cencInfo.ivLen = IV_LEN;
    cencInfo.encryptBlocks = 0;
    cencInfo.skipBlocks = 0;
    cencInfo.firstEncryptOffset = 0;
    cencInfo.subSampleNum = 1;
    cencInfo.mode = MetaDrmCencInfoMode::META_DRM_CENC_INFO_KEY_IV_SUBSAMPLES_NOT_SET;

    cencInfo.subSamples[0].clearHeaderLen = H265_SM4C_ENCRYPTED_BUFFER_SIZE;
    cencInfo.subSamples[0].payLoadLen = 0;
    UNITTEST_INFO_LOG("CreateH265MediaCencInfo done");
}

void CreateAvsMediaCencInfo(MetaDrmCencInfo &cencInfo)
{
    UNITTEST_INFO_LOG("CreateAvsMediaCencInfo begin");
    cencInfo.algo = MetaDrmCencAlgorithm::META_DRM_ALG_CENC_SM4_CBC;
    memcpy_s(cencInfo.keyId, sizeof(cencInfo.keyId), AVS_SM4C_KEY_ID, sizeof(cencInfo.keyId));
    cencInfo.keyIdLen = KEY_ID_LEN;
    memcpy_s(cencInfo.iv, sizeof(cencInfo.iv), AVS_SM4C_IV, sizeof(cencInfo.iv));
    cencInfo.ivLen = IV_LEN;
    cencInfo.encryptBlocks = 0;
    cencInfo.skipBlocks = 0;
    cencInfo.firstEncryptOffset = 0;
    cencInfo.subSampleNum = 1;
    cencInfo.mode = MetaDrmCencInfoMode::META_DRM_CENC_INFO_KEY_IV_SUBSAMPLES_NOT_SET;

    cencInfo.subSamples[0].clearHeaderLen = AVS_SM4C_ENCRYPTED_BUFFER_SIZE;
    cencInfo.subSamples[0].payLoadLen = 0;
    UNITTEST_INFO_LOG("CreateAvsMediaCencInfo done");
}

void CreateAudioCencInfo(MetaDrmCencInfo &cencInfo, MetaDrmCencAlgorithm algo)
{
    cencInfo.algo = algo;
    memcpy_s(cencInfo.keyId, sizeof(cencInfo.keyId), DRM_AUDIO_KEY_ID, sizeof(cencInfo.keyId));
    cencInfo.keyIdLen = KEY_ID_LEN;
    memcpy_s(cencInfo.iv, sizeof(cencInfo.iv), DRM_AUDIO_IV, sizeof(cencInfo.iv));
    cencInfo.ivLen = IV_LEN;
    cencInfo.encryptBlocks = 0;
    cencInfo.skipBlocks = 0;
    cencInfo.firstEncryptOffset = 0;
    cencInfo.subSampleNum = 0;
    cencInfo.mode = MetaDrmCencInfoMode::META_DRM_CENC_INFO_KEY_IV_SUBSAMPLES_SET;
}

void AudioCencDecrypt(std::shared_ptr<AVBuffer> drmInBuf, std::shared_ptr<AVBuffer> drmOutBuf,
    std::shared_ptr<MediaAVCodec::CodecDrmDecryptorMock> decryptorMock, MetaDrmCencAlgorithm algo, int32_t flag)
{
    MetaDrmCencInfo cencInfo;
    CreateAudioCencInfo(cencInfo, algo);

    int32_t drmRes = memcpy_s(drmInBuf->memory_->GetAddr(), DRM_AUDIO_ENCRYPTED_BUFFER_SIZE,
        DRM_AUDIO_ENCRYPTED_BUFFER, DRM_AUDIO_ENCRYPTED_BUFFER_SIZE);
    EXPECT_EQ(drmRes, 0);

    if (flag == 1) {
        std::vector<uint8_t> drmCencVec(reinterpret_cast<uint8_t *>(&cencInfo),
            (reinterpret_cast<uint8_t *>(&cencInfo)) + sizeof(MetaDrmCencInfo));
        drmInBuf->meta_->SetData(Media::Tag::DRM_CENC_INFO, std::move(drmCencVec));
    }
    decryptorMock->DrmAudioCencDecrypt(drmInBuf, drmOutBuf, DRM_AUDIO_ENCRYPTED_BUFFER_SIZE);
}

/**
 * @tc.name: Codec_Drm_Decryptor_SetCodecName_001
 * @tc.desc: set decoder name with H264
 */
HWTEST_F(CodecDrmDecryptorUnitTest, Codec_Drm_Decryptor_SetCodecName_001, TestSize.Level1)
{
    UNITTEST_INFO_LOG("Codec_Drm_Decryptor_SetCodecName_001");
    decryptorMock_ = std::make_shared<CodecDrmDecryptorMock>();
    EXPECT_NE(decryptorMock_, nullptr);
    decryptorMock_->SetCodecName("OH.Media.Codec.Decoder.Video.avc");
}

/**
 * @tc.name: Codec_Drm_Decryptor_SetDecryptionConfig_001
 * @tc.desc: SetDecryptionConfig with nullptr
 */
HWTEST_F(CodecDrmDecryptorUnitTest, Codec_Drm_Decryptor_SetDecryptionConfig_001, TestSize.Level1)
{
    UNITTEST_INFO_LOG("Codec_Drm_Decryptor_SetDecryptionConfig_001");
    decryptorMock_ = std::make_shared<CodecDrmDecryptorMock>();
    EXPECT_NE(decryptorMock_, nullptr);

    sptr<DrmStandard::IMediaKeySessionService> session = nullptr;
    bool svpFlag = false;
    decryptorMock_->SetDecryptionConfig(session, svpFlag);
    svpFlag = true;
    decryptorMock_->SetDecryptionConfig(session, svpFlag);
}

/**
 * @tc.name: Codec_Drm_Decryptor_DrmVideoCencDecrypt_001
 * @tc.desc: DrmVideoCencDecrypt, test for H264
 */
HWTEST_F(CodecDrmDecryptorUnitTest, Codec_Drm_Decryptor_DrmVideoCencDecrypt_001, TestSize.Level1)
{
    UNITTEST_INFO_LOG("Codec_Drm_Decryptor_DrmVideoCencDecrypt_001");
    decryptorMock_ = std::make_shared<CodecDrmDecryptorMock>();
    EXPECT_NE(decryptorMock_, nullptr);

    if (decryptorMock_ != nullptr) {
        decryptorMock_->SetCodecName("OH.Media.Codec.Decoder.Video.avc");
    }

    sptr<DrmStandard::IMediaKeySessionService> session = nullptr;
    bool svpFlag = false;
    decryptorMock_->SetDecryptionConfig(session, svpFlag);

    MemoryFlag memFlag = MEMORY_READ_WRITE;
    std::shared_ptr<AVAllocator> avAllocator = AVAllocatorFactory::CreateSharedAllocator(memFlag);
    EXPECT_NE(avAllocator, nullptr);
    std::shared_ptr<AVBuffer> drmInBuf = AVBuffer::CreateAVBuffer(avAllocator,
        static_cast<int32_t>(H264_ENCRYPTED_BUFFER_SIZE));
    EXPECT_NE(drmInBuf, nullptr);
    EXPECT_NE(drmInBuf->memory_, nullptr);
    EXPECT_EQ(drmInBuf->memory_->GetCapacity(), H264_ENCRYPTED_BUFFER_SIZE);

    std::shared_ptr<AVBuffer> drmOutBuf = AVBuffer::CreateAVBuffer(avAllocator,
        static_cast<int32_t>(H264_ENCRYPTED_BUFFER_SIZE));
    EXPECT_NE(drmOutBuf, nullptr);
    EXPECT_NE(drmOutBuf->memory_, nullptr);
    EXPECT_EQ(drmOutBuf->memory_->GetCapacity(), H264_ENCRYPTED_BUFFER_SIZE);

    H264MediaCencDecrypt(drmInBuf, drmOutBuf, decryptorMock_, 0x0, 0);
    H264MediaCencDecrypt(drmInBuf, drmOutBuf, decryptorMock_, 0x1, 1); // 0x1:SM4-SAMPL SM4S
    H264MediaCencDecrypt(drmInBuf, drmOutBuf, decryptorMock_, 0x2, 1); // 0x2:AES CBCS
    H264MediaCencDecrypt(drmInBuf, drmOutBuf, decryptorMock_, 0x5, 1); // 0x5:AES CBC1
    H264MediaCencDecrypt(drmInBuf, drmOutBuf, decryptorMock_, 0x3, 1); // 0x3:SM4-CBC SM4C
    H264MediaCencDecrypt(drmInBuf, drmOutBuf, decryptorMock_, 0x0, 1); // 0x0:NONE
    H264MediaCencDecrypt(drmInBuf, drmOutBuf, decryptorMock_, 0xff, 1); // 0xff:CLEAR
}

/**
 * @tc.name: Codec_Drm_Decryptor_DrmVideoCencDecrypt_002
 * @tc.desc: DrmVideoCencDecrypt, test for H265
 */
HWTEST_F(CodecDrmDecryptorUnitTest, Codec_Drm_Decryptor_DrmVideoCencDecrypt_002, TestSize.Level1)
{
    UNITTEST_INFO_LOG("Codec_Drm_Decryptor_DrmVideoCencDecrypt_002");
    decryptorMock_ = std::make_shared<CodecDrmDecryptorMock>();
    EXPECT_NE(decryptorMock_, nullptr);

    if (decryptorMock_ != nullptr) {
        decryptorMock_->SetCodecName("OH.Media.Codec.Decoder.Video.hevc");
    }

    sptr<DrmStandard::IMediaKeySessionService> session = nullptr;
    bool svpFlag = false;
    decryptorMock_->SetDecryptionConfig(session, svpFlag);

    MemoryFlag memFlag = MEMORY_READ_WRITE;
    std::shared_ptr<AVAllocator> avAllocator = AVAllocatorFactory::CreateSharedAllocator(memFlag);
    EXPECT_NE(avAllocator, nullptr);
    std::shared_ptr<AVBuffer> drmInBuf = AVBuffer::CreateAVBuffer(avAllocator,
        static_cast<int32_t>(H265_SM4C_ENCRYPTED_BUFFER_SIZE));
    EXPECT_NE(drmInBuf, nullptr);
    EXPECT_NE(drmInBuf->memory_, nullptr);
    EXPECT_EQ(drmInBuf->memory_->GetCapacity(), H265_SM4C_ENCRYPTED_BUFFER_SIZE);
    int32_t drmRes = memcpy_s(drmInBuf->memory_->GetAddr(), H265_SM4C_ENCRYPTED_BUFFER_SIZE,
        H265_SM4C_ENCRYPTED_BUFFER, H265_SM4C_ENCRYPTED_BUFFER_SIZE);
    EXPECT_EQ(drmRes, 0);

    std::shared_ptr<AVBuffer> drmOutBuf = AVBuffer::CreateAVBuffer(avAllocator,
        static_cast<int32_t>(H265_SM4C_ENCRYPTED_BUFFER_SIZE));
    EXPECT_NE(drmOutBuf, nullptr);
    EXPECT_NE(drmOutBuf->memory_, nullptr);
    EXPECT_EQ(drmOutBuf->memory_->GetCapacity(), H265_SM4C_ENCRYPTED_BUFFER_SIZE);

    MetaDrmCencInfo cencInfo;
    CreateH265MediaCencInfo(cencInfo);
    std::vector<uint8_t> drmCencVec(reinterpret_cast<uint8_t *>(&cencInfo),
        (reinterpret_cast<uint8_t *>(&cencInfo)) + sizeof(MetaDrmCencInfo));
    drmInBuf->meta_->SetData(Media::Tag::DRM_CENC_INFO, std::move(drmCencVec));
    decryptorMock_->DrmVideoCencDecrypt(drmInBuf, drmOutBuf, H265_SM4C_ENCRYPTED_BUFFER_SIZE);
}

/**
 * @tc.name: Codec_Drm_Decryptor_DrmVideoCencDecrypt_003
 * @tc.desc: DrmVideoCencDecrypt, test for AVS
 */
HWTEST_F(CodecDrmDecryptorUnitTest, Codec_Drm_Decryptor_DrmVideoCencDecrypt_003, TestSize.Level1)
{
    UNITTEST_INFO_LOG("Codec_Drm_Decryptor_DrmVideoCencDecrypt_003");
    decryptorMock_ = std::make_shared<CodecDrmDecryptorMock>();
    EXPECT_NE(decryptorMock_, nullptr);

    if (decryptorMock_ != nullptr) {
        decryptorMock_->SetCodecName("OH.Media.Codec.Decoder.Video.avs");
    }

    sptr<DrmStandard::IMediaKeySessionService> session = nullptr;
    bool svpFlag = false;
    decryptorMock_->SetDecryptionConfig(session, svpFlag);

    MemoryFlag memFlag = MEMORY_READ_WRITE;
    std::shared_ptr<AVAllocator> avAllocator = AVAllocatorFactory::CreateSharedAllocator(memFlag);
    EXPECT_NE(avAllocator, nullptr);
    std::shared_ptr<AVBuffer> drmInBuf = AVBuffer::CreateAVBuffer(avAllocator,
        static_cast<int32_t>(AVS_SM4C_ENCRYPTED_BUFFER_SIZE));
    EXPECT_NE(drmInBuf, nullptr);
    EXPECT_NE(drmInBuf->memory_, nullptr);
    EXPECT_EQ(drmInBuf->memory_->GetCapacity(), AVS_SM4C_ENCRYPTED_BUFFER_SIZE);
    int32_t drmRes = memcpy_s(drmInBuf->memory_->GetAddr(), AVS_SM4C_ENCRYPTED_BUFFER_SIZE,
        AVS_SM4C_ENCRYPTED_BUFFER, AVS_SM4C_ENCRYPTED_BUFFER_SIZE);
    EXPECT_EQ(drmRes, 0);

    std::shared_ptr<AVBuffer> drmOutBuf = AVBuffer::CreateAVBuffer(avAllocator,
        static_cast<int32_t>(AVS_SM4C_ENCRYPTED_BUFFER_SIZE));
    EXPECT_NE(drmOutBuf, nullptr);
    EXPECT_NE(drmOutBuf->memory_, nullptr);
    EXPECT_EQ(drmOutBuf->memory_->GetCapacity(), AVS_SM4C_ENCRYPTED_BUFFER_SIZE);

    MetaDrmCencInfo cencInfo;
    CreateAvsMediaCencInfo(cencInfo);
    std::vector<uint8_t> drmCencVec(reinterpret_cast<uint8_t *>(&cencInfo),
        (reinterpret_cast<uint8_t *>(&cencInfo)) + sizeof(MetaDrmCencInfo));
    drmInBuf->meta_->SetData(Media::Tag::DRM_CENC_INFO, std::move(drmCencVec));
    decryptorMock_->DrmVideoCencDecrypt(drmInBuf, drmOutBuf, AVS_SM4C_ENCRYPTED_BUFFER_SIZE);
}

/**
 * @tc.name: Codec_Drm_Decryptor_DrmAudioCencDecrypt_001
 * @tc.desc: DrmAudioCencDecrypt, test for H264
 */
HWTEST_F(CodecDrmDecryptorUnitTest, Codec_Drm_Decryptor_DrmAudioCencDecrypt_001, TestSize.Level1)
{
    UNITTEST_INFO_LOG("Codec_Drm_Decryptor_DrmAudioCencDecrypt_001");
    decryptorMock_ = std::make_shared<CodecDrmDecryptorMock>();
    EXPECT_NE(decryptorMock_, nullptr);

    if (decryptorMock_ != nullptr) {
        decryptorMock_->SetCodecName("OH.Media.Codec.Decoder.Audio.aac");
    }

    sptr<DrmStandard::IMediaKeySessionService> session = nullptr;
    bool svpFlag = false;
    decryptorMock_->SetDecryptionConfig(session, svpFlag);

    MemoryFlag memFlag = MEMORY_READ_WRITE;
    std::shared_ptr<AVAllocator> avAllocator = AVAllocatorFactory::CreateSharedAllocator(memFlag);
    EXPECT_NE(avAllocator, nullptr);
    std::shared_ptr<AVBuffer> drmInBuf = AVBuffer::CreateAVBuffer(avAllocator,
        static_cast<int32_t>(DRM_AUDIO_ENCRYPTED_BUFFER_SIZE));
    EXPECT_NE(drmInBuf, nullptr);
    EXPECT_NE(drmInBuf->memory_, nullptr);
    EXPECT_EQ(drmInBuf->memory_->GetCapacity(), DRM_AUDIO_ENCRYPTED_BUFFER_SIZE);

    std::shared_ptr<AVBuffer> drmOutBuf = AVBuffer::CreateAVBuffer(avAllocator,
        static_cast<int32_t>(DRM_AUDIO_ENCRYPTED_BUFFER_SIZE));
    EXPECT_NE(drmOutBuf, nullptr);
    EXPECT_NE(drmOutBuf->memory_, nullptr);
    EXPECT_EQ(drmOutBuf->memory_->GetCapacity(), DRM_AUDIO_ENCRYPTED_BUFFER_SIZE);

    AudioCencDecrypt(drmInBuf, drmOutBuf, decryptorMock_, MetaDrmCencAlgorithm::META_DRM_ALG_CENC_UNENCRYPTED, 0);
    AudioCencDecrypt(drmInBuf, drmOutBuf, decryptorMock_, MetaDrmCencAlgorithm::META_DRM_ALG_CENC_AES_CTR, 1);
    AudioCencDecrypt(drmInBuf, drmOutBuf, decryptorMock_, MetaDrmCencAlgorithm::META_DRM_ALG_CENC_SM4_CTR, 1);
    AudioCencDecrypt(drmInBuf, drmOutBuf, decryptorMock_, MetaDrmCencAlgorithm::META_DRM_ALG_CENC_AES_CBC, 1);
    AudioCencDecrypt(drmInBuf, drmOutBuf, decryptorMock_, MetaDrmCencAlgorithm::META_DRM_ALG_CENC_SM4_CBC, 1);
    AudioCencDecrypt(drmInBuf, drmOutBuf, decryptorMock_, MetaDrmCencAlgorithm::META_DRM_ALG_CENC_UNENCRYPTED, 1);
}
} // namespace Media
} // namespace OHOS