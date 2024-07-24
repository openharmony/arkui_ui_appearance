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

#ifndef JS_UI_APPEARANCE_H
#define JS_UI_APPEARANCE_H

#include <string>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "ui_appearance_ability_client.h"

namespace OHOS {
namespace ArkUi::UiAppearance {
struct AsyncContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    int32_t jsSetArg = -1;
    double jsFontScale = 0;
    double jsFontWeightScale = 0;
    std::string errMsg;
    UiAppearanceAbilityInterface::ErrCode status;
    UiAppearanceAbilityInterface::DarkMode mode;
    std::string fontScale;
    std::string fontWeightScale;
};

class JsUiAppearance final {
public:
    static void OnExecute(napi_env env, void* data);
    static void OnComplete(napi_env env, napi_status status, void* data);
    static void OnSetFontScale(napi_env env, void* data);
    static void OnSetFontWeightScale(napi_env env, void* data);
    static napi_status CheckArgs(napi_env env, size_t argc, napi_value* argv);
    static napi_status CheckFontScaleArgs(napi_env env, size_t argc, napi_value* argv);
    static UiAppearanceAbilityInterface::DarkMode ConvertJsDarkMode2Enum(int32_t jsVal);
};

napi_value JSSetDarkModeSync(napi_env env, napi_callback_info info);
napi_value JSGetDarkModeSync(napi_env env, napi_callback_info info);
napi_value JSSetFontScaleSync(napi_env env, napi_callback_info info);
napi_value JSGetFontScaleSync(napi_env env, napi_callback_info info);
napi_value JSSetFontWeightScaleSync(napi_env env, napi_callback_info info);
napi_value JSGetFontWeightScaleSync(napi_env env, napi_callback_info info);
} // namespace ArkUi::UiAppearance
} // namespace OHOS
#endif //  JS_UI_APPEARANCE_H