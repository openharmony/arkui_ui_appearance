/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ui_appearance_ability.h"

#include <string>

#include "accesstoken_kit.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "global_configuration_key.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "matching_skills.h"
#include "os_account_manager.h"
#include "syspara/parameter.h"
#include "system_ability_definition.h"
#include "ui_appearance_log.h"

namespace {
static const std::string LIGHT = "light";
static const std::string DARK = "dark";
static const std::string BASE_SCALE = "1";
static const std::string PERSIST_DARKMODE_KEY = "persist.ace.darkmode";
static const std::string PERMISSION_UPDATE_CONFIGURATION = "ohos.permission.UPDATE_CONFIGURATION";
// current default accountId = 0, will change when have more user.
static const std::string FONT_SCAL_FOR_USER0 = "persist.sys.font_scale_for_user0";
static const std::string FONT_Weight_SCAL_FOR_USER0 = "persist.sys.font_wght_scale_for_user0";

static const std::string PERSIST_DARKMODE_KEY_FOR_NONE = "persist.ace.darkmode.";
static const std::string FONT_SCAL_FOR_NONE = "persist.sys.font_scale_for_user.";
static const std::string FONT_WEIGHT_SCAL_FOR_NONE = "persist.sys.font_wght_scale_for_user.";

static const std::string FIRST_INITIALIZATION = "persist.uiAppearance.first_initialization";
const static int32_t USER100 = 100;
const static std::string FIRST_UPGRADE = "1";
const static std::string NOT_FIRST_UPGRADE = "0";
} // namespace

namespace OHOS {
namespace ArkUi::UiAppearance {

UserSwitchEventSubscriber::UserSwitchEventSubscriber(const EventFwk::CommonEventSubscribeInfo& subscriberInfo,
    const std::function<void(const int32_t)>& userSwitchCallback)
    : EventFwk::CommonEventSubscriber(subscriberInfo), userSwitchCallback_(userSwitchCallback)
{}

void UserSwitchEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData& data)
{
    const AAFwk::Want& want = data.GetWant();
    std::string action = want.GetAction();
    LOGI("action:%{public}s", action.c_str());

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        if (userSwitchCallback_ != nullptr) {
            userSwitchCallback_(data.GetCode());
        }
    }
}

REGISTER_SYSTEM_ABILITY_BY_ID(UiAppearanceAbility, ARKUI_UI_APPEARANCE_SERVICE_ID, true);

UiAppearanceAbility::UiAppearanceAbility(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate) {}

sptr<AppExecFwk::IAppMgr> UiAppearanceAbility::GetAppManagerInstance()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        LOGE("Getting systemAbilityManager failed.");
        return nullptr;
    }

    sptr<IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (appObject == nullptr) {
        LOGE("Get systemAbility failed.");
        return nullptr;
    }

    sptr<AppExecFwk::IAppMgr> systemAbility = iface_cast<AppExecFwk::IAppMgr>(appObject);
    if (systemAbility == nullptr) {
        LOGE("Get AppMgrProxy from SA failed.");
        return nullptr;
    }
    return systemAbility;
}

bool UiAppearanceAbility::VerifyAccessToken(const std::string& permissionName)
{
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (ret == Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        return true;
    }
    LOGE("permission %{private}s denied, callerToken : %{public}u", permissionName.c_str(), callerToken);
    return false;
}

void UiAppearanceAbility::OnStart()
{
    bool res = Publish(this); // SA registers with SAMGR
    if (!res) {
        LOGE("publish failed.");
        return;
    }

    LOGI("AddSystemAbilityListener start.");
    AddSystemAbilityListener(APP_MGR_SERVICE_ID);
    return;
}

void UiAppearanceAbility::OnStop()
{
    LOGI("UiAppearanceAbility SA stop.");
}

std::vector<int32_t> UiAppearanceAbility::GetUserIds()
{
    std::vector<AccountSA::OsAccountInfo> infos;
    AccountSA::OsAccountManager::QueryAllCreatedOsAccounts(infos);
    std::vector<int32_t> ids;
    for (const auto& info : infos) {
        ids.push_back(info.GetLocalId());
    }
    return ids;
}

