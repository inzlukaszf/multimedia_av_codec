/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#include "dash_seg_tmline_node.h"
#include "dash_mpd_util.h"

namespace OHOS {
namespace Media {
namespace Plugins {
namespace HttpPlugin {
DashSegTmlineNode::DashSegTmlineNode()
{
    for (uint32_t index = 0; index < DASH_SEG_TIMELINE_ATTR_NUM; index++) {
        segTmlineAttr_[index].attr_.assign(segTmlineAttrs_[index]);
        segTmlineAttr_[index].val_.assign("");
    }
}

DashSegTmlineNode::~DashSegTmlineNode() {}

void DashSegTmlineNode::ParseNode(std::shared_ptr<XmlParser> xmlParser, std::shared_ptr<XmlElement> rootElement)
{
    if (xmlParser != nullptr) {
        for (uint32_t index = 0; index < DASH_SEG_TIMELINE_ATTR_NUM; index++) {
            if (static_cast<int32_t>(XmlBaseRtnValue::XML_BASE_OK) ==
                xmlParser->GetAttribute(rootElement, segTmlineAttr_[index].attr_, segTmlineAttr_[index].val_)) {
            }
        }
    }
}

void DashSegTmlineNode::GetAttr(const std::string &attrName, std::string &sAttrVal)
{
    uint32_t index = DashGetAttrIndex(attrName, segTmlineAttrs_, DASH_SEG_TIMELINE_ATTR_NUM);
    if (index < DASH_SEG_TIMELINE_ATTR_NUM) {
        if (segTmlineAttr_[index].val_.length() > 0) {
            sAttrVal = segTmlineAttr_[index].val_;
        } else {
            sAttrVal.assign("");
        }
    }
}

void DashSegTmlineNode::GetAttr(const std::string &attrName, uint32_t &uiAttrVal)
{
    uint32_t index = DashGetAttrIndex(attrName, segTmlineAttrs_, DASH_SEG_TIMELINE_ATTR_NUM);
    if (index < DASH_SEG_TIMELINE_ATTR_NUM) {
        if (segTmlineAttr_[index].val_.length() > 0) {
            uiAttrVal = static_cast<uint32_t>(std::atoll(segTmlineAttr_[index].val_.c_str()));
        } else {
            uiAttrVal = 0;
        }
    }
}

void DashSegTmlineNode::GetAttr(const std::string &attrName, int32_t &iAttrVal)
{
    uint32_t index = DashGetAttrIndex(attrName, segTmlineAttrs_, DASH_SEG_TIMELINE_ATTR_NUM);
    if (index < DASH_SEG_TIMELINE_ATTR_NUM) {
        if (segTmlineAttr_[index].val_.length() > 0) {
            iAttrVal = static_cast<int32_t>(std::atoll(segTmlineAttr_[index].val_.c_str()));
        } else {
            iAttrVal = 0;
        }
    }
}

void DashSegTmlineNode::GetAttr(const std::string &attrName, uint64_t &ullAttrVal)
{
    uint32_t index = DashGetAttrIndex(attrName, segTmlineAttrs_, DASH_SEG_TIMELINE_ATTR_NUM);
    if (index < DASH_SEG_TIMELINE_ATTR_NUM) {
        if (segTmlineAttr_[index].val_.length() > 0) {
            ullAttrVal = static_cast<uint64_t>(std::atoll(segTmlineAttr_[index].val_.c_str()));
        } else {
            ullAttrVal = 0;
        }
    }
}

void DashSegTmlineNode::GetAttr(const std::string &attrName, double &dAttrVal)
{
    uint32_t index = DashGetAttrIndex(attrName, segTmlineAttrs_, DASH_SEG_TIMELINE_ATTR_NUM);
    if (index < DASH_SEG_TIMELINE_ATTR_NUM) {
        if (segTmlineAttr_[index].val_.length() > 0) {
            dAttrVal = atof(segTmlineAttr_[index].val_.c_str());
        } else {
            dAttrVal = 0.0;
        }
    }
}
} // namespace HttpPluginLite
} // namespace Plugin
} // namespace Media
} // namespace OHOS