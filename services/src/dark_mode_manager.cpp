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

#include "dark_mode_manager.h"

#include "setting_data_manager.h"
#include "ui_appearance_log.h"

namespace OHOS::ArkUi::UiAppearance {
namespace {
const std::string SETTING_DARK_MODE_MODE = "settings.uiappearance.darkmode_mode";
const std::string SETTING_DARK_MODE_START_TIME = "settings.uiappearance.darkmode_starttime";
const std::string SETTING_DARK_MODE_END_TIME = "settings.uiappearance.darkmode_endtime";
}

DarkModeManager &DarkModeManager::GetInstance()
{
    static DarkModeManager instance;
    return instance;
}

ErrCode DarkModeManager::Initialize(const std::function<void(bool, int32_t)>& updateCallback)
{
    LOGD("");
    LoadSettingDataObserversCallback();
    updateCallback_ = updateCallback;
    return ERR_OK;
}

ErrCode DarkModeManager::LoadUserSettingData(const int32_t userId, const bool needUpdateCallback, bool &isDarkMode)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    int32_t darkMode = DARK_MODE_INVALID;
    manager.GetInt32Value(SETTING_DARK_MODE_MODE, darkMode, userId);
    if (darkMode < DARK_MODE_INVALID || darkMode >= DARK_MODE_SIZE) {
        LOGE("dark mode out of range: %{public}d, userId: %{public}d", darkMode, userId);
        darkMode = DARK_MODE_INVALID;
    }
    int32_t startTime = -1;
    manager.GetInt32Value(SETTING_DARK_MODE_START_TIME, startTime, userId);
    int32_t endTime = -1;
    manager.GetInt32Value(SETTING_DARK_MODE_END_TIME, endTime, userId);

    std::lock_guard lock(darkModeStatesMutex_);
    DarkModeState& state = darkModeStates_[userId];
    state.settingMode = static_cast<DarkModeMode>(darkMode);
    state.settingStartTime = startTime;
    state.settingEndTime = endTime;
    LOGI("load user setting data, userId: %{public}d, mode: %{public}d, start: %{public}d, end : %{public}d",
        userId, darkMode, startTime, endTime);
    return OnStateChangeLocked(userId, needUpdateCallback, isDarkMode);
}

void DarkModeManager::NotifyDarkModeUpdate(int32_t userId, bool isDarkMode)
{
    std::lock_guard lock(darkModeStatesMutex_);
    const DarkModeState& state = darkModeStates_[userId];
    if (isDarkMode && state.settingMode == DARK_MODE_ALWAYS_LIGHT) {
        LOGI("notify change to always dark, userId: %{public}d", userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        manager.SetStringValue(SETTING_DARK_MODE_MODE, std::to_string(DARK_MODE_ALWAYS_DARK), userId);
    } else if (!isDarkMode && state.settingMode == DARK_MODE_ALWAYS_DARK) {
        LOGI("notify change to always light, userId: %{public}d", userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        manager.SetStringValue(SETTING_DARK_MODE_MODE, std::to_string(DARK_MODE_ALWAYS_LIGHT), userId);
    } else {
        LOGD("no need to change, userId: %{public}d", userId);
    }
}

ErrCode DarkModeManager::OnSwitchUser(const int32_t userId)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    if (manager.IsInitialized() == false) {
        ErrCode code = manager.Initialize();
        if (code != ERR_OK || manager.IsInitialized() == false) {
            LOGE("setting data manager is not initialized");
            return ERR_NO_INIT;
        }
    }

    std::lock_guard lock(settingDataObserversMutex_);
    if (settingDataObserversUserId_ != INVALID_USER_ID) {
        LOGI("clear timers and unregister observers for userId: %{public}d", settingDataObserversUserId_);
        alarmTimerManager_.ClearTimerByUserId(settingDataObserversUserId_);
        UnregisterSettingDataObserversLocked();
        settingDataObserversUserId_ = INVALID_USER_ID;
    }

    ErrCode code = RegisterSettingDataObserversLocked(userId);
    settingDataObserversUserId_ = userId;
    return code;
}

ErrCode DarkModeManager::RestartTimer()
{
    return alarmTimerManager_.RestartTimerByUserId();
}

void DarkModeManager::Dump()
{
    std::lock_guard observersGuard(settingDataObserversMutex_);
    LOGD("settingData observers size: %{public}zu, userId: %{public}d, isAllReg: %{public}d",
        settingDataObservers_.size(), settingDataObserversUserId_, settingDataObserversAllRegistered_);

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
}

ErrCode DarkModeManager::RegisterSettingDataObserversLocked(const int32_t userId)
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
        settingDataObserversAllRegistered_ = false;
        return ERR_NO_INIT;
    }
    LOGD("setting data observers are all initialized");
    settingDataObserversAllRegistered_ = true;
    return ERR_OK;
}

ErrCode DarkModeManager::UnregisterSettingDataObserversLocked()
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    for (const auto& observer : settingDataObservers_) {
        manager.UnregisterObserver(observer.first, settingDataObserversUserId_);
    }
    settingDataObserversAllRegistered_ = false;
    return ERR_OK;
}

void DarkModeManager::SettingDataDarkModeModeUpdateFunc(const std::string& key, const int32_t userId)
{
    LOGD("");
    SettingDataManager& manager = SettingDataManager::GetInstance();
    int32_t value = DARK_MODE_INVALID;
    ErrCode code = manager.GetInt32Value(key, value, userId);
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
    OnStateChangeLocked(userId, true, isDarkMode);
}