void UiAppearanceAbility::DoCompatibleProcess()
{
    LOGI("DoCompatibleProcess");
    auto getOldParam = [this](const std::string& paramName, std::string& result) {
        return GetParameterWrap(paramName, result);
    };

    auto isParamAllreadaySetted = [this](const std::string& paramName) {
        std::string value;
        return GetParameterWrap(paramName, value);
    };

    const std::vector<int32_t> userIds = GetUserIds();
    std::unique_lock<std::recursive_mutex> guard(usersParamMutex_);
    std::string darkMode = LIGHT;
    if (getOldParam(PERSIST_DARKMODE_KEY, darkMode)) {
        for (auto id : userIds) {
            if (isParamAllreadaySetted(DarkModeParamAssignUser(id))) {
                continue;
            }
            SetParameterWrap(DarkModeParamAssignUser(id), darkMode);
        }
        LOGD(" set darkMode %{public}s", darkMode.c_str());
    }
    std::string fontSize = BASE_SCALE;
    if (getOldParam(FONT_SCAL_FOR_USER0, fontSize)) {
        for (auto id : userIds) {
            if (isParamAllreadaySetted(FontScaleParamAssignUser(id))) {
                continue;
            }
            SetParameterWrap(FontScaleParamAssignUser(id), fontSize);
        }
        LOGD(" set fontSize %{public}s", fontSize.c_str());
    }
    std::string fontWeightSize = BASE_SCALE;
    if (getOldParam(FONT_Weight_SCAL_FOR_USER0, fontWeightSize)) {
        for (auto id : userIds) {
            if (isParamAllreadaySetted(FontWeightScaleParamAssignUser(id))) {
                continue;
            }
            SetParameterWrap(FontWeightScaleParamAssignUser(id), fontWeightSize);
        }
        LOGD(" set fontWeightSize %{public}s", fontWeightSize.c_str());
    }
    SetParameterWrap(FIRST_INITIALIZATION, "0");
    isNeedDoCompatibleProcess_ = false;
}

void UiAppearanceAbility::DoInitProcess()
{
    LOGI("DoInitProcess");
    const std::vector<int32_t> userIds = GetUserIds();
    for (auto userId : userIds) {
        std::string darkValue = LIGHT;
        GetParameterWrap(DarkModeParamAssignUser(userId), darkValue);

        std::string fontSize = BASE_SCALE;
        GetParameterWrap(FontScaleParamAssignUser(userId), fontSize);

        std::string fontWeight = BASE_SCALE;
        GetParameterWrap(FontWeightScaleParamAssignUser(userId), fontWeight);

        UiAppearanceParam tmpParam;
        tmpParam.darkMode = darkValue == DARK ? DarkMode::ALWAYS_DARK : DarkMode::ALWAYS_LIGHT;
        tmpParam.fontScale = fontSize;
        tmpParam.fontWeightScale = fontWeight;
        usersParam_[userId] = tmpParam;
    }
    isInitializationFinished_ = true;
}

void UiAppearanceAbility::UpdateCurrentUserConfiguration(const int32_t userId)
{
    UiAppearanceParam tmpParam = usersParam_[userId];
    AppExecFwk::Configuration config;
    config.AddItem(
        AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, tmpParam.darkMode == DarkMode::ALWAYS_DARK ? DARK : LIGHT);
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE, tmpParam.fontScale);
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE, tmpParam.fontWeightScale);

    auto appManagerInstance = GetAppManagerInstance();
    if (appManagerInstance != nullptr) {
        appManagerInstance->UpdateConfiguration(config, userId);
        SetParameterWrap(PERSIST_DARKMODE_KEY, tmpParam.darkMode == DarkMode::ALWAYS_DARK ? DARK : LIGHT);
        SetParameterWrap(FONT_SCAL_FOR_USER0, tmpParam.fontScale);
        SetParameterWrap(FONT_Weight_SCAL_FOR_USER0, tmpParam.fontWeightScale);
        LOGD("user switch update config %{public}s", config.GetName().c_str());
    }
}

void UiAppearanceAbility::UserSwitchFunc(const int32_t userId)
{
    std::unique_lock<std::recursive_mutex> guard(usersParamMutex_);
    if (isNeedDoCompatibleProcess_) {
        DoCompatibleProcess();
    }
    if (!isInitializationFinished_) {
        DoInitProcess();
    }
    UpdateCurrentUserConfiguration(userId);
}

void UiAppearanceAbility::SubscribeUserSwitchEvent()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);

    userSwitchSubscriber_ = std::make_shared<UserSwitchEventSubscriber>(
        subscribeInfo, [this](const int32_t userId) { UserSwitchFunc(userId); });
    bool subResult = EventFwk::CommonEventManager::SubscribeCommonEvent(userSwitchSubscriber_);
    if (!subResult) {
        LOGW("subscribe user switch event error");
    }
}

