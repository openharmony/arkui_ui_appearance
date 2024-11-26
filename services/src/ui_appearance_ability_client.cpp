/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "ui_appearance_ability_client.h"

#include <string>
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ui_appearance_ability_proxy.h"
#include "ui_appearance_log.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

namespace OHOS {
namespace ArkUi::UiAppearance {
sptr<UiAppearanceAbilityClient> UiAppearanceAbilityClient::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (!instance_) {
            uiAppearanceServiceProxy_ = CreateUiAppearanceServiceProxy();
            instance_ = new UiAppearanceAbilityClient;
        }
    }
    return instance_;
}

sptr<UiAppearanceAbilityInterface> UiAppearanceAbilityClient::GetUiAppearanceServiceProxy()
{
    if (uiAppearanceServiceProxy_ == nullptr) {
        LOGE("Redo CreateUiAppearanceServiceProxy");
        uiAppearanceServiceProxy_ = CreateUiAppearanceServiceProxy();
    }
    return uiAppearanceServiceProxy_;
}

int32_t UiAppearanceAbilityClient::SetDarkMode(UiAppearanceAbilityInterface::DarkMode mode)
{
    if (!GetUiAppearanceServiceProxy()) {
        LOGE("SetDarkMode quit because redoing CreateUiAppearanceServiceProxy failed.");
        return UiAppearanceAbilityInterface::ErrCode::SYS_ERR;
    }
    return uiAppearanceServiceProxy_->SetDarkMode(mode);
}

int32_t UiAppearanceAbilityClient::GetDarkMode()
{
    if (!GetUiAppearanceServiceProxy()) {
        LOGE("GetDarkMode quit because redoing CreateUiAppearanceServiceProxy failed.");
        return UiAppearanceAbilityInterface::ErrCode::SYS_ERR;
    }
    return uiAppearanceServiceProxy_->GetDarkMode();
}

int32_t UiAppearanceAbilityClient::SetFontScale(std::string &fontScale)
{
    if (!GetUiAppearanceServiceProxy()) {
        LOGE("SetDarkMode quit because redoing CreateUiAppearanceServiceProxy failed.");
        return UiAppearanceAbilityInterface::ErrCode::SYS_ERR;
    }
    return uiAppearanceServiceProxy_->SetFontScale(fontScale);
}

int32_t UiAppearanceAbilityClient::GetFontScale(std::string &fontScale)
{
    if (!GetUiAppearanceServiceProxy()) {
        LOGE("GetDarkMode quit because redoing CreateUiAppearanceServiceProxy failed.");
        return UiAppearanceAbilityInterface::ErrCode::SYS_ERR;
    }
    int id = HiviewDFX::XCollie::GetInstance().SetTimer(
        "GetFontScale", 10, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
    auto res = uiAppearanceServiceProxy_->GetFontScale(fontScale);
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return res;
}

int32_t UiAppearanceAbilityClient::SetFontWeightScale(std::string &fontWeightScale)
{
    if (!GetUiAppearanceServiceProxy()) {
        LOGE("SetDarkMode quit because redoing CreateUiAppearanceServiceProxy failed.");
        return UiAppearanceAbilityInterface::ErrCode::SYS_ERR;
    }
    return uiAppearanceServiceProxy_->SetFontWeightScale(fontWeightScale);
}

int32_t UiAppearanceAbilityClient::GetFontWeightScale(std::string &fontWeightScale)
{
    if (!GetUiAppearanceServiceProxy()) {
        LOGE("GetDarkMode quit because redoing CreateUiAppearanceServiceProxy failed.");
        return UiAppearanceAbilityInterface::ErrCode::SYS_ERR;
    }
    return uiAppearanceServiceProxy_->GetFontWeightScale(fontWeightScale);
}

sptr<UiAppearanceAbilityInterface> UiAppearanceAbilityClient::CreateUiAppearanceServiceProxy()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        LOGE("Get SystemAbilityManager failed.");
        return nullptr;
    }

    sptr<IRemoteObject> systemAbility = systemAbilityManager->GetSystemAbility(ARKUI_UI_APPEARANCE_SERVICE_ID);
    if (systemAbility == nullptr) {
        LOGE("Get SystemAbility failed.");
        return nullptr;
    }

    deathRecipient_ = new UiAppearanceDeathRecipient;
    systemAbility->AddDeathRecipient(deathRecipient_);
    sptr<UiAppearanceAbilityInterface> uiAppearanceServiceProxy =
        iface_cast<UiAppearanceAbilityInterface>(systemAbility);
    if (uiAppearanceServiceProxy == nullptr) {
        LOGE("Get uiAppearanceServiceProxy from SA failed.");
        return nullptr;
    }
    LOGI("Get uiAppearanceServiceProxy successful.");
    return uiAppearanceServiceProxy;
}

void UiAppearanceAbilityClient::OnRemoteSaDied(const wptr<IRemoteObject>& remote)
{
    // Used for new connections after the service may be disconnected.
    uiAppearanceServiceProxy_ = CreateUiAppearanceServiceProxy();
}

void UiAppearanceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& object)
{
    LOGI("UiAppearanceDeathRecipient on remote systemAbility died.");
    UiAppearanceAbilityClient::GetInstance()->OnRemoteSaDied(object);
}
} // namespace ArkUi::UiAppearance
} // namespace OHOS
