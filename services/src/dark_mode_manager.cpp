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

#include "dark_mode_manager.h"

#include "setting_data_manager.h"
#include "ui_appearance_log.h"

namespace OHOS::ArkUi::UiAppearance {
namespace {
const std::string SETTING_DARK_MODE_MODE = "settings.uiappearance.darkmode_mode";
const std::string SETTING_DARK_MODE_START_TIME = "settings.uiappearance.darkmode_starttime";
const std::string SETTING_DARK_MODE_END_TIME = "settings.uiappearance.darkmode_endtime";
const std::string SETTING_DARK_MODE_SUN_SET = "settings.display.sun_set";
const std::string SETTING_DARK_MODE_SUN_RISE = "settings.display.sun_rise";
const static int32_t USER100 = 100;
}

DarkModeManager &DarkModeManager::GetInstance()
{
    static DarkModeManager instance;
    return instance;
}

ErrCode DarkModeManager::Initialize(const std::function<void(bool, int32_t)>& updateCallback)
{
    LoadSettingDataObserversCallback();
    updateCallback_ = updateCallback;
    return ERR_OK;
}

ErrCode DarkModeManager::LoadUserSettingData(
    const int32_t userId, const bool needUpdateCallback, bool &isDarkMode, const bool bootLoadFlag)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    int32_t darkMode = DARK_MODE_INVALID;
    manager.GetInt32ValueStrictly(SETTING_DARK_MODE_MODE, darkMode, userId);
    if (darkMode < DARK_MODE_INVALID || darkMode >= DARK_MODE_SIZE) {
        LOGE("dark mode out of range: %{public}d, userId: %{public}d", darkMode, userId);
        darkMode = DARK_MODE_INVALID;
    }
    int32_t startTime = -1;
    manager.GetInt32ValueStrictly(SETTING_DARK_MODE_START_TIME, startTime, userId);
    int32_t endTime = -1;
    manager.GetInt32ValueStrictly(SETTING_DARK_MODE_END_TIME, endTime, userId);
    int32_t sunsetTime = SUNSET_TIME_DEFAULT;
    manager.GetInt32ValueStrictly(SETTING_DARK_MODE_SUN_SET, sunsetTime, userId);
    int32_t sunriseTime = SUNRISE_TIME_DEFAULT;
    manager.GetInt32ValueStrictly(SETTING_DARK_MODE_SUN_RISE, sunriseTime, userId);

    std::lock_guard lock(darkModeStatesMutex_);
    DarkModeState& state = darkModeStates_[userId];
    state.settingMode = static_cast<DarkModeMode>(darkMode);
    state.settingStartTime = startTime;
    state.settingEndTime = endTime;
    state.settingSunsetTime = sunsetTime;
    state.settingSunriseTime = sunriseTime;
    LOGI("load user setting data, userId: %{public}d, mode: %{public}d, start: %{public}d, end : %{public}d",
        userId, darkMode, startTime, endTime);
    temporaryColorModeMgr_.InitData(userId);
    if (temporaryColorModeMgr_.IsColorModeTemporary(userId) &&
        temporaryColorModeMgr_.CheckTemporaryStateEffective(userId) == false) {
        temporaryColorModeMgr_.SetColorModeNormal(userId);
    }
    screenSwitchOperatorMgr_.ResetScreenOffOperateInfo();
    return OnStateChangeLocked(userId, needUpdateCallback, isDarkMode, false, bootLoadFlag);
}

void DarkModeManager::NotifyDarkModeUpdate(const int32_t userId, const bool isDarkMode)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    std::lock_guard lock(darkModeStatesMutex_);
    const DarkModeState& state = darkModeStates_[userId];
    if (isDarkMode) {
        if (state.settingMode == DARK_MODE_ALWAYS_LIGHT || state.settingMode == DARK_MODE_INVALID) {
            LOGI("notify change to always dark, userId: %{public}d", userId);
            manager.SetStringValue(SETTING_DARK_MODE_MODE, std::to_string(DARK_MODE_ALWAYS_DARK), userId);
        } // else no need to change
    } else {
        if (state.settingMode == DARK_MODE_ALWAYS_DARK || state.settingMode == DARK_MODE_INVALID) {
            LOGI("notify change to always light, userId: %{public}d", userId);
            manager.SetStringValue(SETTING_DARK_MODE_MODE, std::to_string(DARK_MODE_ALWAYS_LIGHT), userId);
        } // else no need to change
    }
}

