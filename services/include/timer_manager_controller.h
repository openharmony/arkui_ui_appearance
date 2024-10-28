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

#ifndef TIMER_MANAGER_CONTROLLER_H
#define TIMER_MANAGER_CONTROLLER_H

#include <array>
#include <map>
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include "ui_appearance_timer_manager.h"

namespace OHOS {
namespace ArkUi::UiAppearance {

enum class UiAppearanceType {
    DarkColorMode,
};

class TimerManagerController {
public:
    TimerManagerController() = default;
    virtual ~TimerManagerController() = default;
    static TimerManagerController& GetInstance();
    std::shared_ptr<UiAppearanceTimerManager> GetTimerManagerByType(UiAppearanceType uiAppearanceType);

private:
    std::map<UiAppearanceType, std::shared_ptr<UiAppearanceTimerManager>> timerManagerMap_;
};

} // namespace ArkUi::UiAppearance
} // namespace OHOS
#endif // TIMER_MANAGER_CONTROLLER_H