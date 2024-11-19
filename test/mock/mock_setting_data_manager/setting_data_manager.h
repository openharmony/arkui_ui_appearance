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

#ifndef UI_APPEARANCE_MOCK_UTILS_SETTING_DATA_MANAGER_H
#define UI_APPEARANCE_MOCK_UTILS_SETTING_DATA_MANAGER_H

#include <gmock/gmock.h>

#include "errors.h"
#include "nocopyable.h"

namespace OHOS::ArkUi::UiAppearance {
constexpr int32_t INVALID_USER_ID = -1;

class SettingDataManager final : public NoCopyable {
public:
    static SettingDataManager &GetInstance()
    {
        static SettingDataManager instance;
        return instance;
    }

    ErrCode RegisterObserver(const std::string& key, const std::function<void(const std::string&, int32_t)>& updateFunc,
        int32_t userId = INVALID_USER_ID)
    {
        return MockRegisterObserver(key, updateFunc, userId);
    }

    ErrCode UnregisterObserver(const std::string& key, int32_t userId = INVALID_USER_ID)
    {
        return MockUnregisterObserver(key, userId);
    }

    ErrCode GetStringValue(const std::string& key, std::string& value, int32_t userId = INVALID_USER_ID) const
    {
        return MockGetStringValue(key, value, userId);
    }

    ErrCode GetInt32Value(const std::string& key, int32_t& value, int32_t userId = INVALID_USER_ID) const
    {
        return MockGetInt32Value(key, value, userId);
    }

    ErrCode GetInt32ValueStrictly(const std::string& key, int32_t& value, int32_t userId = INVALID_USER_ID) const
    {
        return MockGetInt32ValueStrictly(key, value, userId);
    }

    ErrCode GetBoolValue(const std::string& key, bool& value, int32_t userId = INVALID_USER_ID) const
    {
        return MockGetBoolValue(key, value, userId);
    }

    ErrCode SetStringValue(const std::string& key, const std::string& value, int32_t userId = INVALID_USER_ID,
        bool needNotify = true) const
    {
        return MockSetStringValue(key, value, userId, needNotify);
    }

    ErrCode SetInt32Value(const std::string& key, int32_t value, int32_t userId = INVALID_USER_ID,
        bool needNotify = true) const
    {
        return MockSetInt32Value(key, value, userId, needNotify);
    }

    ErrCode SetBoolValue(const std::string& key, bool value, int32_t userId = INVALID_USER_ID,
        bool needNotify = true) const
    {
        return MockSetBoolValue(key, value, userId, needNotify);
    }

    bool IsValidKey(const std::string& key, int32_t userId = INVALID_USER_ID) const
    {
        return MockIsValidKey(key, userId);
    }

    MOCK_METHOD(ErrCode, Initialize, ());
    MOCK_METHOD(bool, IsInitialized, (), (const));
    MOCK_METHOD(ErrCode, MockRegisterObserver,
        (const std::string&, const std::function<void(const std::string&, int32_t)>&, int32_t));
    MOCK_METHOD(ErrCode, MockUnregisterObserver, (const std::string&, int32_t));
    MOCK_METHOD(ErrCode, MockGetStringValue, (const std::string&, std::string&, int32_t), (const));
    MOCK_METHOD(ErrCode, MockGetInt32Value, (const std::string&, int32_t&, int32_t), (const));
    MOCK_METHOD(ErrCode, MockGetInt32ValueStrictly, (const std::string&, int32_t&, int32_t), (const));
    MOCK_METHOD(ErrCode, MockGetBoolValue, (const std::string&, bool&, int32_t), (const));
    MOCK_METHOD(ErrCode, MockSetStringValue, (const std::string&, const std::string&, int32_t, bool), (const));
    MOCK_METHOD(ErrCode, MockSetInt32Value, (const std::string&, int32_t, int32_t, bool), (const));
    MOCK_METHOD(ErrCode, MockSetBoolValue, (const std::string&, bool, int32_t, bool), (const));
    MOCK_METHOD(ErrCode, MockIsValidKey, (const std::string&, int32_t), (const));
};
} // namespace OHOS::ArkUi::UiAppearance

#endif // UI_APPEARANCE_MOCK_UTILS_SETTING_DATA_MANAGER_H
