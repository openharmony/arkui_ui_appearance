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

#ifndef UI_APPEARANCE_ABILITY_CLIENT_H
#define UI_APPEARANCE_ABILITY_CLIENT_H

#include <string>
#include "iremote_object.h"
#include "refbase.h"
#include "ui_appearance_ability_interface.h"

namespace OHOS {
namespace ArkUi::UiAppearance {
class UiAppearanceDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit UiAppearanceDeathRecipient() = default;
    ~UiAppearanceDeathRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject>& object) override;
};

class UiAppearanceAbilityClient : public RefBase {
public:
    UiAppearanceAbilityClient() = default;
    ~UiAppearanceAbilityClient() = default;
    static sptr<UiAppearanceAbilityClient> GetInstance();

    int32_t SetDarkMode(UiAppearanceAbilityInterface::DarkMode mode);
    int32_t GetDarkMode();
    int32_t GetFontScale(std::string& fontScale);
    int32_t SetFontScale(std::string& fontScale);
    int32_t GetFontWeightScale(std::string& fontWeightScale);
    int32_t SetFontWeightScale(std::string& fontWeightScale);
    void OnRemoteSaDied(const wptr<IRemoteObject>& object);

private:
    sptr<UiAppearanceAbilityInterface> GetUiAppearanceServiceProxy();
    static sptr<UiAppearanceAbilityInterface> CreateUiAppearanceServiceProxy();

    static inline std::mutex instanceLock_;
    static inline sptr<UiAppearanceAbilityClient> instance_;
    static inline sptr<UiAppearanceAbilityInterface> uiAppearanceServiceProxy_;
    static inline sptr<UiAppearanceDeathRecipient> deathRecipient_;
};
} // namespace ArkUi::UiAppearance
} // namespace OHOS
#endif // UI_APPEARANCE_ABILITY_CLIENT_H