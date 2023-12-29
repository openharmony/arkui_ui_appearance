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
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "syspara/parameter.h"
#include "system_ability_definition.h"
#include "ui_appearance_log.h"

namespace {
static const std::string LIGHT = "light";
static const std::string DARK = "dark";
static const std::string PERSIST_DARKMODE_KEY = "persist.ace.darkmode";
static const std::string PERMISSION_UPDATE_CONFIGURATION = "ohos.permission.UPDATE_CONFIGURATION";
} // namespace

namespace OHOS {
namespace ArkUi::UiAppearance {
REGISTER_SYSTEM_ABILITY_BY_ID(UiAppearanceAbility, ARKUI_UI_APPEARANCE_SERVICE_ID, true);

UiAppearanceAbility::UiAppearanceAbility(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate) {}

sptr<AppExecFwk::IAppMgr> UiAppearanceAbility::GetAppManagerInstance()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        HILOG_ERROR("Getting systemAbilityManager failed.");
        return nullptr;
    }

    sptr<IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (appObject == nullptr) {
        HILOG_ERROR("Get systemAbility failed.");
        return nullptr;
    }

    sptr<AppExecFwk::IAppMgr> systemAbility = iface_cast<AppExecFwk::IAppMgr>(appObject);
    if (systemAbility == nullptr) {
        HILOG_ERROR("Get AppMgrProxy from SA failed.");
        return nullptr;
    }
    return systemAbility;
}

bool UiAppearanceAbility::VerifyAccessToken(const std::string& permissionName)
{
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (ret == Security::AccessToken::PermissionState::PERMISSION_DENIED) {
        HILOG_ERROR("permission %{private}s: PERMISSION_DENIED", permissionName.c_str());
        return false;
    }
    return true;
}

int32_t UiAppearanceAbility::OnSetDarkMode(DarkMode mode)
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
        HILOG_ERROR("AddItem failed, mode = %{public}d", mode);
        return INVALID_ARG;
    }

    auto appManagerInstance = GetAppManagerInstance();
    if (appManagerInstance == nullptr) {
        HILOG_ERROR("Get app manager proxy failed.");
        return SYS_ERR;
    }

    HILOG_INFO("update Configuration start, mode = %{public}d.", mode);
    auto errcode = appManagerInstance->UpdateConfiguration(config);
    if (errcode != 0) {
        HILOG_ERROR("update configuration failed, errcode = %{public}d.", errcode);
        return SYS_ERR;
    }
    darkMode_ = mode;

    // persist to file: etc/para/ui_appearance.para
    auto isSetPara = SetParameter(PERSIST_DARKMODE_KEY.c_str(), paramValue.c_str());
    if (isSetPara < 0) {
        HILOG_ERROR("set parameter failed");
        return SYS_ERR;
    }
    return SUCCEEDED;
}

int32_t UiAppearanceAbility::SetDarkMode(DarkMode mode)
{
    // Verify permissions
    auto isCallingPerm = VerifyAccessToken(PERMISSION_UPDATE_CONFIGURATION);
    if (!isCallingPerm) {
        HILOG_ERROR("permission verification failed");
        return PERMISSION_ERR;
    }
    if (mode != darkMode_) {
        return OnSetDarkMode(mode);
    } else {
        HILOG_WARN("current color mode is %{public}d, no need to change!", darkMode_);
    }
    return SYS_ERR;
}

int32_t UiAppearanceAbility::OnGetDarkMode()
{
    constexpr int buffSize = 64; // buff len: 64
    char valueGet[buffSize] = { 0 };

    // LIGHT is the default.
    auto res = GetParameter(PERSIST_DARKMODE_KEY.c_str(), LIGHT.c_str(), valueGet, buffSize);
    if (res <= 0) {
        HILOG_ERROR("get parameter failed.");
        return SYS_ERR;
    }
    if (strcmp(valueGet, DARK.c_str()) == 0) {
        HILOG_INFO("current color mode is dark.");
        return ALWAYS_DARK;
    } else if (strcmp(valueGet, LIGHT.c_str()) == 0) {
        HILOG_INFO("current color mode is light.");
        return ALWAYS_LIGHT;
    }
    return SYS_ERR;
}

int32_t UiAppearanceAbility::GetDarkMode()
{
    auto isCallingPerm = VerifyAccessToken(PERMISSION_UPDATE_CONFIGURATION);
    if (!isCallingPerm) {
        HILOG_ERROR("permission verification failed");
        return PERMISSION_ERR;
    }
    return darkMode_;
}

void UiAppearanceAbility::OnStart()
{
    bool res = Publish(this); // SA registers with SAMGR
    if (!res) {
        HILOG_ERROR("publish failed.");
        return;
    }

    HILOG_INFO("AddSystemAbilityListener start.");
    AddSystemAbilityListener(APP_MGR_SERVICE_ID);
    return;
}

void UiAppearanceAbility::OnStop()
{
    HILOG_INFO("UiAppearanceAbility SA stop.");
}

void UiAppearanceAbility::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOG_INFO("systemAbilityId = %{public}d added.", systemAbilityId);
    if (systemAbilityId == APP_MGR_SERVICE_ID) {
        auto res = OnSetDarkMode(static_cast<UiAppearanceAbilityInterface::DarkMode>(OnGetDarkMode()));
        if (res < 0) {
            HILOG_ERROR("set darkmode init error.");
        }
    }
}

void UiAppearanceAbility::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOG_INFO("systemAbilityId = %{public}d removed.", systemAbilityId);
}
} // namespace ArkUi::UiAppearance
} // namespace OHOS