void DarkModeManager::ScreenOnCallback()
{
    screenSwitchOperatorMgr_.SetScreenOn();
}

void DarkModeManager::ScreenOffCallback()
{
    screenSwitchOperatorMgr_.SetScreenOff();
    if (screenSwitchOperatorMgr_.HaveScreenOffOperate()) {
        bool switchToDark = false;
        int32_t userId = USER100;
        screenSwitchOperatorMgr_.GetScreenOffOperateInfo(switchToDark, userId);
        OnChangeDarkMode(
            switchToDark == true ? DarkModeMode::DARK_MODE_ALWAYS_DARK : DarkModeMode::DARK_MODE_ALWAYS_LIGHT,
            userId);
        screenSwitchOperatorMgr_.ResetScreenOffOperateInfo();
    }
}

ErrCode DarkModeManager::OnSwitchUser(const int32_t userId)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    if (!manager.IsInitialized()) {
        ErrCode code = manager.Initialize();
        if (code != ERR_OK || manager.IsInitialized() == false) {
            LOGE("setting data manager is not initialized");
            return ERR_NO_INIT;
        }
    }

    if (userId <= INVALID_USER_ID) {
        LOGE("invalid userId: %{public}d", userId);
        return ERR_INVALID_OPERATION;
    }

    std::lock_guard lock(settingDataObserversMutex_);
    if (settingDataObserversUserId_ != INVALID_USER_ID) {
        LOGI("clear timers and unregister observers for userId: %{public}d", settingDataObserversUserId_);
        alarmTimerManager_.ClearTimerByUserId(settingDataObserversUserId_);
        UnregisterSettingDataObserversLocked(settingDataObserversUserId_);
        settingDataObserversUserId_ = INVALID_USER_ID;
    }

    ErrCode code = RegisterSettingDataObserversLocked(userId);
    settingDataObserversUserId_ = userId;
    return code;
}

void DarkModeManager::DoSwitchTemporaryColorMode(const int32_t userId, bool isDarkMode)
{
    if (IsDarkModeCustomAuto(userId) || IsDarkModeSunsetSunrise(userId)) {
        screenSwitchOperatorMgr_.ResetScreenOffOperateInfo();
        int32_t settingStartTime = 0;
        int32_t settingEndTime = 0;
        auto res = GetSettingTime(userId, settingStartTime, settingEndTime);
        if (res == false) {
            LOGE("GetSettingTime faild userId: %{public}d", userId);
            return;
        }
        if ((AlarmTimerManager::IsWithinTimeInterval(settingStartTime, settingEndTime) && isDarkMode == true) ||
            (!AlarmTimerManager::IsWithinTimeInterval(settingStartTime, settingEndTime) && isDarkMode == false)) {
            temporaryColorModeMgr_.SetColorModeNormal(userId);
        } else {
            temporaryColorModeMgr_.SetColorModeTemporary(userId);
        }
    }
}

void DarkModeManager::UpdateDarkModeSchedule(
    const DarkModeMode mode, const int32_t userId, const bool resetTempColorModeFlag, const bool bootLoadFlag)
{
    screenSwitchOperatorMgr_.ResetScreenOffOperateInfo();
    if (resetTempColorModeFlag == true) {
        OnChangeDarkMode(mode, userId);
        return;
    }

    if (screenSwitchOperatorMgr_.IsScreenOff() || bootLoadFlag) {
        if (temporaryColorModeMgr_.IsColorModeNormal(userId) ||
            temporaryColorModeMgr_.CheckTemporaryStateEffective(userId) == false) {
            OnChangeDarkMode(mode, userId);
        }
        return;
    }

    if (temporaryColorModeMgr_.IsColorModeNormal(userId) ||
        temporaryColorModeMgr_.CheckTemporaryStateEffective(userId) == false) {
        screenSwitchOperatorMgr_.SetScreenOffOperateInfo(mode == DARK_MODE_ALWAYS_DARK, userId);
        LOGI("SetScreenOffOperateInfo userId:%{public}d operate:%{public}d", userId, static_cast<int32_t>(mode));
    }
}