void DarkModeManager::SettingDataDarkModeStartTimeUpdateFunc(const std::string& key, const int32_t userId)
{
    LOGD("");
    SettingDataManager& manager = SettingDataManager::GetInstance();
    int32_t value = -1;
    manager.GetInt32Value(key, value, userId);
    std::lock_guard lock(darkModeStatesMutex_);
    LOGI("dark mode start time change, key: %{public}s, userId: %{public}d, from %{public}d to %{public}d",
        key.c_str(), userId, darkModeStates_[userId].settingStartTime, value);
    darkModeStates_[userId].settingStartTime = value;
    bool isDarkMode = false;
    OnStateChangeLocked(userId, true, isDarkMode);
}

void DarkModeManager::SettingDataDarkModeEndTimeUpdateFunc(const std::string& key, const int32_t userId)
{
    LOGD("");
    SettingDataManager& manager = SettingDataManager::GetInstance();
    int32_t value = -1;
    manager.GetInt32Value(key, value, userId);
    std::lock_guard lock(darkModeStatesMutex_);
    LOGI("dark mode end time change, key: %{public}s, userId: %{public}d, from %{public}d to %{public}d",
        key.c_str(), userId, darkModeStates_[userId].settingEndTime, value);
    darkModeStates_[userId].settingEndTime = value;
    bool isDarkMode = false;
    OnStateChangeLocked(userId, true, isDarkMode);
}

ErrCode DarkModeManager::OnStateChangeLocked(const int32_t userId, const bool needUpdateCallback, bool& isDarkMode)
{
    ErrCode code = ERR_OK;
    DarkModeState& state = darkModeStates_[userId];
    switch (state.settingMode) {
        case DARK_MODE_ALWAYS_LIGHT:
        case DARK_MODE_ALWAYS_DARK:
            code = OnStateChangeToAllDayMode(userId, state.settingMode, needUpdateCallback, isDarkMode);
            break;
        case DARK_MODE_CUSTOM_AUTO:
            code = OnStateChangeToCustomAutoMode(userId, state, needUpdateCallback, isDarkMode);
            break;
        default:
            // do nothing
            code = ERR_INVALID_OPERATION;
            break;
    }
    return code;
}

ErrCode DarkModeManager::OnStateChangeToAllDayMode(
    const int32_t userId, const DarkModeMode darkMode, const bool needUpdateCallback, bool &isDarkMode)
{
    alarmTimerManager_.ClearTimerByUserId(userId);
    isDarkMode = darkMode == DARK_MODE_ALWAYS_DARK;
    if (needUpdateCallback) {
        OnChangeDarkMode(darkMode, userId);
    }
    return ERR_OK;
}

ErrCode DarkModeManager::OnStateChangeToCustomAutoMode(
    const int32_t userId, const DarkModeState& state, const bool needUpdateCallback, bool &isDarkMode)
{
    ErrCode code = CreateOrUpdateTimers(state.settingStartTime, state.settingEndTime, userId);
    if (code != ERR_OK) {
        return code;
    }
    DarkModeMode mode = DARK_MODE_INVALID;
    if (AlarmTimerManager::IsWithinTimeInterval(state.settingStartTime, state.settingEndTime)) {
        isDarkMode = true;
        mode = DARK_MODE_ALWAYS_DARK;
    } else {
        isDarkMode = false;
        mode = DARK_MODE_ALWAYS_LIGHT;
    }

    if (needUpdateCallback) {
        OnChangeDarkMode(mode, userId);
    }
    return ERR_OK;
}

void DarkModeManager::OnChangeDarkMode(const DarkModeMode mode, const int32_t userId) const
{
    if (!updateCallback_) {
        LOGE("no update callback, mode: %{public}d, userId: %{public}d", mode, userId);
        return;
    }
    updateCallback_(mode == DARK_MODE_ALWAYS_DARK, userId);
}

ErrCode DarkModeManager::CreateOrUpdateTimers(int32_t startTime, int32_t endTime, int32_t userId)
{
    auto callback = [this, startTime, endTime, userId]() {
        LOGI("timer callback, startTime: %{public}d, endTime: %{public}d, userId: %{public}d",
            startTime, endTime, userId);
        ErrCode code = CheckTimerCallbackParams(startTime, endTime, userId);
        if (code != ERR_OK) {
            LOGE("timer callback, params check failed: %{public}d", code);
            return;
        }
        if (AlarmTimerManager::IsWithinTimeInterval(startTime, endTime)) {
            OnChangeDarkMode(DARK_MODE_ALWAYS_DARK, userId);
        } else {
            OnChangeDarkMode(DARK_MODE_ALWAYS_LIGHT, userId);
        }
    };
    return alarmTimerManager_.SetScheduleTime(startTime, endTime, userId, callback, callback);
}

ErrCode DarkModeManager::CheckTimerCallbackParams(const int32_t startTime, const int32_t endTime, const int32_t userId)
{
    std::lock_guard lock(darkModeStatesMutex_);
    DarkModeState& state = darkModeStates_[userId];
    if (state.settingMode != DARK_MODE_CUSTOM_AUTO) {
        LOGE("timer callback, param wrong, setting mode: %{public}d", state.settingMode);
        return ERR_INVALID_OPERATION;
    }
    if (state.settingStartTime != startTime) {
        LOGE("timer callback, param wrong, startTime: %{public}d, setting: %{public}d",
            startTime, state.settingStartTime);
        return ERR_INVALID_OPERATION;
    }
    if (state.settingEndTime != endTime) {
        LOGE("timer callback, param wrong, endTime: %{public}d, setting: %{public}d", endTime, state.settingEndTime);
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}
} // namespace OHOS::ArkUi::UiAppearance
