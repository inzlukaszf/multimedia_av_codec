/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef LAYER_INFO_ONE_I_HEVC_H
#define LAYER_INFO_ONE_I_HEVC_H

#include <iostream>

auto GopInfoOneIHevc = R"([
	{"gopId": 0, "gopSize": 372, "startFrameId": 0}
])"_json;

auto FrameLayerInfoOneIHevc = R"([
    {"frameId": 0, "dts": -43666, "layer": 2, "discardable": false},
	{"frameId": 1, "dts": -10333, "layer": 1, "discardable": false},
	{"frameId": 2, "dts": 23000, "layer": 1, "discardable": false},
	{"frameId": 3, "dts": 56333, "layer": 0, "discardable": true},
	{"frameId": 4, "dts": 89666, "layer": 0, "discardable": true},
	{"frameId": 5, "dts": 123000, "layer": 1, "discardable": false},
	{"frameId": 6, "dts": 156333, "layer": 0, "discardable": true},
	{"frameId": 7, "dts": 189666, "layer": 1, "discardable": false},
	{"frameId": 8, "dts": 223000, "layer": 1, "discardable": false},
	{"frameId": 9, "dts": 256333, "layer": 0, "discardable": true},
	{"frameId": 10, "dts": 289666, "layer": 1, "discardable": false},
	{"frameId": 11, "dts": 323000, "layer": 1, "discardable": false},
	{"frameId": 12, "dts": 356333, "layer": 1, "discardable": false},
	{"frameId": 13, "dts": 389666, "layer": 1, "discardable": false},
	{"frameId": 14, "dts": 423000, "layer": 1, "discardable": false},
	{"frameId": 15, "dts": 456333, "layer": 1, "discardable": false},
	{"frameId": 16, "dts": 489666, "layer": 1, "discardable": false},
	{"frameId": 17, "dts": 523000, "layer": 0, "discardable": true},
	{"frameId": 18, "dts": 556333, "layer": 0, "discardable": true},
	{"frameId": 19, "dts": 589666, "layer": 0, "discardable": true},
	{"frameId": 20, "dts": 623000, "layer": 1, "discardable": false},
	{"frameId": 21, "dts": 656333, "layer": 1, "discardable": false},
	{"frameId": 22, "dts": 689666, "layer": 1, "discardable": false},
	{"frameId": 23, "dts": 723000, "layer": 0, "discardable": true},
	{"frameId": 24, "dts": 756333, "layer": 0, "discardable": true},
	{"frameId": 25, "dts": 789666, "layer": 1, "discardable": false},
	{"frameId": 26, "dts": 823000, "layer": 1, "discardable": false},
	{"frameId": 27, "dts": 856333, "layer": 1, "discardable": false},
	{"frameId": 28, "dts": 889666, "layer": 0, "discardable": true},
	{"frameId": 29, "dts": 923000, "layer": 0, "discardable": true},
	{"frameId": 30, "dts": 956333, "layer": 1, "discardable": false},
	{"frameId": 31, "dts": 989666, "layer": 1, "discardable": false},
	{"frameId": 32, "dts": 1023000, "layer": 0, "discardable": true},
	{"frameId": 33, "dts": 1056333, "layer": 0, "discardable": true},
	{"frameId": 34, "dts": 1089666, "layer": 1, "discardable": false},
	{"frameId": 35, "dts": 1123000, "layer": 1, "discardable": false},
	{"frameId": 36, "dts": 1156333, "layer": 0, "discardable": true},
	{"frameId": 37, "dts": 1189666, "layer": 1, "discardable": false},
	{"frameId": 38, "dts": 1223000, "layer": 1, "discardable": false},
	{"frameId": 39, "dts": 1256333, "layer": 0, "discardable": true},
	{"frameId": 40, "dts": 1289666, "layer": 0, "discardable": true},
	{"frameId": 41, "dts": 1323000, "layer": 1, "discardable": false},
	{"frameId": 42, "dts": 1356333, "layer": 1, "discardable": false},
	{"frameId": 43, "dts": 1389666, "layer": 1, "discardable": false},
	{"frameId": 44, "dts": 1423000, "layer": 0, "discardable": true},
	{"frameId": 45, "dts": 1456333, "layer": 0, "discardable": true},
	{"frameId": 46, "dts": 1489666, "layer": 1, "discardable": false},
	{"frameId": 47, "dts": 1523000, "layer": 1, "discardable": false},
	{"frameId": 48, "dts": 1556333, "layer": 1, "discardable": false},
	{"frameId": 49, "dts": 1589666, "layer": 1, "discardable": false},
	{"frameId": 50, "dts": 1623000, "layer": 0, "discardable": true},
	{"frameId": 51, "dts": 1656333, "layer": 0, "discardable": true},
	{"frameId": 52, "dts": 1689666, "layer": 1, "discardable": false},
	{"frameId": 53, "dts": 1723000, "layer": 1, "discardable": false},
	{"frameId": 54, "dts": 1756333, "layer": 1, "discardable": false},
	{"frameId": 55, "dts": 1789666, "layer": 0, "discardable": true},
	{"frameId": 56, "dts": 1823000, "layer": 0, "discardable": true},
	{"frameId": 57, "dts": 1856333, "layer": 1, "discardable": false},
	{"frameId": 58, "dts": 1889666, "layer": 1, "discardable": false},
	{"frameId": 59, "dts": 1923000, "layer": 1, "discardable": false},
	{"frameId": 60, "dts": 1956333, "layer": 0, "discardable": true},
	{"frameId": 61, "dts": 1989666, "layer": 0, "discardable": true},
	{"frameId": 62, "dts": 2023000, "layer": 1, "discardable": false},
	{"frameId": 63, "dts": 2056333, "layer": 1, "discardable": false},
	{"frameId": 64, "dts": 2089666, "layer": 0, "discardable": true},
	{"frameId": 65, "dts": 2123000, "layer": 0, "discardable": true},
	{"frameId": 66, "dts": 2156333, "layer": 1, "discardable": false},
	{"frameId": 67, "dts": 2189666, "layer": 1, "discardable": false},
	{"frameId": 68, "dts": 2223000, "layer": 0, "discardable": true},
	{"frameId": 69, "dts": 2256333, "layer": 1, "discardable": false},
	{"frameId": 70, "dts": 2289666, "layer": 1, "discardable": false},
	{"frameId": 71, "dts": 2323000, "layer": 0, "discardable": true},
	{"frameId": 72, "dts": 2356333, "layer": 0, "discardable": true},
	{"frameId": 73, "dts": 2389666, "layer": 1, "discardable": false},
	{"frameId": 74, "dts": 2423000, "layer": 1, "discardable": false},
	{"frameId": 75, "dts": 2456333, "layer": 0, "discardable": true},
	{"frameId": 76, "dts": 2489666, "layer": 0, "discardable": true},
	{"frameId": 77, "dts": 2523000, "layer": 0, "discardable": true},
	{"frameId": 78, "dts": 2556333, "layer": 1, "discardable": false},
	{"frameId": 79, "dts": 2589666, "layer": 1, "discardable": false},
	{"frameId": 80, "dts": 2623000, "layer": 0, "discardable": true},
	{"frameId": 81, "dts": 2656333, "layer": 0, "discardable": true},
	{"frameId": 82, "dts": 2689666, "layer": 1, "discardable": false},
	{"frameId": 83, "dts": 2723000, "layer": 1, "discardable": false},
	{"frameId": 84, "dts": 2756333, "layer": 1, "discardable": false},
	{"frameId": 85, "dts": 2789666, "layer": 0, "discardable": true},
	{"frameId": 86, "dts": 2823000, "layer": 1, "discardable": false},
	{"frameId": 87, "dts": 2856333, "layer": 1, "discardable": false},
	{"frameId": 88, "dts": 2889666, "layer": 1, "discardable": false},
	{"frameId": 89, "dts": 2923000, "layer": 1, "discardable": false},
	{"frameId": 90, "dts": 2956333, "layer": 0, "discardable": true},
	{"frameId": 91, "dts": 2989666, "layer": 0, "discardable": true},
	{"frameId": 92, "dts": 3023000, "layer": 0, "discardable": true},
	{"frameId": 93, "dts": 3056333, "layer": 1, "discardable": false},
	{"frameId": 94, "dts": 3089666, "layer": 1, "discardable": false},
	{"frameId": 95, "dts": 3123000, "layer": 1, "discardable": false},
	{"frameId": 96, "dts": 3156333, "layer": 0, "discardable": true},
	{"frameId": 97, "dts": 3189666, "layer": 0, "discardable": true},
	{"frameId": 98, "dts": 3223000, "layer": 0, "discardable": true},
	{"frameId": 99, "dts": 3256333, "layer": 1, "discardable": false},
	{"frameId": 100, "dts": 3289666, "layer": 1, "discardable": false},
	{"frameId": 101, "dts": 3323000, "layer": 0, "discardable": true},
	{"frameId": 102, "dts": 3356333, "layer": 0, "discardable": true},
	{"frameId": 103, "dts": 3389666, "layer": 1, "discardable": false},
	{"frameId": 104, "dts": 3423000, "layer": 1, "discardable": false},
	{"frameId": 105, "dts": 3456333, "layer": 0, "discardable": true},
	{"frameId": 106, "dts": 3489666, "layer": 0, "discardable": true},
	{"frameId": 107, "dts": 3523000, "layer": 0, "discardable": true},
	{"frameId": 108, "dts": 3556333, "layer": 1, "discardable": false},
	{"frameId": 109, "dts": 3589666, "layer": 1, "discardable": false},
	{"frameId": 110, "dts": 3623000, "layer": 1, "discardable": false},
	{"frameId": 111, "dts": 3656333, "layer": 0, "discardable": true},
	{"frameId": 112, "dts": 3689666, "layer": 0, "discardable": true},
	{"frameId": 113, "dts": 3723000, "layer": 1, "discardable": false},
	{"frameId": 114, "dts": 3756333, "layer": 0, "discardable": true},
	{"frameId": 115, "dts": 3789666, "layer": 1, "discardable": false},
	{"frameId": 116, "dts": 3823000, "layer": 1, "discardable": false},
	{"frameId": 117, "dts": 3856333, "layer": 0, "discardable": true},
	{"frameId": 118, "dts": 3889666, "layer": 0, "discardable": true},
	{"frameId": 119, "dts": 3923000, "layer": 1, "discardable": false},
	{"frameId": 120, "dts": 3956333, "layer": 0, "discardable": true},
	{"frameId": 121, "dts": 3989666, "layer": 1, "discardable": false},
	{"frameId": 122, "dts": 4023000, "layer": 1, "discardable": false},
	{"frameId": 123, "dts": 4056333, "layer": 1, "discardable": false},
	{"frameId": 124, "dts": 4089666, "layer": 0, "discardable": true},
	{"frameId": 125, "dts": 4123000, "layer": 0, "discardable": true},
	{"frameId": 126, "dts": 4156333, "layer": 0, "discardable": true},
	{"frameId": 127, "dts": 4189666, "layer": 1, "discardable": false},
	{"frameId": 128, "dts": 4223000, "layer": 0, "discardable": true},
	{"frameId": 129, "dts": 4256333, "layer": 1, "discardable": false},
	{"frameId": 130, "dts": 4289666, "layer": 0, "discardable": true},
	{"frameId": 131, "dts": 4323000, "layer": 1, "discardable": false},
	{"frameId": 132, "dts": 4356333, "layer": 1, "discardable": false},
	{"frameId": 133, "dts": 4389666, "layer": 0, "discardable": true},
	{"frameId": 134, "dts": 4423000, "layer": 1, "discardable": false},
	{"frameId": 135, "dts": 4456333, "layer": 1, "discardable": false},
	{"frameId": 136, "dts": 4489666, "layer": 1, "discardable": false},
	{"frameId": 137, "dts": 4523000, "layer": 1, "discardable": false},
	{"frameId": 138, "dts": 4556333, "layer": 1, "discardable": false},
	{"frameId": 139, "dts": 4589666, "layer": 0, "discardable": true},
	{"frameId": 140, "dts": 4623000, "layer": 0, "discardable": true},
	{"frameId": 141, "dts": 4656333, "layer": 0, "discardable": true},
	{"frameId": 142, "dts": 4689666, "layer": 1, "discardable": false},
	{"frameId": 143, "dts": 4723000, "layer": 1, "discardable": false},
	{"frameId": 144, "dts": 4756333, "layer": 0, "discardable": true},
	{"frameId": 145, "dts": 4789666, "layer": 0, "discardable": true},
	{"frameId": 146, "dts": 4823000, "layer": 0, "discardable": true},
	{"frameId": 147, "dts": 4856333, "layer": 1, "discardable": false},
	{"frameId": 148, "dts": 4889666, "layer": 1, "discardable": false},
	{"frameId": 149, "dts": 4923000, "layer": 0, "discardable": true},
	{"frameId": 150, "dts": 4956333, "layer": 0, "discardable": true},
	{"frameId": 151, "dts": 4989666, "layer": 1, "discardable": false},
	{"frameId": 152, "dts": 5023000, "layer": 1, "discardable": false},
	{"frameId": 153, "dts": 5056333, "layer": 0, "discardable": true},
	{"frameId": 154, "dts": 5089666, "layer": 1, "discardable": false},
	{"frameId": 155, "dts": 5123000, "layer": 1, "discardable": false},
	{"frameId": 156, "dts": 5156333, "layer": 0, "discardable": true},
	{"frameId": 157, "dts": 5189666, "layer": 0, "discardable": true},
	{"frameId": 158, "dts": 5223000, "layer": 1, "discardable": false},
	{"frameId": 159, "dts": 5256333, "layer": 1, "discardable": false},
	{"frameId": 160, "dts": 5289666, "layer": 0, "discardable": true},
	{"frameId": 161, "dts": 5323000, "layer": 0, "discardable": true},
	{"frameId": 162, "dts": 5356333, "layer": 1, "discardable": false},
	{"frameId": 163, "dts": 5389666, "layer": 1, "discardable": false},
	{"frameId": 164, "dts": 5423000, "layer": 1, "discardable": false},
	{"frameId": 165, "dts": 5456333, "layer": 0, "discardable": true},
	{"frameId": 166, "dts": 5489666, "layer": 0, "discardable": true},
	{"frameId": 167, "dts": 5523000, "layer": 1, "discardable": false},
	{"frameId": 168, "dts": 5556333, "layer": 1, "discardable": false},
	{"frameId": 169, "dts": 5589666, "layer": 0, "discardable": true},
	{"frameId": 170, "dts": 5623000, "layer": 0, "discardable": true},
	{"frameId": 171, "dts": 5656333, "layer": 1, "discardable": false},
	{"frameId": 172, "dts": 5689666, "layer": 1, "discardable": false},
	{"frameId": 173, "dts": 5723000, "layer": 1, "discardable": false},
	{"frameId": 174, "dts": 5756333, "layer": 1, "discardable": false},
	{"frameId": 175, "dts": 5789666, "layer": 0, "discardable": true},
	{"frameId": 176, "dts": 5823000, "layer": 0, "discardable": true},
	{"frameId": 177, "dts": 5856333, "layer": 0, "discardable": true},
	{"frameId": 178, "dts": 5889666, "layer": 1, "discardable": false},
	{"frameId": 179, "dts": 5923000, "layer": 1, "discardable": false},
	{"frameId": 180, "dts": 5956333, "layer": 1, "discardable": false},
	{"frameId": 181, "dts": 5989666, "layer": 1, "discardable": false},
	{"frameId": 182, "dts": 6023000, "layer": 1, "discardable": false},
	{"frameId": 183, "dts": 6056333, "layer": 0, "discardable": true},
	{"frameId": 184, "dts": 6089666, "layer": 1, "discardable": false},
	{"frameId": 185, "dts": 6123000, "layer": 1, "discardable": false},
	{"frameId": 186, "dts": 6156333, "layer": 1, "discardable": false},
	{"frameId": 187, "dts": 6189666, "layer": 1, "discardable": false},
	{"frameId": 188, "dts": 6223000, "layer": 0, "discardable": true},
	{"frameId": 189, "dts": 6256333, "layer": 0, "discardable": true},
	{"frameId": 190, "dts": 6289666, "layer": 1, "discardable": false},
	{"frameId": 191, "dts": 6323000, "layer": 1, "discardable": false},
	{"frameId": 192, "dts": 6356333, "layer": 0, "discardable": true},
	{"frameId": 193, "dts": 6389666, "layer": 0, "discardable": true},
	{"frameId": 194, "dts": 6423000, "layer": 1, "discardable": false},
	{"frameId": 195, "dts": 6456333, "layer": 1, "discardable": false},
	{"frameId": 196, "dts": 6489666, "layer": 0, "discardable": true},
	{"frameId": 197, "dts": 6523000, "layer": 0, "discardable": true},
	{"frameId": 198, "dts": 6556333, "layer": 1, "discardable": false},
	{"frameId": 199, "dts": 6589666, "layer": 1, "discardable": false},
	{"frameId": 200, "dts": 6623000, "layer": 0, "discardable": true},
	{"frameId": 201, "dts": 6656333, "layer": 1, "discardable": false},
	{"frameId": 202, "dts": 6689666, "layer": 1, "discardable": false},
	{"frameId": 203, "dts": 6723000, "layer": 1, "discardable": false},
	{"frameId": 204, "dts": 6756333, "layer": 0, "discardable": true},
	{"frameId": 205, "dts": 6789666, "layer": 0, "discardable": true},
	{"frameId": 206, "dts": 6823000, "layer": 1, "discardable": false},
	{"frameId": 207, "dts": 6856333, "layer": 1, "discardable": false},
	{"frameId": 208, "dts": 6889666, "layer": 1, "discardable": false},
	{"frameId": 209, "dts": 6923000, "layer": 0, "discardable": true},
	{"frameId": 210, "dts": 6956333, "layer": 1, "discardable": false},
	{"frameId": 211, "dts": 6989666, "layer": 1, "discardable": false},
	{"frameId": 212, "dts": 7023000, "layer": 1, "discardable": false},
	{"frameId": 213, "dts": 7056333, "layer": 0, "discardable": true},
	{"frameId": 214, "dts": 7089666, "layer": 1, "discardable": false},
	{"frameId": 215, "dts": 7123000, "layer": 1, "discardable": false},
	{"frameId": 216, "dts": 7156333, "layer": 1, "discardable": false},
	{"frameId": 217, "dts": 7189666, "layer": 0, "discardable": true},
	{"frameId": 218, "dts": 7223000, "layer": 0, "discardable": true},
	{"frameId": 219, "dts": 7256333, "layer": 1, "discardable": false},
	{"frameId": 220, "dts": 7289666, "layer": 1, "discardable": false},
	{"frameId": 221, "dts": 7323000, "layer": 0, "discardable": true},
	{"frameId": 222, "dts": 7356333, "layer": 0, "discardable": true},
	{"frameId": 223, "dts": 7389666, "layer": 1, "discardable": false},
	{"frameId": 224, "dts": 7423000, "layer": 1, "discardable": false},
	{"frameId": 225, "dts": 7456333, "layer": 0, "discardable": true},
	{"frameId": 226, "dts": 7489666, "layer": 0, "discardable": true},
	{"frameId": 227, "dts": 7523000, "layer": 1, "discardable": false},
	{"frameId": 228, "dts": 7556333, "layer": 1, "discardable": false},
	{"frameId": 229, "dts": 7589666, "layer": 0, "discardable": true},
	{"frameId": 230, "dts": 7623000, "layer": 0, "discardable": true},
	{"frameId": 231, "dts": 7656333, "layer": 1, "discardable": false},
	{"frameId": 232, "dts": 7689666, "layer": 1, "discardable": false},
	{"frameId": 233, "dts": 7723000, "layer": 1, "discardable": false},
	{"frameId": 234, "dts": 7756333, "layer": 1, "discardable": false},
	{"frameId": 235, "dts": 7789666, "layer": 1, "discardable": false},
	{"frameId": 236, "dts": 7823000, "layer": 1, "discardable": false},
	{"frameId": 237, "dts": 7856333, "layer": 0, "discardable": true},
	{"frameId": 238, "dts": 7889666, "layer": 0, "discardable": true},
	{"frameId": 239, "dts": 7923000, "layer": 1, "discardable": false},
	{"frameId": 240, "dts": 7956333, "layer": 1, "discardable": false},
	{"frameId": 241, "dts": 7989666, "layer": 0, "discardable": true},
	{"frameId": 242, "dts": 8023000, "layer": 0, "discardable": true},
	{"frameId": 243, "dts": 8056333, "layer": 1, "discardable": false},
	{"frameId": 244, "dts": 8089666, "layer": 0, "discardable": true},
	{"frameId": 245, "dts": 8123000, "layer": 1, "discardable": false},
	{"frameId": 246, "dts": 8156333, "layer": 1, "discardable": false},
	{"frameId": 247, "dts": 8189666, "layer": 1, "discardable": false},
	{"frameId": 248, "dts": 8223000, "layer": 0, "discardable": true},
	{"frameId": 249, "dts": 8256333, "layer": 0, "discardable": true},
	{"frameId": 250, "dts": 8289666, "layer": 1, "discardable": false},
	{"frameId": 251, "dts": 8323000, "layer": 0, "discardable": true},
	{"frameId": 252, "dts": 8356333, "layer": 1, "discardable": false},
	{"frameId": 253, "dts": 8389666, "layer": 0, "discardable": true},
	{"frameId": 254, "dts": 8423000, "layer": 1, "discardable": false},
	{"frameId": 255, "dts": 8456333, "layer": 1, "discardable": false},
	{"frameId": 256, "dts": 8489666, "layer": 1, "discardable": false},
	{"frameId": 257, "dts": 8523000, "layer": 0, "discardable": true},
	{"frameId": 258, "dts": 8556333, "layer": 0, "discardable": true},
	{"frameId": 259, "dts": 8589666, "layer": 1, "discardable": false},
	{"frameId": 260, "dts": 8623000, "layer": 1, "discardable": false},
	{"frameId": 261, "dts": 8656333, "layer": 1, "discardable": false},
	{"frameId": 262, "dts": 8689666, "layer": 1, "discardable": false},
	{"frameId": 263, "dts": 8723000, "layer": 0, "discardable": true},
	{"frameId": 264, "dts": 8756333, "layer": 0, "discardable": true},
	{"frameId": 265, "dts": 8789666, "layer": 1, "discardable": false},
	{"frameId": 266, "dts": 8823000, "layer": 1, "discardable": false},
	{"frameId": 267, "dts": 8856333, "layer": 0, "discardable": true},
	{"frameId": 268, "dts": 8889666, "layer": 0, "discardable": true},
	{"frameId": 269, "dts": 8923000, "layer": 0, "discardable": true},
	{"frameId": 270, "dts": 8956333, "layer": 1, "discardable": false},
	{"frameId": 271, "dts": 8989666, "layer": 1, "discardable": false},
	{"frameId": 272, "dts": 9023000, "layer": 1, "discardable": false},
	{"frameId": 273, "dts": 9056333, "layer": 0, "discardable": true},
	{"frameId": 274, "dts": 9089666, "layer": 0, "discardable": true},
	{"frameId": 275, "dts": 9123000, "layer": 1, "discardable": false},
	{"frameId": 276, "dts": 9156333, "layer": 1, "discardable": false},
	{"frameId": 277, "dts": 9189666, "layer": 1, "discardable": false},
	{"frameId": 278, "dts": 9223000, "layer": 1, "discardable": false},
	{"frameId": 279, "dts": 9256333, "layer": 0, "discardable": true},
	{"frameId": 280, "dts": 9289666, "layer": 0, "discardable": true},
	{"frameId": 281, "dts": 9323000, "layer": 1, "discardable": false},
	{"frameId": 282, "dts": 9356333, "layer": 1, "discardable": false},
	{"frameId": 283, "dts": 9389666, "layer": 0, "discardable": true},
	{"frameId": 284, "dts": 9423000, "layer": 0, "discardable": true},
	{"frameId": 285, "dts": 9456333, "layer": 1, "discardable": false},
	{"frameId": 286, "dts": 9489666, "layer": 1, "discardable": false},
	{"frameId": 287, "dts": 9523000, "layer": 1, "discardable": false},
	{"frameId": 288, "dts": 9556333, "layer": 1, "discardable": false},
	{"frameId": 289, "dts": 9589666, "layer": 0, "discardable": true},
	{"frameId": 290, "dts": 9623000, "layer": 0, "discardable": true},
	{"frameId": 291, "dts": 9656333, "layer": 1, "discardable": false},
	{"frameId": 292, "dts": 9689666, "layer": 1, "discardable": false},
	{"frameId": 293, "dts": 9723000, "layer": 1, "discardable": false},
	{"frameId": 294, "dts": 9756333, "layer": 0, "discardable": true},
	{"frameId": 295, "dts": 9789666, "layer": 0, "discardable": true},
	{"frameId": 296, "dts": 9823000, "layer": 1, "discardable": false},
	{"frameId": 297, "dts": 9856333, "layer": 1, "discardable": false},
	{"frameId": 298, "dts": 9889666, "layer": 0, "discardable": true},
	{"frameId": 299, "dts": 9923000, "layer": 0, "discardable": true},
	{"frameId": 300, "dts": 9956333, "layer": 0, "discardable": true},
	{"frameId": 301, "dts": 9989666, "layer": 1, "discardable": false},
	{"frameId": 302, "dts": 10023000, "layer": 1, "discardable": false},
	{"frameId": 303, "dts": 10056333, "layer": 1, "discardable": false},
	{"frameId": 304, "dts": 10089666, "layer": 1, "discardable": false},
	{"frameId": 305, "dts": 10123000, "layer": 0, "discardable": true},
	{"frameId": 306, "dts": 10156333, "layer": 0, "discardable": true},
	{"frameId": 307, "dts": 10189666, "layer": 1, "discardable": false},
	{"frameId": 308, "dts": 10223000, "layer": 1, "discardable": false},
	{"frameId": 309, "dts": 10256333, "layer": 1, "discardable": false},
	{"frameId": 310, "dts": 10289666, "layer": 1, "discardable": false},
	{"frameId": 311, "dts": 10323000, "layer": 0, "discardable": true},
	{"frameId": 312, "dts": 10356333, "layer": 0, "discardable": true},
	{"frameId": 313, "dts": 10389666, "layer": 1, "discardable": false},
	{"frameId": 314, "dts": 10423000, "layer": 1, "discardable": false},
	{"frameId": 315, "dts": 10456333, "layer": 0, "discardable": true},
	{"frameId": 316, "dts": 10489666, "layer": 0, "discardable": true},
	{"frameId": 317, "dts": 10523000, "layer": 0, "discardable": true},
	{"frameId": 318, "dts": 10556333, "layer": 1, "discardable": false},
	{"frameId": 319, "dts": 10589666, "layer": 1, "discardable": false},
	{"frameId": 320, "dts": 10623000, "layer": 0, "discardable": true},
	{"frameId": 321, "dts": 10656333, "layer": 0, "discardable": true},
	{"frameId": 322, "dts": 10689666, "layer": 1, "discardable": false},
	{"frameId": 323, "dts": 10723000, "layer": 1, "discardable": false},
	{"frameId": 324, "dts": 10756333, "layer": 0, "discardable": true},
	{"frameId": 325, "dts": 10789666, "layer": 1, "discardable": false},
	{"frameId": 326, "dts": 10823000, "layer": 1, "discardable": false},
	{"frameId": 327, "dts": 10856333, "layer": 0, "discardable": true},
	{"frameId": 328, "dts": 10889666, "layer": 0, "discardable": true},
	{"frameId": 329, "dts": 10923000, "layer": 1, "discardable": false},
	{"frameId": 330, "dts": 10956333, "layer": 1, "discardable": false},
	{"frameId": 331, "dts": 10989666, "layer": 1, "discardable": false},
	{"frameId": 332, "dts": 11023000, "layer": 0, "discardable": true},
	{"frameId": 333, "dts": 11056333, "layer": 1, "discardable": false},
	{"frameId": 334, "dts": 11089666, "layer": 1, "discardable": false},
	{"frameId": 335, "dts": 11123000, "layer": 1, "discardable": false},
	{"frameId": 336, "dts": 11156333, "layer": 0, "discardable": true},
	{"frameId": 337, "dts": 11189666, "layer": 0, "discardable": true},
	{"frameId": 338, "dts": 11223000, "layer": 1, "discardable": false},
	{"frameId": 339, "dts": 11256333, "layer": 1, "discardable": false},
	{"frameId": 340, "dts": 11289666, "layer": 0, "discardable": true},
	{"frameId": 341, "dts": 11323000, "layer": 0, "discardable": true},
	{"frameId": 342, "dts": 11356333, "layer": 1, "discardable": false},
	{"frameId": 343, "dts": 11389666, "layer": 1, "discardable": false},
	{"frameId": 344, "dts": 11423000, "layer": 0, "discardable": true},
	{"frameId": 345, "dts": 11456333, "layer": 0, "discardable": true},
	{"frameId": 346, "dts": 11489666, "layer": 1, "discardable": false},
	{"frameId": 347, "dts": 11523000, "layer": 1, "discardable": false},
	{"frameId": 348, "dts": 11556333, "layer": 0, "discardable": true},
	{"frameId": 349, "dts": 11589666, "layer": 0, "discardable": true},
	{"frameId": 350, "dts": 11623000, "layer": 1, "discardable": false},
	{"frameId": 351, "dts": 11656333, "layer": 1, "discardable": false},
	{"frameId": 352, "dts": 11689666, "layer": 0, "discardable": true},
	{"frameId": 353, "dts": 11723000, "layer": 0, "discardable": true},
	{"frameId": 354, "dts": 11756333, "layer": 1, "discardable": false},
	{"frameId": 355, "dts": 11789666, "layer": 1, "discardable": false},
	{"frameId": 356, "dts": 11823000, "layer": 1, "discardable": false},
	{"frameId": 357, "dts": 11856333, "layer": 1, "discardable": false},
	{"frameId": 358, "dts": 11889666, "layer": 1, "discardable": false},
	{"frameId": 359, "dts": 11923000, "layer": 1, "discardable": false},
	{"frameId": 360, "dts": 11956333, "layer": 0, "discardable": true},
	{"frameId": 361, "dts": 11989666, "layer": 0, "discardable": true},
	{"frameId": 362, "dts": 12023000, "layer": 1, "discardable": false},
	{"frameId": 363, "dts": 12056333, "layer": 1, "discardable": false},
	{"frameId": 364, "dts": 12089666, "layer": 0, "discardable": true},
	{"frameId": 365, "dts": 12123000, "layer": 0, "discardable": true},
	{"frameId": 366, "dts": 12156333, "layer": 1, "discardable": false},
	{"frameId": 367, "dts": 12189666, "layer": 0, "discardable": true},
	{"frameId": 368, "dts": 12223000, "layer": 1, "discardable": false},
	{"frameId": 369, "dts": 12256333, "layer": 1, "discardable": false},
	{"frameId": 370, "dts": 12289666, "layer": 0, "discardable": true},
	{"frameId": 371, "dts": 12323000, "layer": 0, "discardable": true}
])"_json;

#endif //LAYER_INFO_ONE_I_HEVC_H