ErrCode DarkModeManager::RestartTimer()
{
    std::lock_guard lock(darkModeStatesMutex_);
    DarkModeMode mode = darkModeStates_[settingDataObserversUserId_].settingMode;
    int32_t startTime = -1;
    int32_t endTime = -1;

    if (mode == DARK_MODE_SUNRISE_SUNSET) {
        startTime = darkModeStates_[settingDataObserversUserId_].settingSunsetTime;
        endTime = darkModeStates_[settingDataObserversUserId_].settingSunriseTime;
    } else if (mode == DARK_MODE_CUSTOM_AUTO) {
        startTime = darkModeStates_[settingDataObserversUserId_].settingStartTime;
        endTime = darkModeStates_[settingDataObserversUserId_].settingEndTime;
    } else {
        LOGD("no need to restart timer.");
        return ERR_OK;
    }

    if (AlarmTimerManager::IsWithinTimeInterval(startTime, endTime)) {
        UpdateDarkModeSchedule(DARK_MODE_ALWAYS_DARK, settingDataObserversUserId_, false, false);
    } else {
        UpdateDarkModeSchedule(DARK_MODE_ALWAYS_LIGHT, settingDataObserversUserId_, false, false);
    }
    return alarmTimerManager_.RestartAllTimer();
}

bool DarkModeManager::IsDarkModeCustomAuto(const int32_t userId)
{
    std::lock_guard lock(darkModeStatesMutex_);
    return darkModeStates_[userId].settingMode == DARK_MODE_CUSTOM_AUTO;
}

bool DarkModeManager::IsDarkModeSunsetSunrise(const int32_t userId)
{
    std::lock_guard lock(darkModeStatesMutex_);
    return darkModeStates_[userId].settingMode == DARK_MODE_SUNRISE_SUNSET;
}

bool DarkModeManager::GetSettingTime(const int32_t userId, int32_t& settingStartTime, int32_t& settingEndTime)
{
    std::lock_guard lock(darkModeStatesMutex_);
    auto it = darkModeStates_.find(userId);
    if (it != darkModeStates_.end()) {
        if (it->second.settingMode == DARK_MODE_CUSTOM_AUTO) {
            settingStartTime = it->second.settingStartTime;
            settingEndTime = it->second.settingEndTime;
        } else {
            settingStartTime = it->second.settingSunsetTime;
            settingEndTime = it->second.settingSunriseTime;
        }
        return true;
    }
    return false;
}

bool DarkModeManager::IsColorModeNormal(const int32_t userId)
{
    return temporaryColorModeMgr_.IsColorModeNormal(userId);
}

void DarkModeManager::Dump()
{
    {
        std::lock_guard observersGuard(settingDataObserversMutex_);
        LOGD("settingData observers size: %{public}zu, userId: %{public}d",
            settingDataObservers_.size(), settingDataObserversUserId_);
    }

    std::lock_guard stateGuard(darkModeStatesMutex_);
    LOGD("darkModeStates size: %{public}zu", darkModeStates_.size());
    for (const auto& state : darkModeStates_) {
        LOGD("userId: %{public}d, mode: %{public}d, start: %{public}d, end: %{public}d",
            state.first, state.second.settingMode, state.second.settingStartTime, state.second.settingEndTime);
    }

    alarmTimerManager_.Dump();
}

