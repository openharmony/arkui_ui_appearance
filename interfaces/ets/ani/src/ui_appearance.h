/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef UI_APPEARANCE_H
#define UI_APPEARANCE_H

#include <ani.h>
#include <array>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <future>
#include <thread>
#include <map>
#include <memory>
#include <string>
#include <ani_signature_builder.h>

#include "ui_appearance_types.h"
#include "ui_appearance_log.h"
namespace OHOS {
namespace ArkUi::UiAppearance {

struct AsyncContext {
    ani_resolver deferred = nullptr;
    ani_object callbackRef = nullptr;
    ani_int ani_SetArg = -1;
    ani_double ani_FontScale = 0;
    ani_double ani_FontWeightScale = 0;
    std::string errMsg;
    UiAppearanceAbilityErrCode status;
    DarkMode mode;
    std::string fontScale;
    std::string fontWeightScale;
};

ani_object GetErrorObject(ani_env *env, const std::string &errMsg, int32_t code);
void AniThrow(ani_env *env, const std::string &errMsg, int32_t code);
void OnComplete([[maybe_unused]] ani_env* env, AsyncContext* asyncContext);
DarkMode ConvertJsDarkMode2Enum(int32_t jsVal);
void SetDarkMode([[maybe_unused]] ani_env* env, ani_enum_item mode, ani_object callbackObj);
ani_object SetDarkModeWithPromise([[maybe_unused]] ani_env* env, ani_enum_item mode);
ani_enum_item GetDarkMode([[maybe_unused]] ani_env* env);
ani_object SetFontScale([[maybe_unused]] ani_env* env, ani_double fontScale);
ani_double GetFontScale([[maybe_unused]] ani_env* env);
ani_object SetFontWeightScale([[maybe_unused]] ani_env* env, ani_double fontWeightScale);
ani_double GetFontWeightScale([[maybe_unused]] ani_env* env);
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result);
} // namespace ArkUi::UiAppearance
} // namespace OHOS
#endif // UI_APPEARANCE_H
