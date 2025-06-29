/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ui_appearance_log.h"
#include "background_app_color_switch_settings.h"
#include "json_utils.h"
#include <algorithm>

namespace {
constexpr const char* CONFIG_PATH = "/etc/dark_mode_whilelist.json";
constexpr const char* ALLOW_LIST = "whiteList";
constexpr const char* BUNDLE_NAME = "bundleName";
constexpr const char* STRATEGY = "strategy";
constexpr const char* DURATION = "duration";
constexpr const char* PERTASK_NUMBER = "perTaskNumber";
} // namespace

namespace OHOS::ArkUi::UiAppearance {
    BackGroundAppColorSwitchSettings &BackGroundAppColorSwitchSettings::GetInstance()
{
    static BackGroundAppColorSwitchSettings instance;
    return instance;
}

bool BackGroundAppColorSwitchSettings::IsSupportHotUpdate()
{
    std::lock_guard lock(policyMutex_);
    return isAllowListEnable_;
}

ErrCode BackGroundAppColorSwitchSettings::Initialize()
{
    std::lock_guard lock(policyMutex_);
    nlohmann::json object;
    if (!JsonUtils::LoadConfiguration(CONFIG_PATH, object)) {
        LOGW("BackGroundAppColorSwitchSettings read file failed");
        return ERR_NAME_NOT_FOUND;
    }

    if (!object.contains(ALLOW_LIST) || !object.at(ALLOW_LIST).is_array()) {
        LOGW("BackGroundAppColorSwitchSettings unable to query white list");
        return ERR_INVALID_VALUE;
    }

    for (auto &item : object.at(ALLOW_LIST).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.contains(BUNDLE_NAME) || !jsonObject.at(BUNDLE_NAME).is_string()) {
            continue;
        }
        auto bundleName = jsonObject.at(BUNDLE_NAME).get<std::string>();
        LOGI("insert allowList_ bundleName = %{public}s", bundleName.c_str());
        allowList_.push_back(bundleName);
    }

    if (!object.contains(STRATEGY)) {
        LOGW("BackGroundAppColorSwitchSettings unable to query strategy");
        return ERR_NAME_NOT_FOUND;
    }

    auto strategy = object.at(STRATEGY);
    if (!strategy.contains(DURATION) || !strategy.at(DURATION).is_number()) {
        LOGW("BackGroundAppColorSwitchSettings unable to query duration");
        return ERR_INVALID_VALUE;
    }
    durationMillisecond_ = strategy.at(DURATION).get<int32_t>();

    if (!strategy.contains(PERTASK_NUMBER) || !strategy.at(PERTASK_NUMBER).is_number()) {
        LOGW("BackGroundAppColorSwitchSettings unable to query perTaskNumber");
        return ERR_INVALID_VALUE;
    }
    taskQuantity_ = strategy.at(PERTASK_NUMBER).get<int32_t>();
    if (taskQuantity_ <= 0 || durationMillisecond_ <= 0) {
        LOGW("settings error, taskQuantity_:%{public}d durationMillisecond_:%{public}d",
            taskQuantity_, durationMillisecond_);
        return ERR_INVALID_VALUE;
    }
    isAllowListEnable_ = true;
    
    LOGI("taskQuantity_= %{public}d durationMillisecond_= %{public}d", taskQuantity_, durationMillisecond_);
    return ERR_OK;
}

int32_t BackGroundAppColorSwitchSettings::GetTaskQuantity()
{
    std::lock_guard lock(policyMutex_);
    return taskQuantity_;
}

int32_t BackGroundAppColorSwitchSettings::GetDurationMillisecond()
{
    std::lock_guard lock(policyMutex_);
    return durationMillisecond_;
}

bool BackGroundAppColorSwitchSettings::CheckInWhileList(const std::string& bundleName)
{
    std::lock_guard lock(policyMutex_);
    if (std::find(allowList_.begin(), allowList_.end(), bundleName) != allowList_.end()) {
        return true;
    }
    return false;
}

void BackGroundAppColorSwitchSettings::Reset()
{
    std::lock_guard lock(policyMutex_);
    allowList_.clear();
    taskQuantity_ = -1;
    durationMillisecond_ = -1;
    isAllowListEnable_ = false;
}

std::list<std::string> BackGroundAppColorSwitchSettings::GetWhileList()
{
    std::lock_guard lock(policyMutex_);
    return allowList_;
}
} // namespace OHOS::ArkUi::UiAppearance
