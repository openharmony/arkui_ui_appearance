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

#include <functional>
#include <vector>
#include <mutex>
#include "screen_switch_operator_manager.h"

namespace {
constexpr int32_t USER100 = 100;
}
namespace OHOS::ArkUi::UiAppearance {
bool ScreenSwitchOperatorManager::IsScreenOn()
{
    std::lock_guard guard(screenSwitchMutex_);
    return screenSwitch_ == ScreenSwitchType::SCREEN_ON;
}
bool ScreenSwitchOperatorManager::IsScreenOff()
{
    std::lock_guard guard(screenSwitchMutex_);
    return screenSwitch_ == ScreenSwitchType::SCREEN_OFF;
}
void ScreenSwitchOperatorManager::SetScreenOn()
{
    std::lock_guard guard(screenSwitchMutex_);
    screenSwitch_ = ScreenSwitchType::SCREEN_ON;
}
void ScreenSwitchOperatorManager::SetScreenOff()
{
    std::lock_guard guard(screenSwitchMutex_);
    screenSwitch_ = ScreenSwitchType::SCREEN_OFF;
}

void ScreenSwitchOperatorManager::ResetScreenOffOperateInfo()
{
    std::lock_guard guard(screenOffOperateInfoMutex_);
    screenOffOperateInfo_.screenOffOperator = ScreenOffOperateType::NONE;
    screenOffOperateInfo_.userId = USER100;
}
void ScreenSwitchOperatorManager::SetScreenOffOperateInfo(const bool switchToDark, const int32_t userId)
{
    std::lock_guard guard(screenOffOperateInfoMutex_);
    screenOffOperateInfo_.screenOffOperator =
        switchToDark ? ScreenOffOperateType::SWITCH_TO_DARK : ScreenOffOperateType::SWITCH_TO_LIGHT;
    screenOffOperateInfo_.userId = userId;
}
void ScreenSwitchOperatorManager::GetScreenOffOperateInfo(bool &switchToDark, int32_t &userId)
{
    std::lock_guard guard(screenOffOperateInfoMutex_);
    userId = screenOffOperateInfo_.userId;
    switchToDark = screenOffOperateInfo_.screenOffOperator == ScreenOffOperateType::SWITCH_TO_DARK ? true : false;
}
bool ScreenSwitchOperatorManager::HaveScreenOffOperate()
{
    std::lock_guard guard(screenOffOperateInfoMutex_);
    return screenOffOperateInfo_.screenOffOperator != ScreenOffOperateType::NONE;
}

} // namespace OHOS::ArkUi::UiAppearance