void UiAppearanceAbility::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId != APP_MGR_SERVICE_ID) {
        return;
    }

    auto checkIfFirstUpgrade = [this]() {
        std::string initFlag = NOT_FIRST_UPGRADE;
        GetParameterWrap(FIRST_INITIALIZATION, initFlag);
        if (initFlag == FIRST_UPGRADE) {
            return true;
        }
        return false;
    };
    isNeedDoCompatibleProcess_ = checkIfFirstUpgrade();
    SubscribeUserSwitchEvent();
    std::unique_lock<std::recursive_mutex> guard(usersParamMutex_);
    if (isNeedDoCompatibleProcess_ && !GetUserIds().empty()) {
        DoCompatibleProcess();
    }

    if (!isInitializationFinished_ && !GetUserIds().empty()) {
        DoInitProcess();
        int32_t userId = USER100;
        auto errCode = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(userId);
        if (errCode != 0) {
            LOGW("GetForegroundOsAccountLocalId error: %{public}d.", errCode);
            userId = USER100;
        }
        UpdateCurrentUserConfiguration(userId);
    }
}

void UiAppearanceAbility::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("systemAbilityId = %{public}d removed.", systemAbilityId);
}

int32_t UiAppearanceAbility::GetCallingUserId()
{
    const static int32_t UID_TRANSFORM_DIVISOR = 200000;

    LOGD("CallingUid = %{public}d", OHOS::IPCSkeleton::GetCallingUid());
    int32_t userId = OHOS::IPCSkeleton::GetCallingUid() / UID_TRANSFORM_DIVISOR;
    if (userId == 0) {
        auto errNo = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(userId);
        if (errNo != 0) {
            LOGE("CallingUid = %{public}d, GetForegroundOsAccountLocalId error:%{public}d",
                OHOS::IPCSkeleton::GetCallingUid(), errNo);
            userId = USER100;
        }
    }
    return userId;
}

std::string UiAppearanceAbility::DarkModeParamAssignUser(const int32_t userId)
{
    return PERSIST_DARKMODE_KEY_FOR_NONE + std::to_string(userId);
}
std::string UiAppearanceAbility::FontScaleParamAssignUser(const int32_t userId)
{
    return FONT_SCAL_FOR_NONE + std::to_string(userId);
}
std::string UiAppearanceAbility::FontWeightScaleParamAssignUser(const int32_t userId)
{
    return FONT_WEIGHT_SCAL_FOR_NONE + std::to_string(userId);
}
bool UiAppearanceAbility::IsUserExist(const int32_t userId)
{
    std::unique_lock<std::recursive_mutex> guard(usersParamMutex_);
    return usersParam_.find(userId) != usersParam_.end();
}

bool UiAppearanceAbility::GetParameterWrap(
    const std::string& paramName, std::string& value, const std::string& defaultValue)
{
    char buf[256] = { 0 };
    auto res = GetParameter(paramName.c_str(), defaultValue.c_str(), buf, sizeof(buf));
    if (res <= 0) {
        LOGE("get parameter %{public}s failed", paramName.c_str());
        return false;
    }
    LOGD("get parameter %{public}s:%{public}s", paramName.c_str(), value.c_str());
    value = buf;
    return true;
}
bool UiAppearanceAbility::GetParameterWrap(const std::string& paramName, std::string& value)
{
    const auto defaultValue = value;
    return GetParameterWrap(paramName, value, defaultValue);
}
bool UiAppearanceAbility::SetParameterWrap(const std::string& paramName, const std::string& value)
{
    auto res = SetParameter(paramName.c_str(), value.c_str());
    if (res != 0) {
        LOGE("set parameter %{public}s failed", paramName.c_str());
        return false;
    }
    LOGD("set parameter %{public}s:%{public}s", paramName.c_str(), value.c_str());
    return true;
}

bool UiAppearanceAbility::UpdateConfiguration(const AppExecFwk::Configuration& configuration, const int32_t userId)
{
    auto appManagerInstance = GetAppManagerInstance();
    if (appManagerInstance == nullptr) {
        LOGE("Get app manager proxy failed.");
        return false;
    }

    LOGI("update Configuration start,userId:%{public}d config = %{public}s.", userId, configuration.GetName().c_str());
    auto errcode = appManagerInstance->UpdateConfiguration(configuration, userId);
    if (errcode != 0) {
        AppExecFwk::Configuration config;
        auto retVal = appManagerInstance->GetConfiguration(config);
        if (retVal != 0) {
            LOGE("get configuration failed, update error, error is %{public}d.", retVal);
            return false;
        }
        std::vector<std::string> diffVe;
        config.CompareDifferent(diffVe, configuration);

        if (!diffVe.empty()) {
            LOGE("update configuration failed, errcode = %{public}d.", errcode);
            return false;
        } else {
            LOGW("uiappearance is different against configuration. Forced to use the configuration, error is "
                 "%{public}d.",
                errcode);
        }
    }
    return true;
}

