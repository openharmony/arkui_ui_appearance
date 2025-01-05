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

#ifndef UI_APPEARANCE_SCREEN_SWITCH_OPERATOR_MANAGER_H
#define UI_APPEARANCE_SCREEN_SWITCH_OPERATOR_MANAGER_H

#include <functional>
#include <vector>
#include <mutex>


namespace OHOS::ArkUi::UiAppearance {
class ScreenSwitchOperatorManager {
public:
    bool IsScreenOn();
    bool IsScreenOff();
    void SetScreenOn();
    void SetScreenOff();

    void ResetScreenOffOperateInfo();
    void SetScreenOffOperateInfo(const bool switchToDark, const int32_t userId);
    void GetScreenOffOperateInfo(bool &switchToDark, int32_t &userId);
    bool HaveScreenOffOperate();

private:
    enum class ScreenSwitchType {
        SCREEN_OFF = 0,
        SCREEN_ON,
    };
    enum class ScreenOffOperateType {
        NONE = 0,
        SWITCH_TO_LIGHT,
        SWITCH_TO_DARK,
    };
    struct ScreenOffOperateInfoType {
        ScreenOffOperateType screenOffOperator = ScreenOffOperateType::NONE;
        int32_t userId;
    };
    std::mutex screenSwitchMutex_;
    ScreenSwitchType screenSwitch_;
    std::mutex screenOffOperateInfoMutex_;
    ScreenOffOperateInfoType screenOffOperateInfo_;
};
} // namespace OHOS::ArkUi::UiAppearance

#endif // UI_APPEARANCE_SCREEN_SWITCH_OPERATOR_MANAGER_H