void DarkModeManager::LoadSettingDataObserversCallback()
{
    std::lock_guard lock(settingDataObserversMutex_);
    settingDataObservers_.clear();
    settingDataObservers_.emplace_back(SETTING_DARK_MODE_MODE, [&](const std::string& key, int32_t userId) {
        SettingDataDarkModeModeUpdateFunc(key, userId);
    });
    settingDataObservers_.emplace_back(SETTING_DARK_MODE_START_TIME, [&](const std::string& key, int32_t userId) {
        SettingDataDarkModeStartTimeUpdateFunc(key, userId);
    });
    settingDataObservers_.emplace_back(SETTING_DARK_MODE_END_TIME, [&](const std::string& key, int32_t userId) {
        SettingDataDarkModeEndTimeUpdateFunc(key, userId);
    });
    settingDataObservers_.emplace_back(SETTING_DARK_MODE_SUN_SET, [&](const std::string& key, int32_t userId) {
        SettingDataDarkModeSunsetTimeUpdateFunc(key, userId);
    });
    settingDataObservers_.emplace_back(SETTING_DARK_MODE_SUN_RISE, [&](const std::string& key, int32_t userId) {
        SettingDataDarkModeSunriseTimeUpdateFunc(key, userId);
    });
}

ErrCode DarkModeManager::RegisterSettingDataObserversLocked(const int32_t userId) const
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    size_t count = 0;
    for (const auto& observer : settingDataObservers_) {
        if (manager.RegisterObserver(observer.first, observer.second, userId) != ERR_OK) {
            count++;
        }
    }
    if (count != 0) {
        LOGE("setting data observers are not all initialized");
        return ERR_NO_INIT;
    }
    LOGD("setting data observers are all initialized");
    return ERR_OK;
}

void DarkModeManager::UnregisterSettingDataObserversLocked(const int32_t userId) const
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    for (const auto& observer : settingDataObservers_) {
        manager.UnregisterObserver(observer.first, userId);
    }
}

void DarkModeManager::SettingDataDarkModeModeUpdateFunc(const std::string& key, const int32_t userId)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    int32_t value = DARK_MODE_INVALID;
    ErrCode code = manager.GetInt32ValueStrictly(key, value, userId);
    if (code != ERR_OK) {
        LOGE("get dark mode value failed, key: %{public}s, userId: %{public}d, code: %{public}d, set to default",
            key.c_str(), userId, code);
        value = DARK_MODE_INVALID;
    }
    if (value < DARK_MODE_INVALID || value >= DARK_MODE_SIZE) {
        LOGE("dark mode value is invalid, key: %{public}s, userId: %{public}d, value: %{public}d, set to default",
            key.c_str(), userId, value);
        value = DARK_MODE_INVALID;
    }

    auto mode = static_cast<DarkModeMode>(value);
    std::lock_guard lock(darkModeStatesMutex_);
    LOGI("dark mode change, key: %{public}s, userId: %{public}d, from %{public}d to %{public}d",
        key.c_str(), userId, darkModeStates_[userId].settingMode, value);
    darkModeStates_[userId].settingMode = mode;
    bool isDarkMode = false;
    OnStateChangeLocked(userId, true, isDarkMode, true, false);
}

void DarkModeManager::SettingDataDarkModeStartTimeUpdateFunc(const std::string& key, const int32_t userId)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    int32_t value = -1;
    manager.GetInt32ValueStrictly(key, value, userId);
    std::lock_guard lock(darkModeStatesMutex_);
    LOGI("dark mode start time change, key: %{public}s, userId: %{public}d, from %{public}d to %{public}d",
        key.c_str(), userId, darkModeStates_[userId].settingStartTime, value);
    darkModeStates_[userId].settingStartTime = value;
    bool isDarkMode = false;
    OnStateChangeLocked(userId, true, isDarkMode, true, false);
}

void DarkModeManager::SettingDataDarkModeEndTimeUpdateFunc(const std::string& key, const int32_t userId)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    int32_t value = -1;
    manager.GetInt32ValueStrictly(key, value, userId);
    std::lock_guard lock(darkModeStatesMutex_);
    LOGI("dark mode end time change, key: %{public}s, userId: %{public}d, from %{public}d to %{public}d",
        key.c_str(), userId, darkModeStates_[userId].settingEndTime, value);
    darkModeStates_[userId].settingEndTime = value;
    bool isDarkMode = false;
    OnStateChangeLocked(userId, true, isDarkMode, true, false);
}