int32_t UiAppearanceAbility::OnSetDarkMode(const int32_t userId, DarkMode mode)
{
    bool ret = false;
    std::string paramValue;
    AppExecFwk::Configuration config;
    switch (mode) {
        case ALWAYS_LIGHT: {
            ret = config.AddItem(
                AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, AppExecFwk::ConfigurationInner::COLOR_MODE_LIGHT);
            paramValue.assign(LIGHT);
            break;
        }
        case ALWAYS_DARK: {
            ret = config.AddItem(
                AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, AppExecFwk::ConfigurationInner::COLOR_MODE_DARK);
            paramValue.assign(DARK);
            break;
        }
        default:
            break;
    }
    if (!ret) {
        LOGE("AddItem failed, mode = %{public}d", mode);
        return INVALID_ARG;
    }

    if (!UpdateConfiguration(config, userId)) {
        return SYS_ERR;
    }

    std::unique_lock<std::recursive_mutex> guard(usersParamMutex_);
    if (IsUserExist(userId)) {
        usersParam_[userId].darkMode = mode;
    } else {
        UiAppearanceParam tmpParam;
        tmpParam.darkMode = mode;
        usersParam_[userId] = tmpParam;
    }

    SetParameterWrap(PERSIST_DARKMODE_KEY, paramValue);

    // persist to file: etc/para/ui_appearance.para
    auto isSetPara = SetParameterWrap(DarkModeParamAssignUser(userId), paramValue);
    if (!isSetPara) {
        LOGE("set parameter failed");
        return SYS_ERR;
    }
    return SUCCEEDED;
}

int32_t UiAppearanceAbility::SetDarkMode(DarkMode mode)
{
    // Verify permissions
    auto isCallingPerm = VerifyAccessToken(PERMISSION_UPDATE_CONFIGURATION);
    if (!isCallingPerm) {
        LOGE("permission verification failed");
        return PERMISSION_ERR;
    }
    std::unique_lock<std::recursive_mutex> guard(usersParamMutex_);
    auto userId = GetCallingUserId();
    auto it = usersParam_.find(userId);
    if (it != usersParam_.end()) {
        if (mode != it->second.darkMode) {
            return OnSetDarkMode(userId, mode);
        } else {
            LOGW("current color mode is %{public}d, no need to change", mode);
        }
    } else {
        return OnSetDarkMode(userId, mode);
    }

    return SYS_ERR;
}

UiAppearanceAbility::DarkMode UiAppearanceAbility::InitGetDarkMode(const int32_t userId)
{
    std::string valueGet = LIGHT;

    // LIGHT is the default.
    auto res = GetParameterWrap(DarkModeParamAssignUser(userId), valueGet);
    if (!res) {
        return ALWAYS_LIGHT;
    }
    if (valueGet == DARK) {
        LOGI("current color mode is dark.");
        return ALWAYS_DARK;
    } else if (valueGet == LIGHT) {
        LOGI("current color mode is light.");
        return ALWAYS_LIGHT;
    }
    return ALWAYS_LIGHT;
}

int32_t UiAppearanceAbility::GetDarkMode()
{
    auto isCallingPerm = VerifyAccessToken(PERMISSION_UPDATE_CONFIGURATION);
    if (!isCallingPerm) {
        LOGE("permission verification failed");
        return PERMISSION_ERR;
    }
    std::unique_lock<std::recursive_mutex> guard(usersParamMutex_);
    auto it = usersParam_.find(GetCallingUserId());
    if (it != usersParam_.end()) {
        return it->second.darkMode;
    }
    return DarkMode::ALWAYS_LIGHT;
}

