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

#ifndef UI_APPEARANCE_ABILITY_H
#define UI_APPEARANCE_ABILITY_H

#include <cstdint>
#include <string>
#include "system_ability.h"
#include "appmgr/app_mgr_proxy.h"
#include "ui_appearance_ability_stub.h"

namespace OHOS {
namespace ArkUi::UiAppearance {
class UiAppearanceAbility : public SystemAbility, public UiAppearanceAbilityStub {
    DECLARE_SYSTEM_ABILITY(UiAppearanceAbility);

public:
    UiAppearanceAbility(int32_t saId, bool runOnCreate);
    ~UiAppearanceAbility() = default;

    int32_t SetDarkMode(DarkMode mode) override;
    int32_t GetDarkMode() override;
    int32_t GetFontScale(std::string &fontScale) override;
    int32_t SetFontScale(std::string &fontScale) override;
    int32_t GetFontWeightScale(std::string &fontWeightScale) override;
    int32_t SetFontWeightScale(std::string &fontWeightScale) override;

protected:
    void OnStart() override;
    void OnStop() override;

    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

private:
    sptr<AppExecFwk::IAppMgr> GetAppManagerInstance();
    bool VerifyAccessToken(const std::string& permissionName);
    int32_t OnSetDarkMode(DarkMode mode);
    int32_t OnGetDarkMode();
    int32_t OnGetFontScale(std::string &fontScale);
    int32_t OnSetFontScale(std::string &fontScale);
    int32_t OnGetFontWeightScale(std::string &fontWeightScale);
    int32_t OnSetFontWeightScale(std::string &fontWeightScale);

    DarkMode darkMode_ = DarkMode::ALWAYS_LIGHT;
    std::string fontScale_ = "1";
    std::string fontWeightScale_ = "1";
};
} // namespace ArkUi::UiAppearance
} // namespace OHOS
#endif // UI_APPEARANCE_ABILITY_H