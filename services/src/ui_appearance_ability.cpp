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
        LOGE("AddItem failed, mode = %{public}d", mode);
        return INVALID_ARG;
    }

    auto appManagerInstance = GetAppManagerInstance();
    if (appManagerInstance == nullptr) {
        LOGE("Get app manager proxy failed.");
        return SYS_ERR;
    }

    LOGI("update Configuration start, mode = %{public}d.", mode);
    auto errcode = appManagerInstance->UpdateConfiguration(config);
    if (errcode != 0) {
        auto retVal = appManagerInstance->GetConfiguration(config);
        if (retVal != 0) {
            LOGE("get configuration failed, update error, error is %{public}d.", retVal);
            return SYS_ERR;
        }
        auto colorMode = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        if (colorMode != paramValue) {
            LOGE("update configuration failed, errcode = %{public}d.", errcode);
            return SYS_ERR;
        } else {
            LOGW("uiappearance is different against configuration. Forced to use the configuration, error is "
                "%{public}d.", errcode);
        }
    }
    darkMode_ = mode;

    // persist to file: etc/para/ui_appearance.para
    auto isSetPara = SetParameter(PERSIST_DARKMODE_KEY.c_str(), paramValue.c_str());
    if (isSetPara < 0) {
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
    if (mode != darkMode_) {
        return OnSetDarkMode(mode);
    } else {
        LOGW("current color mode is %{public}d, no need to change!", darkMode_);
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
        LOGE("get parameter failed.");
        return SYS_ERR;
    }
    if (strcmp(valueGet, DARK.c_str()) == 0) {
        LOGI("current color mode is dark.");
        return ALWAYS_DARK;
    } else if (strcmp(valueGet, LIGHT.c_str()) == 0) {
        LOGI("current color mode is light.");
        return ALWAYS_LIGHT;
    }
    return SYS_ERR;
}

int32_t UiAppearanceAbility::GetDarkMode()
{
    auto isCallingPerm = VerifyAccessToken(PERMISSION_UPDATE_CONFIGURATION);
    if (!isCallingPerm) {
        LOGE("permission verification failed");
        return PERMISSION_ERR;
    }
    return darkMode_;
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

void UiAppearanceAbility::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("systemAbilityId = %{public}d added.", systemAbilityId);
    if (systemAbilityId == APP_MGR_SERVICE_ID) {
        auto res = OnSetDarkMode(static_cast<UiAppearanceAbilityInterface::DarkMode>(OnGetDarkMode()));
        if (res < 0) {
            LOGE("set darkmode init error.");
        }
    }
}

void UiAppearanceAbility::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LOGI("systemAbilityId = %{public}d removed.", systemAbilityId);
}
} // namespace ArkUi::UiAppearance
} // namespace OHOS
