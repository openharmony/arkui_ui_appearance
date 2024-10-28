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

#ifndef UI_APPEARANCE_TIMER_MANAGER_H
#define UI_APPEARANCE_TIMER_MANAGER_H

#include <array>
#include <map>
#include <cstdint>
#include <sys/types.h>
#include "alarm_timer.h"

namespace OHOS {
namespace ArkUi::UiAppearance {

const uint32_t TRIGGER_ARRAY_SIZE = 2;

class UiAppearanceTimerManager {
public:
    UiAppearanceTimerManager() = default;
    virtual ~UiAppearanceTimerManager() = default;
    uint32_t SetScheduleTime(const uint64_t startTime, const uint64_t endTime, const uint32_t userId,
        std::function<void()> startCallback, std::function<void()> endCallback);
    bool IsVaildScheduleTime(const uint64_t startTime, const uint64_t endTime);
    void SetTimerTriggerTime(const uint64_t startTime, const uint64_t endTime,
        std::array<uint64_t, TRIGGER_ARRAY_SIZE> &triggerTimeInterval);
    void SetTimer(const int8_t index, const uint32_t userId, const uint64_t time, std::function<void()> callback);
    uint64_t InitTimer(const uint64_t time, std::function<void()> callback);
    void ClearTimer(const uint64_t id);
    uint64_t UpdateTimer(const uint64_t id, const uint64_t time, std::function<void()> callback);
    void ClearTimerByUserId(const uint64_t userId);
    bool IsWithinTimeInterval(const uint64_t startTime, const uint64_t endTime);
    void RecordInitialSetupTime(const uint64_t startTime, const uint64_t endTime, const uint32_t userId);
    bool RestartTimerByUserId(const uint64_t userId = 0);
    bool RestartAllTimer();
    void RestartTimerByTimerId(const uint64_t timerId, const uint64_t time);
private:
    std::map<uint32_t, std::array<uint64_t, TRIGGER_ARRAY_SIZE>> timerIdMap_;
    std::map<uint32_t, std::array<uint64_t, TRIGGER_ARRAY_SIZE>> initialSetupTimeMap_;
};

} // namespace ArkUi::UiAppearance
} // namespace OHOS
#endif // UI_APPEARANCE_DARK_MODE_TIMER_H