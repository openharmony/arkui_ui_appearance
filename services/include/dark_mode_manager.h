/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef UI_APPEARANCE_DARK_MODE_MANAGER_H
#define UI_APPEARANCE_DARK_MODE_MANAGER_H

#include <functional>
#include <list>

#include "errors.h"
#include "nocopyable.h"
#include "alarm_timer_manager.h"
#include "dark_mode_temp_state_manager.h"
#include "screen_switch_operator_manager.h"

namespace OHOS::ArkUi::UiAppearance {
constexpr int32_t HOUR_TO_MINUTE = 60;
constexpr int32_t DAY_TO_MINUTE = 24 * 60;
constexpr int32_t SUNSET_TIME_DEFAULT = 18 * HOUR_TO_MINUTE;
constexpr int32_t SUNRISE_TIME_DEFAULT = 7 * HOUR_TO_MINUTE + DAY_TO_MINUTE;
class DarkModeManager final : public NoCopyable {
public:
    static DarkModeManager &GetInstance();

    ErrCode Initialize(const std::function<void(bool, int32_t)>& updateCallback);

    ErrCode LoadUserSettingData(int32_t userId, bool needUpdateCallback, bool &isDarkMode);

    void NotifyDarkModeUpdate(int32_t userId, bool isDarkMode);

    ErrCode OnSwitchUser(int32_t userId);

    void ScreenOnCallback();

    void ScreenOffCallback();

    ErrCode RestartTimer();

    void Dump();

    bool GetSettingTime(const int32_t userId, int32_t& settingStartTime, int32_t& settingEndTime);

    bool IsColorModeNormal(const int32_t userId);

    void DoSwitchTemporaryColorMode(const int32_t userId, bool isDarkMode);

private:
    enum DarkModeMode {
        DARK_MODE_INVALID = -1,
        DARK_MODE_ALWAYS_LIGHT = 0,
        DARK_MODE_ALWAYS_DARK = 1,
        DARK_MODE_CUSTOM_AUTO = 2,
        DARK_MODE_SUNRISE_SUNSET = 3,
        DARK_MODE_SIZE,
    };

    struct DarkModeState {
        DarkModeMode settingMode = DARK_MODE_INVALID;
        int32_t settingStartTime = -1;
        int32_t settingEndTime = -1;
        int32_t settingSunsetTime = SUNSET_TIME_DEFAULT;   // Default sunset Time 6pm
        int32_t settingSunriseTime = SUNRISE_TIME_DEFAULT; // Default sunrise time: 7am the next day
    };

    void LoadSettingDataObserversCallback();

    ErrCode RegisterSettingDataObserversLocked(int32_t userId) const;

    void UnregisterSettingDataObserversLocked(int32_t userId) const;

    void SettingDataDarkModeModeUpdateFunc(const std::string& key, int32_t userId);

    void SettingDataDarkModeStartTimeUpdateFunc(const std::string& key, int32_t userId);

    void SettingDataDarkModeEndTimeUpdateFunc(const std::string& key, int32_t userId);

    void SettingDataDarkModeSunsetTimeUpdateFunc(const std::string& key, int32_t userId);

    void SettingDataDarkModeSunriseTimeUpdateFunc(const std::string& key, int32_t userId);

    ErrCode OnStateChangeLocked(
        int32_t userId, bool needUpdateCallback, bool& isDarkMode, const bool resetTempColorModeFlag);

    ErrCode OnStateChangeToAllDayMode(int32_t userId, DarkModeMode darkMode, bool needUpdateCallback, bool& isDarkMode,
        const bool resetTempColorModeFlag);

    ErrCode OnStateChangeToCustomAutoMode(int32_t userId, const DarkModeState& state, bool needUpdateCallback,
        bool& isDarkMode, const bool resetTempColorModeFlag);

    void OnChangeDarkMode(DarkModeMode mode, int32_t userId);

    ErrCode CreateOrUpdateTimers(int32_t startTime, int32_t endTime, int32_t userId);

    ErrCode CheckTimerCallbackParams(int32_t startTime, int32_t endTime, int32_t userId);

    void UpdateDarkModeSchedule(const DarkModeMode isDarkMode, const int32_t userId, const bool resetTempColorModeFlag);

    bool IsDarkModeCustomAuto(const int32_t userId);

    bool IsDarkModeSunsetSunrise(const int32_t userId);

    std::mutex settingDataObserversMutex_;
    std::list<std::pair<std::string, std::function<void(const std::string&, int32_t)>>> settingDataObservers_;
    int32_t settingDataObserversUserId_ = -1;

    AlarmTimerManager alarmTimerManager_;
    std::mutex darkModeStatesMutex_;
    std::map<int32_t, DarkModeState> darkModeStates_;

    std::function<void(bool, int32_t)> updateCallback_;

    TemporaryColorModeManager temporaryColorModeMgr_;
    ScreenSwitchOperatorManager screenSwitchOperatorMgr_;
};
} // namespace OHOS::ArkUi::UiAppearance

#endif // UI_APPEARANCE_DARK_MODE_MANAGER_H
