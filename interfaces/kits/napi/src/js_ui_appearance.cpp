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

#include "js_ui_appearance.h"

#include "ui_appearance_log.h"

namespace OHOS {
namespace ArkUi::UiAppearance {
void JsDarkMode::OnExecute(napi_env env, void* data)
{
    JS_HILOG_INFO("OnExecute begin.");
    AsyncContext* asyncContext = static_cast<AsyncContext*>(data);
    if (asyncContext->isArgsValid) {
        auto res = UiAppearanceAbilityClient::GetInstance()->SetDarkMode(asyncContext->mode);
        asyncContext->status = res;
    }
}

void JsDarkMode::OnComplete(napi_env env, napi_status status, void* data)
{
    JS_HILOG_INFO("OnComplete begin.");
    AsyncContext* asyncContext = static_cast<AsyncContext*>(data);
    napi_value result[EnumNapiResults::BUTT] = { 0 };

    // set result
    if (asyncContext->isArgsValid && asyncContext->status == UiAppearanceAbilityInterface::ErrCode::SUCCEEDED) {
        napi_get_undefined(env, &result[EnumNapiResults::ERROR]);
        napi_get_undefined(env, &result[EnumNapiResults::COMMON]);
    } else {
        std::string err;
        napi_value message = nullptr;
        if (!asyncContext->isArgsValid || asyncContext->status == UiAppearanceAbilityInterface::ErrCode::INVALID_ARG) {
            err.assign("invalid argument.");
        } else if (asyncContext->status == UiAppearanceAbilityInterface::ErrCode::PERMISSION_ERR) {
            err.assign("permission verification failed.");
        } else {
            err.assign("system error.");
        }
        napi_create_string_utf8(env, err.c_str(), NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, nullptr, message, &result[EnumNapiResults::ERROR]);
        napi_get_undefined(env, &result[EnumNapiResults::COMMON]);
    }

    if (asyncContext->deferred) { // promise
        if (asyncContext->isArgsValid && asyncContext->status == UiAppearanceAbilityInterface::ErrCode::SUCCEEDED) {
            napi_resolve_deferred(env, asyncContext->deferred, result[EnumNapiResults::COMMON]);
        } else {
            napi_reject_deferred(env, asyncContext->deferred, result[EnumNapiResults::ERROR]);
        }
    } else { // AsyncCallback
        JS_HILOG_INFO("napi call function start.");
        napi_value callback = nullptr;
        napi_value returnValue = nullptr;
        auto errCode =
            (asyncContext->isArgsValid) ? asyncContext->status : UiAppearanceAbilityInterface::ErrCode::INVALID_ARG;
        napi_create_int32(env, errCode, result);
        napi_get_reference_value(env, asyncContext->callbackRef, &callback);
        napi_call_function(env, nullptr, callback, EnumNapiResults::BUTT, result, &returnValue);
        napi_delete_reference(env, asyncContext->callbackRef);
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
}

napi_status JsDarkMode::CheckArgs(napi_env env, size_t argc, napi_value* argv)
{
    if (argc < 1 || argc > 2) { // 1 and 2 is the number of arguments
        return napi_invalid_arg;
    }

    napi_valuetype valueType = napi_undefined;
    switch (argc) {
        case 2: // 2 is the number of arguments
            napi_typeof(env, argv[1], &valueType);
            if (valueType != napi_function) {
                return napi_invalid_arg;
            }
            [[fallthrough]];
        case 1:
            napi_typeof(env, argv[0], &valueType);
            if (valueType != napi_number) {
                return napi_invalid_arg;
            }
            break;
        default:
            return napi_invalid_arg;
    }
    return napi_ok;
}

UiAppearanceAbilityInterface::DarkMode JsDarkMode::ConvertJsDarkMode2Enum(int32_t jsVal)
{
    switch (jsVal) {
        case 0:
            return UiAppearanceAbilityInterface::DarkMode::ALWAYS_DARK;
        case 1:
            return UiAppearanceAbilityInterface::DarkMode::ALWAYS_LIGHT;
        default:
            return UiAppearanceAbilityInterface::DarkMode::UNKNOWN;
    }
}

static napi_value JSSetDarkMode(napi_env env, napi_callback_info info)
{
    JS_HILOG_INFO("JSSetDarkMode begin.");

    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_status napiStatus = napi_ok;
    do {
        napiStatus = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
        if (napiStatus != napi_ok) {
            JS_HILOG_ERROR("get callback info failed.");
            break;
        }
        napiStatus = JsDarkMode::CheckArgs(env, argc, argv);
        if (napiStatus != napi_ok) {
            JS_HILOG_ERROR("invalid argument.");
            break;
        }
    } while (0);

    auto asyncContext = new (std::nothrow) AsyncContext();
    if (asyncContext == nullptr) {
        JS_HILOG_ERROR("new AsyncContext failed.");
        napi_value result = nullptr;
        napi_get_undefined(env, &result);
        return result;
    }

    asyncContext->isArgsValid = (napiStatus == napi_ok);
    if (asyncContext->isArgsValid) {
        napi_get_value_int32(env, argv[0], &asyncContext->jsSetArg);
        asyncContext->mode = JsDarkMode::ConvertJsDarkMode2Enum(asyncContext->jsSetArg);
    }
    if (argc == 2) { // 2: the number of arguments
        napi_create_reference(env, argv[1], 1, &asyncContext->callbackRef);
    }

    napi_value result = nullptr;
    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "JSSetDarkMode", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(env, nullptr, resource, JsDarkMode::OnExecute, JsDarkMode::OnComplete,
        reinterpret_cast<void*>(asyncContext), &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);

    return result;
}

static napi_value JSGetDarkMode(napi_env env, napi_callback_info info)
{
    JS_HILOG_INFO("JSGetDarkMode begin.");

    size_t argc = 0;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr));
    NAPI_ASSERT(env, argc == 0, "requires no parameter");

    UiAppearanceAbilityInterface::DarkMode mode = UiAppearanceAbilityClient::GetInstance()->GetDarkMode();
    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, mode, &result));
    return result;
}

EXTERN_C_START
static napi_value UiAppearanceExports(napi_env env, napi_value exports)
{
    napi_value DarkMode = nullptr;
    napi_value alwaysDark = nullptr;
    napi_value alwaysLight = nullptr;
    NAPI_CALL(env, napi_create_int32(env, 0, &alwaysDark));
    NAPI_CALL(env, napi_create_int32(env, 1, &alwaysLight));
    NAPI_CALL(env, napi_create_object(env, &DarkMode));
    NAPI_CALL(env, napi_set_named_property(env, DarkMode, "ALWAYS_DARK", alwaysDark));
    NAPI_CALL(env, napi_set_named_property(env, DarkMode, "ALWAYS_LIGHT", alwaysLight));
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("setDarkMode", JSSetDarkMode),
        DECLARE_NAPI_FUNCTION("getDarkMode", JSGetDarkMode),
        DECLARE_NAPI_STATIC_PROPERTY("DarkMode", DarkMode),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));
    return exports;
}
EXTERN_C_END

static napi_module ui_appearance_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = UiAppearanceExports,
    .nm_modname = "uiAppearance", // relative to the module name while import.
    .nm_priv = nullptr,
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void UiAppearanceRegister()
{
    napi_module_register(&ui_appearance_module);
}
} // namespace ArkUi::UiAppearance
} // namespace OHOS
