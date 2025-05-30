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

#ifndef UI_APPEARANCE_BACKGROUND_APP_COLOR_SWICH_SETTINGS_H
#define UI_APPEARANCE_BACKGROUND_APP_COLOR_SWICH_SETTINGS_H

#include <mutex>
#include <unordered_set>

#include "errors.h"
#include "nocopyable.h"

namespace OHOS::ArkUi::UiAppearance {
class BackGroundAppColorSwitchSettings final : public NoCopyable {
public:
    static BackGroundAppColorSwitchSettings& GetInstance();

    bool IsSupportHotUpdate();

    ErrCode Initialize();

    void Reset();

    int32_t GetTaskQuantity();

    int32_t GetDurationMillisecond();

    bool CheckInWhileList(const std::string& bundleName);

    std::unordered_set<std::string> GetWhileList();
private:
    std::mutex policyMutex_;
    bool isAllowListEnable_ = false;

    std::unordered_set<std::string> allowList_;
    int32_t taskQuantity_ = -1;
    int32_t durationMillisecond_ = -1;
};
} // namespace OHOS::ArkUi::UiAppearance

#endif // UI_APPEARANCE_UPDATE_POLICY_MANAGER_H
