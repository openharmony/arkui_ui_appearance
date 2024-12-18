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

#ifndef UI_APPEARANCE_ABILITY_INTERFACE_H
#define UI_APPEARANCE_ABILITY_INTERFACE_H

#include <iremote_broker.h>
#include <string>

namespace OHOS {
namespace ArkUi::UiAppearance {
class UiAppearanceAbilityInterface : public IRemoteBroker {
public:
    enum DarkMode {
        ALWAYS_DARK = 0,
        ALWAYS_LIGHT = 1,
        UNKNOWN = 2,
    };

    enum ErrCode {
        SUCCEEDED = 0,
        PERMISSION_ERR = 201,
        INVALID_ARG = 401,
        SYS_ERR = 500001,
    };

    virtual int32_t SetDarkMode(DarkMode mode) = 0;
    virtual int32_t GetDarkMode() = 0;
    virtual int32_t GetFontScale(std::string &fontScale) = 0;
    virtual int32_t SetFontScale(std::string &fontScale) = 0;
    virtual int32_t GetFontWeightScale(std::string &fontWeightScale) = 0;
    virtual int32_t SetFontWeightScale(std::string &fontWeightScale) = 0;

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.UiAppea.Interface");
};
} // namespace ArkUi::UiAppearance
} // namespace OHOS
#endif // UI_APPEARANCE_ABILITY_INTERFACE_H