void DarkModeManager::SettingDataDarkModeSunsetTimeUpdateFunc(const std::string& key, const int32_t userId)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    int32_t value = SUNSET_TIME_DEFAULT;
    manager.GetInt32ValueStrictly(key, value, userId);
    std::lock_guard lock(darkModeStatesMutex_);
    LOGI("dark mode sunset time change, key: %{public}s, userId: %{public}d, from %{public}d to %{public}d",
        key.c_str(), userId, darkModeStates_[userId].settingSunsetTime, value);
    if (value >= darkModeStates_[userId].settingSunriseTime) {
        darkModeStates_[userId].settingSunsetTime = SUNSET_TIME_DEFAULT;
        darkModeStates_[userId].settingSunriseTime = SUNRISE_TIME_DEFAULT;
    } else {
        darkModeStates_[userId].settingSunsetTime = value;
    }
    bool isDarkMode = false;
    OnStateChangeLocked(userId, true, isDarkMode, false, false);
}

void DarkModeManager::SettingDataDarkModeSunriseTimeUpdateFunc(const std::string& key, const int32_t userId)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    int32_t value = SUNRISE_TIME_DEFAULT;
    manager.GetInt32ValueStrictly(key, value, userId);
    std::lock_guard lock(darkModeStatesMutex_);
    LOGI("dark mode sunrise time change, key: %{public}s, userId: %{public}d, from %{public}d to %{public}d",
        key.c_str(), userId, darkModeStates_[userId].settingSunriseTime, value);
    if (value <= darkModeStates_[userId].settingSunsetTime) {
        darkModeStates_[userId].settingSunsetTime = SUNSET_TIME_DEFAULT;
        darkModeStates_[userId].settingSunriseTime = SUNRISE_TIME_DEFAULT;
    } else {
        darkModeStates_[userId].settingSunriseTime = value;
    }
    bool isDarkMode = false;
    OnStateChangeLocked(userId, true, isDarkMode, false, false);
}

ErrCode DarkModeManager::OnStateChangeLocked(const int32_t userId, const bool needUpdateCallback, bool& isDarkMode,
    const bool resetTempColorModeFlag, const bool bootLoadFlag)
{
    ErrCode code = ERR_OK;
    DarkModeState& state = darkModeStates_[userId];
    switch (state.settingMode) {
        case DARK_MODE_ALWAYS_LIGHT:
        case DARK_MODE_ALWAYS_DARK:
            code = OnStateChangeToAllDayMode(
                userId, state.settingMode, needUpdateCallback, isDarkMode, resetTempColorModeFlag, bootLoadFlag);
            break;
        case DARK_MODE_CUSTOM_AUTO:
        case DARK_MODE_SUNRISE_SUNSET:
            code = OnStateChangeToCustomAutoMode(userId, state, needUpdateCallback, isDarkMode, resetTempColorModeFlag,
                bootLoadFlag);
            break;
        default:
            // do nothing
            code = ERR_INVALID_OPERATION;
            break;
    }
    return code;
}

ErrCode DarkModeManager::OnStateChangeToAllDayMode(const int32_t userId, const DarkModeMode darkMode,
    const bool needUpdateCallback, bool& isDarkMode, const bool resetTempColorModeFlag, const bool bootLoadFlag)
{
    alarmTimerManager_.ClearTimerByUserId(userId);
    isDarkMode = darkMode == DARK_MODE_ALWAYS_DARK;
    if (needUpdateCallback) {
        UpdateDarkModeSchedule(darkMode, userId, resetTempColorModeFlag, bootLoadFlag);
    }
    return ERR_OK;
}