int32_t UiAppearanceAbility::OnSetFontScale(const int32_t userId, std::string& fontScale)
{
    bool ret = false;
    AppExecFwk::Configuration config;
    ret = config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE, fontScale);
    if (!ret) {
        LOGE("AddItem failed, fontScale = %{public}s", fontScale.c_str());
        return INVALID_ARG;
    }
    if (!UpdateConfiguration(config, userId)) {
        return SYS_ERR;
    }

    std::unique_lock<std::recursive_mutex> guard(usersParamMutex_);
    if (IsUserExist(userId)) {
        usersParam_[userId].fontScale = fontScale;
    } else {
        UiAppearanceParam tmpParam;
        tmpParam.fontScale = fontScale;
        usersParam_[userId] = tmpParam;
    }

    SetParameterWrap(FONT_SCAL_FOR_USER0, fontScale);
    // persist to file: etc/para/ui_appearance.para
    auto isSetPara = SetParameterWrap(FontScaleParamAssignUser(userId), fontScale);
    if (!isSetPara) {
        LOGE("set parameter failed");
        return SYS_ERR;
    }
    return SUCCEEDED;
}

int32_t UiAppearanceAbility::SetFontScale(std::string& fontScale)
{
    // Verify permissions
    auto isCallingPerm = VerifyAccessToken(PERMISSION_UPDATE_CONFIGURATION);
    if (!isCallingPerm) {
        LOGE("permission verification failed");
        return PERMISSION_ERR;
    }
    if (!fontScale.empty()) {
        return OnSetFontScale(GetCallingUserId(), fontScale);
    } else {
        LOGE("current fontScale is empty!");
    }
    return SYS_ERR;
}

int32_t UiAppearanceAbility::GetFontScale(std::string& fontScale)
{
    auto isCallingPerm = VerifyAccessToken(PERMISSION_UPDATE_CONFIGURATION);
    if (!isCallingPerm) {
        LOGE("permission verification failed");
        return PERMISSION_ERR;
    }
    std::unique_lock<std::recursive_mutex> guard(usersParamMutex_);
    auto it = usersParam_.find(GetCallingUserId());
    if (it != usersParam_.end()) {
        fontScale = it->second.fontScale;
    } else {
        fontScale = BASE_SCALE;
    }
    LOGD("get font scale :%{public}s", fontScale.c_str());
    return SUCCEEDED;
}

int32_t UiAppearanceAbility::OnSetFontWeightScale(const int32_t userId, std::string& fontWeightScale)
{
    bool ret = false;
    AppExecFwk::Configuration config;
    ret = config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE, fontWeightScale);
    if (!ret) {
        LOGE("AddItem failed, fontWeightScale = %{public}s", fontWeightScale.c_str());
        return INVALID_ARG;
    }

    if (!UpdateConfiguration(config, userId)) {
        return SYS_ERR;
    }
    std::unique_lock<std::recursive_mutex> guard(usersParamMutex_);
    if (IsUserExist(userId)) {
        usersParam_[userId].fontWeightScale = fontWeightScale;
    } else {
        UiAppearanceParam tmpParam;
        tmpParam.fontWeightScale = fontWeightScale;
        usersParam_[userId] = tmpParam;
    }

    SetParameterWrap(FONT_Weight_SCAL_FOR_USER0, fontWeightScale);

    // persist to file: etc/para/ui_appearance.para
    auto isSetPara = SetParameterWrap(FontWeightScaleParamAssignUser(userId), fontWeightScale);
    if (!isSetPara) {
        LOGE("set parameter failed");
        return SYS_ERR;
    }
    return SUCCEEDED;
}

int32_t UiAppearanceAbility::SetFontWeightScale(std::string& fontWeightScale)
{
    // Verify permissions
    auto isCallingPerm = VerifyAccessToken(PERMISSION_UPDATE_CONFIGURATION);
    if (!isCallingPerm) {
        LOGE("permission verification failed");
        return PERMISSION_ERR;
    }
    if (!fontWeightScale.empty()) {
        return OnSetFontWeightScale(GetCallingUserId(), fontWeightScale);
    } else {
        LOGE("current fontWeightScale is empty!");
    }
    return SYS_ERR;
}

int32_t UiAppearanceAbility::GetFontWeightScale(std::string& fontWeightScale)
{
    auto isCallingPerm = VerifyAccessToken(PERMISSION_UPDATE_CONFIGURATION);
    if (!isCallingPerm) {
        LOGE("permission verification failed");
        return PERMISSION_ERR;
    }
    std::unique_lock<std::recursive_mutex> guard(usersParamMutex_);
    auto it = usersParam_.find(GetCallingUserId());
    if (it != usersParam_.end()) {
        fontWeightScale = it->second.fontWeightScale;
    } else {
        fontWeightScale = BASE_SCALE;
    }
    LOGD("get font weight scale :%{public}s", fontWeightScale.c_str());
    return SUCCEEDED;
}

} // namespace ArkUi::UiAppearance
} // namespace OHOS
