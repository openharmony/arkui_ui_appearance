/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ui_appearance_log.h"
#include "parameter_wrap.h"
#include "syspara/parameter.h"

namespace OHOS::ArkUi::UiAppearance {
bool GetParameterWrap(const std::string& paramName, std::string& value, const std::string& defaultValue)
{
    char buf[256] = { 0 };
    auto res = GetParameter(paramName.c_str(), defaultValue.c_str(), buf, sizeof(buf));
    if (res <= 0) {
        LOGE("get parameter %{public}s failed", paramName.c_str());
        return false;
    }
    LOGI("get parameter %{public}s:%{public}s", paramName.c_str(), value.c_str());
    value = buf;
    return true;
}
bool GetParameterWrap(const std::string& paramName, std::string& value)
{
    const auto defaultValue = value;
    return GetParameterWrap(paramName, value, defaultValue);
}
bool SetParameterWrap(const std::string& paramName, const std::string& value)
{
    auto res = SetParameter(paramName.c_str(), value.c_str());
    if (res != 0) {
        LOGE("set parameter %{public}s failed", paramName.c_str());
        return false;
    }
    LOGD("set parameter %{public}s:%{public}s", paramName.c_str(), value.c_str());
    return true;
}
} // namespace OHOS::ArkUi::UiAppearance