ErrCode DarkModeManager::OnStateChangeToCustomAutoMode(const int32_t userId, const DarkModeState& state,
    const bool needUpdateCallback, bool& isDarkMode, const bool resetTempColorModeFlag, const bool bootLoadFlag)
{
    int32_t startTime = -1;
    int32_t endTime = -1;
    if (state.settingMode == DARK_MODE_SUNRISE_SUNSET) {
        startTime = state.settingSunsetTime;
        endTime = state.settingSunriseTime;
    } else {
        startTime = state.settingStartTime;
        endTime = state.settingEndTime;
    }

    ErrCode code = CreateOrUpdateTimers(startTime, endTime, userId);
    if (code != ERR_OK) {
        alarmTimerManager_.ClearTimerByUserId(userId);
        return code;
    }
    DarkModeMode mode = DARK_MODE_INVALID;
    if (AlarmTimerManager::IsWithinTimeInterval(startTime, endTime)) {
        isDarkMode = true;
        mode = DARK_MODE_ALWAYS_DARK;
    } else {
        isDarkMode = false;
        mode = DARK_MODE_ALWAYS_LIGHT;
    }

    if (needUpdateCallback) {
        UpdateDarkModeSchedule(mode, userId, resetTempColorModeFlag, bootLoadFlag);
    }
    return ERR_OK;
}

void DarkModeManager::OnChangeDarkMode(const DarkModeMode mode, const int32_t userId)
{
    if (!updateCallback_) {
        LOGE("no update callback, mode: %{public}d, userId: %{public}d", mode, userId);
        return;
    }
    updateCallback_(mode == DARK_MODE_ALWAYS_DARK, userId);
    if (temporaryColorModeMgr_.IsColorModeTemporary(userId)) {
        temporaryColorModeMgr_.SetColorModeNormal(userId);
    }
}

ErrCode DarkModeManager::CreateOrUpdateTimers(int32_t startTime, int32_t endTime, int32_t userId)
{
    auto callbackSetDark = [startTime, endTime, userId]() {
        LOGI("timer callback, startTime: %{public}d, endTime: %{public}d, userId: %{public}d",
            startTime, endTime, userId);
        ErrCode code = GetInstance().CheckTimerCallbackParams(startTime, endTime, userId);
        if (code != ERR_OK) {
            LOGE("timer callback, params check failed: %{public}d", code);
            return;
        }
        GetInstance().UpdateDarkModeSchedule(DARK_MODE_ALWAYS_DARK, userId, false, false);
    };

    auto callbackSetLight = [startTime, endTime, userId]() {
        LOGI("timer callback, startTime: %{public}d, endTime: %{public}d, userId: %{public}d",
            startTime, endTime, userId);
        ErrCode code = GetInstance().CheckTimerCallbackParams(startTime, endTime, userId);
        if (code != ERR_OK) {
            LOGE("timer callback, params check failed: %{public}d", code);
            return;
        }
        GetInstance().UpdateDarkModeSchedule(DARK_MODE_ALWAYS_LIGHT, userId, false, false);
    };

    return alarmTimerManager_.SetScheduleTime(startTime, endTime, userId, callbackSetDark, callbackSetLight);
}

ErrCode DarkModeManager::CheckTimerCallbackParams(const int32_t startTime, const int32_t endTime, const int32_t userId)
{
    std::lock_guard lock(darkModeStatesMutex_);
    DarkModeState& state = darkModeStates_[userId];
    if (state.settingMode == DARK_MODE_CUSTOM_AUTO) {
        if (state.settingStartTime != startTime) {
            LOGE("timer callback, param wrong, startTime: %{public}d, setting: %{public}d",
                startTime, state.settingStartTime);
            return ERR_INVALID_OPERATION;
        }
        if (state.settingEndTime != endTime) {
            LOGE("timer callback, param wrong, endTime: %{public}d, setting: %{public}d",
                endTime, state.settingEndTime);
            return ERR_INVALID_OPERATION;
        }
    } else if (state.settingMode == DARK_MODE_SUNRISE_SUNSET) {
        if (state.settingSunsetTime != startTime) {
            LOGE("timer callback, param wrong, sunsetTime: %{public}d, setting: %{public}d",
                startTime, state.settingSunsetTime);
            return ERR_INVALID_OPERATION;
        }
        if (state.settingSunriseTime != endTime) {
            LOGE("timer callback, param wrong, sunriseTime: %{public}d, setting: %{public}d",
                endTime, state.settingSunriseTime);
            return ERR_INVALID_OPERATION;
        }
    } else {
        LOGE("timer callback, param wrong, setting mode: %{public}d", state.settingMode);
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}
} // namespace OHOS::ArkUi::UiAppearance
