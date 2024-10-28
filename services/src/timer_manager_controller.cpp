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

#include "timer_manager_controller.h"
#include <memory>
#include "ui_appearance_log.h"

namespace OHOS {
namespace ArkUi::UiAppearance {

TimerManagerController& TimerManagerController::GetInstance()
{
    static TimerManagerController instance;
    return instance;
}

std::shared_ptr<UiAppearanceTimerManager> TimerManagerController::GetTimerManagerByType(
    UiAppearanceType uiAppearanceType)
{
    if (timerManagerMap_.find(uiAppearanceType) == timerManagerMap_.end()) {
        LOGI("init UiAppearanceTimerManager %{public}d", static_cast<uint8_t>(uiAppearanceType));
        UiAppearanceTimerManager uiAppearanceTimerManager = UiAppearanceTimerManager();
        timerManagerMap_[uiAppearanceType] = std::make_shared<UiAppearanceTimerManager>(
            uiAppearanceTimerManager);
    }
    return timerManagerMap_[uiAppearanceType];
}

} // namespace ArkUi::UiAppearance
} // namespace OHOS