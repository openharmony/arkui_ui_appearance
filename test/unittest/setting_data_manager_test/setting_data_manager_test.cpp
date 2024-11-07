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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

// mock
#include "data_ability_observer_stub.h"
#include "datashare_helper.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#define private public
#include "setting_data_manager.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS::ArkUi::UiAppearance {
namespace {
constexpr const char* TEST_KEY1 = "testKey1";
constexpr const char* TEST_KEY2 = "testKey2";
constexpr const char* TEST_KEY3 = "testKey3";
constexpr const char* TEST_VAL1 = "testVal1";
constexpr const char* TEST_VAL2 = "testVal2";
constexpr const char* TEST_VAL3 = "testVal3";
constexpr int32_t TEST_USER100 = 100;
constexpr int32_t TEST_USER1 = 1;
constexpr const char* SETTING_DATA_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
constexpr const char* SETTING_DATA_USER_URI_PREFIX =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_";
constexpr const char* SETTING_DATA_USER_URI_SUFFIX = "?Proxy=true";
constexpr const char* SETTING_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
constexpr const char* SETTING_DATA_KEY_URI = "&key=";
constexpr const char* SETTING_DATA_COLUMN_KEYWORD = "KEYWORD";
constexpr const char* SETTING_DATA_COLUMN_VALUE = "VALUE";

std::string GetUserUri(const int32_t userId)
{
    if (userId == INVALID_USER_ID) {
        return SETTING_DATA_URI;
    }
    return SETTING_DATA_USER_URI_PREFIX + std::to_string(userId) + SETTING_DATA_USER_URI_SUFFIX;
}

std::string GetUserUriWithKey(const int32_t userId, const std::string& key)
{
    if (userId == INVALID_USER_ID) {
        std::string uriString = SETTING_DATA_URI;
        uriString.append(SETTING_DATA_KEY_URI + key);
        return uriString;
    }
    std::string uriString = SETTING_DATA_USER_URI_PREFIX;
    uriString.append(std::to_string(userId) + SETTING_DATA_USER_URI_SUFFIX);
    uriString.append(SETTING_DATA_KEY_URI + key);
    return uriString;
}

std::string GenerateObserverName(const int32_t userId, const std::string& key)
{
    return key + "::" + std::to_string(userId);
}
}

class SettingDataManagerInitializeTest : public Test {
protected:
    void TearDown() override
    {
        SettingDataManager& manager = SettingDataManager::GetInstance();
        manager.isInitialized_ = false;
        manager.remoteObject_ = nullptr;
        manager.observers_.clear();
    }
};

class SettingDataManagerTest : public Test {
protected:
    sptr<IRemoteObject> object_ = sptr<IRemoteObject>::MakeSptr();
    sptr<ISystemAbilityManager> saMgr_ = sptr<ISystemAbilityManager>::MakeSptr();

    void SetUp() override
    {
        SettingDataManager& manager = SettingDataManager::GetInstance();
        SystemAbilityManagerClient& client = SystemAbilityManagerClient::GetInstance();
        EXPECT_CALL(client, GetSystemAbilityManager()).Times(1).WillOnce(Return(saMgr_));
        EXPECT_CALL(*saMgr_, GetSystemAbility(ARKUI_UI_APPEARANCE_SERVICE_ID)).Times(1).WillOnce(Return(object_));
        EXPECT_EQ(manager.Initialize(), ERR_OK);
        EXPECT_EQ(manager.IsInitialized(), true);
        EXPECT_EQ(manager.remoteObject_, object_);
    }

    void TearDown() override
    {
        SettingDataManager& manager = SettingDataManager::GetInstance();
        manager.isInitialized_ = false;
        manager.remoteObject_ = nullptr;
        manager.observers_.clear();
    }

    void RegisterObserverCreateFailTest(const int32_t userId, const std::string& key) const
    {
        GTEST_LOG_(INFO) << "RegisterObserverCreateFailTest userId: " << userId << ", key: " << key;
        HelperCreateFailTest(userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        size_t observerSize = manager.observers_.size();
        EXPECT_EQ(manager.observers_.size(), observerSize);
        EXPECT_EQ(manager.RegisterObserver(key, nullptr, userId), ERR_NO_INIT);
        EXPECT_EQ(manager.observers_.size(), observerSize);
    }

    void RegisterObserverTest(const int32_t userId, const std::string& key) const
    {
        GTEST_LOG_(INFO) << "RegisterObserverTest userId: " << userId << ", key: " << key;
        ExpectationSet expectationSet;
        HelperCreateTest(userId, expectationSet);

        auto checkRegisterObserver = [userId, key](const Uri& uri, const sptr<AAFwk::IDataAbilityObserver>& obs) {
            EXPECT_EQ(uri.ToString(), GetUserUriWithKey(userId, key));
            auto* settingDataObserver = static_cast<SettingDataObserver *>(obs.GetRefPtr());
            EXPECT_EQ(settingDataObserver->GetKey(), key);
            EXPECT_EQ(settingDataObserver->GetUserId(), userId);
        };
        expectationSet += EXPECT_CALL(*myHelper_, RegisterObserver(_, _))
            .Times(1).After(expectationSet).WillOnce(Invoke(checkRegisterObserver));

        HelperNotifyChangeTest(expectationSet, key, userId);
        HelperReleaseTest(expectationSet);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        size_t observerSize = manager.observers_.size();
        EXPECT_EQ(manager.RegisterObserver(key, nullptr, userId), ERR_OK);
        EXPECT_EQ(manager.observers_.size(), observerSize + 1);
        EXPECT_EQ(manager.RegisterObserver(key, nullptr, userId), ERR_OK);
        EXPECT_EQ(manager.observers_.size(), observerSize + 1);
    }

    void UnregisterObserverCreateFailTest(const int32_t userId, const std::string& key) const
    {
        GTEST_LOG_(INFO) << "UnregisterObserverCreateFailTest userId: " << userId << ", key: " << key;
        HelperCreateFailTest(userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        size_t observerSize = manager.observers_.size();
        EXPECT_EQ(manager.observers_.size(), observerSize);
        EXPECT_EQ(manager.UnregisterObserver(key, userId), ERR_NO_INIT);
        EXPECT_EQ(manager.observers_.size(), observerSize - 1);
    }

    void UnregisterObserverTest(const int32_t userId, const std::string& key) const
    {
        GTEST_LOG_(INFO) << "UnregisterObserverTest userId: " << userId << ", key: " << key;
        ExpectationSet expectationSet;
        HelperCreateTest(userId, expectationSet);

        auto checkUnregisterObserver = [userId, key](const Uri& uri, const sptr<AAFwk::IDataAbilityObserver>& obs) {
            EXPECT_EQ(uri.ToString(), GetUserUriWithKey(userId, key));
            auto* settingDataObserver = static_cast<SettingDataObserver *>(obs.GetRefPtr());
            EXPECT_EQ(settingDataObserver->GetKey(), key);
            EXPECT_EQ(settingDataObserver->GetUserId(), userId);
        };
        expectationSet += EXPECT_CALL(*myHelper_, UnregisterObserver(_, _))
            .Times(1).After(expectationSet).WillOnce(Invoke(checkUnregisterObserver));

        HelperReleaseTest(expectationSet);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        size_t observerSize = manager.observers_.size();
        EXPECT_EQ(manager.UnregisterObserver(key, userId), ERR_OK);
        EXPECT_EQ(manager.observers_.size(), observerSize - 1);
        EXPECT_EQ(manager.UnregisterObserver(key, userId), ERR_INVALID_VALUE);
        EXPECT_EQ(manager.observers_.size(), observerSize - 1);
    }

    void SetStringValueCreateFailTest(const std::string& key,
        const std::string& value, const int32_t userId, const bool needNotify) const
    {
        GTEST_LOG_(INFO) << "SetStringValueCreateFailTest userId: " << userId << ", key: " << key;
        HelperCreateFailTest(userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.SetStringValue(key, value, userId, needNotify), ERR_NO_INIT);
    }

    void SetStringValueInsertAndUpdateFailTest(const std::string& key,
        const std::string& value, const int32_t userId, const bool needNotify) const
    {
        GTEST_LOG_(INFO) << "SetStringValueInsertAndUpdateFailTest userId: " << userId << ", key: " << key;
        SetStringValueInsertAndUpdateFailInnerTest(key, value, userId, needNotify);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.SetStringValue(key, value, userId, needNotify), ERR_INVALID_OPERATION);
    }

    void SetStringValueUpdateTest(const std::string& key,
        const std::string& value, const int32_t userId, const bool needNotify) const
    {
        GTEST_LOG_(INFO) << "SetStringValueUpdateTest userId: " << userId << ", key: " << key;
        SetStringValueUpdateInnerTest(key, value, userId, needNotify);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.SetStringValue(key, value, userId, needNotify), ERR_OK);
    }

    void SetStringValueInsertTest(const std::string& key,
        const std::string& value, const int32_t userId, const bool needNotify) const
    {
        GTEST_LOG_(INFO) << "SetStringValueInsertTest userId: " << userId << ", key: " << key;
        SetStringValueInsertInnerTest(key, value, userId, needNotify);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.SetStringValue(key, value, userId, needNotify), ERR_OK);
    }

    void SetInt32ValueCreateFailTest(const std::string& key,
        const int32_t value, const int32_t userId, const bool needNotify) const
    {
        GTEST_LOG_(INFO) << "SetInt32ValueCreateFailTest userId: " << userId << ", key: " << key;
        HelperCreateFailTest(userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.SetInt32Value(key, value, userId, needNotify), ERR_NO_INIT);
    }

    void SetInt32ValueInsertAndUpdateFailTest(const std::string& key,
        const int32_t value, const int32_t userId, const bool needNotify) const
    {
        GTEST_LOG_(INFO) << "SetInt32ValueInsertAndUpdateFailTest userId: " << userId << ", key: " << key;
        SetStringValueInsertAndUpdateFailInnerTest(key, std::to_string(value), userId, needNotify);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.SetInt32Value(key, value, userId, needNotify), ERR_INVALID_OPERATION);
    }

    void SetInt32ValueUpdateTest(const std::string& key,
        const int32_t value, const int32_t userId, const bool needNotify) const
    {
        GTEST_LOG_(INFO) << "SetInt32ValueUpdateTest userId: " << userId << ", key: " << key;
        SetStringValueUpdateInnerTest(key, std::to_string(value), userId, needNotify);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.SetInt32Value(key, value, userId, needNotify), ERR_OK);
    }

    void SetInt32ValueInsertTest(const std::string& key,
        const int32_t value, const int32_t userId, const bool needNotify) const
    {
        GTEST_LOG_(INFO) << "SetInt32ValueInsertTest userId: " << userId << ", key: " << key;
        SetStringValueInsertInnerTest(key, std::to_string(value), userId, needNotify);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.SetInt32Value(key, value, userId, needNotify), ERR_OK);
    }

    void SetBoolValueCreateFailTest(const std::string& key,
        const bool value, const int32_t userId, const bool needNotify) const
    {
        GTEST_LOG_(INFO) << "SetBoolValueCreateFailTest userId: " << userId << ", key: " << key;
        HelperCreateFailTest(userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.SetBoolValue(key, value, userId, needNotify), ERR_NO_INIT);
    }

    void SetBoolValueInsertAndUpdateFailTest(const std::string& key,
        const bool value, const int32_t userId, const bool needNotify) const
    {
        GTEST_LOG_(INFO) << "SetBoolValueInsertAndUpdateFailTest userId: " << userId << ", key: " << key;
        SetStringValueInsertAndUpdateFailInnerTest(key, value ? "true" : "false", userId, needNotify);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.SetBoolValue(key, value, userId, needNotify), ERR_INVALID_OPERATION);
    }

    void SetBoolValueUpdateTest(const std::string& key,
        const bool value, const int32_t userId, const bool needNotify) const
    {
        GTEST_LOG_(INFO) << "SetBoolValueUpdateTest userId: " << userId << ", key: " << key;
        SetStringValueUpdateInnerTest(key, value ? "true" : "false", userId, needNotify);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.SetBoolValue(key, value, userId, needNotify), ERR_OK);
    }

    void SetBoolValueInsertTest(const std::string& key,
        const bool value, const int32_t userId, const bool needNotify) const
    {
        GTEST_LOG_(INFO) << "SetBoolValueInsertTest userId: " << userId << ", key: " << key;
        SetStringValueInsertInnerTest(key, value ? "true" : "false", userId, needNotify);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.SetBoolValue(key, value, userId, needNotify), ERR_OK);
    }

    void GetStringValueCreateFailTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "GetStringValueCreateFailTest userId: " << userId << ", key: " << key;
        HelperCreateFailTest(userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        std::string value;
        EXPECT_EQ(manager.GetStringValue(key, value, userId), ERR_NO_INIT);
    }

    void GetStringValueQueryNullTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "GetStringValueQueryNullTest userId: " << userId << ", key: " << key;
        GetStringValueQueryNullInnerTest(key, userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        std::string value;
        EXPECT_EQ(manager.GetStringValue(key, value, userId), ERR_INVALID_OPERATION);
    }

    void GetStringValueResultNotFoundTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "GetStringValueResultNotFoundTest userId: " << userId << ", key: " << key;
        GetStringValueResultNotFoundInnerTest(key, userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        std::string value;
        EXPECT_EQ(manager.GetStringValue(key, value, userId), ERR_NAME_NOT_FOUND);
    }

    void GetStringValueResultGetStringFailTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "GetStringValueResultGetStringFailTest userId: " << userId << ", key: " << key;
        GetStringValueResultGetStringFailInnerTest(key, userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        std::string value;
        EXPECT_EQ(manager.GetStringValue(key, value, userId), ERR_INVALID_VALUE);
    }

    void GetStringValueResultTest(const std::string& key, const std::string& valueExpect, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "GetStringValueResultTest userId: " << userId << ", key: " << key << ", value: " <<
            valueExpect;
        GetStringValueResultInnerTest(key, userId, valueExpect);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        std::string value;
        EXPECT_EQ(manager.GetStringValue(key, value, userId), ERR_OK);
        EXPECT_EQ(value, valueExpect);
    }

    void GetInt32ValueCreateFailTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "GetInt32ValueCreateFailTest userId: " << userId << ", key: " << key;
        HelperCreateFailTest(userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        int32_t value;
        EXPECT_EQ(manager.GetInt32Value(key, value, userId), ERR_NO_INIT);
    }

    void GetInt32ValueQueryNullTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "GetInt32ValueQueryNullTest userId: " << userId << ", key: " << key;
        GetStringValueQueryNullInnerTest(key, userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        int32_t value;
        EXPECT_EQ(manager.GetInt32Value(key, value, userId), ERR_INVALID_OPERATION);
    }

    void GetInt32ValueResultNotFoundTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "GetInt32ValueResultNotFoundTest userId: " << userId << ", key: " << key;
        GetStringValueResultNotFoundInnerTest(key, userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        int32_t value;
        EXPECT_EQ(manager.GetInt32Value(key, value, userId), ERR_NAME_NOT_FOUND);
    }

    void GetInt32ValueResultGetStringFailTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "GetInt32ValueResultGetStringFailTest userId: " << userId << ", key: " << key;
        GetStringValueResultGetStringFailInnerTest(key, userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        int32_t value;
        EXPECT_EQ(manager.GetInt32Value(key, value, userId), ERR_INVALID_VALUE);
    }

    void GetInt32ValueResultTest(const std::string& key, const int32_t userId,
        const int32_t valueExpect, const std::string& savedVal, const int32_t code) const
    {
        GTEST_LOG_(INFO) << "GetInt32ValueResultTest userId: " << userId << ", key: " << key << ", value: " <<
            savedVal;
        GetStringValueResultInnerTest(key, userId, savedVal);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        int32_t value;
        EXPECT_EQ(manager.GetInt32Value(key, value, userId), code);
        if (code == ERR_OK) {
            EXPECT_EQ(value, valueExpect);
        }
    }

    void GetInt32ValueStrictlyResultTest(const std::string& key, const int32_t userId,
        const int32_t valueExpect, const std::string& savedVal, const int32_t code) const
    {
        GTEST_LOG_(INFO) << "GetInt32ValueStrictlyResultTest userId: " << userId << ", key: " << key;
        GetStringValueResultInnerTest(key, userId, savedVal);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        int32_t value;
        EXPECT_EQ(manager.GetInt32ValueStrictly(key, value, userId), code);
        if (code == ERR_OK) {
            EXPECT_EQ(value, valueExpect);
        }
    }

    void GetBoolValueCreateFailTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "GetBoolValueCreateFailTest userId: " << userId << ", key: " << key;
        HelperCreateFailTest(userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        bool value;
        EXPECT_EQ(manager.GetBoolValue(key, value, userId), ERR_NO_INIT);
    }

    void GetBoolValueQueryNullTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "GetBoolValueQueryNullTest userId: " << userId << ", key: " << key;
        GetStringValueQueryNullInnerTest(key, userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        bool value;
        EXPECT_EQ(manager.GetBoolValue(key, value, userId), ERR_INVALID_OPERATION);
    }

    void GetBoolValueResultNotFoundTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "GetBoolValueResultNotFoundTest userId: " << userId << ", key: " << key;
        GetStringValueResultNotFoundInnerTest(key, userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        bool value;
        EXPECT_EQ(manager.GetBoolValue(key, value, userId), ERR_NAME_NOT_FOUND);
    }

    void GetBoolValueResultGetStringFailTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "GetBoolValueResultGetStringFailTest userId: " << userId << ", key: " << key;
        GetStringValueResultGetStringFailInnerTest(key, userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        bool value;
        EXPECT_EQ(manager.GetBoolValue(key, value, userId), ERR_INVALID_VALUE);
    }

    void GetBoolValueResultTest(const std::string& key, const int32_t userId,
        const bool valueExpect, const std::string& savedVal, const int32_t code) const
    {
        GTEST_LOG_(INFO) << "GetBoolValueResultTest userId: " << userId << ", key: " << key << ", value: " <<
            savedVal;
        GetStringValueResultInnerTest(key, userId, savedVal);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        bool value;
        EXPECT_EQ(manager.GetBoolValue(key, value, userId), code);
        if (code == ERR_OK) {
            EXPECT_EQ(value, valueExpect);
        }
    }

    void IsValidKeyCreateFailTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "IsValidKeyCreateFailTest userId: " << userId << ", key: " << key;
        HelperCreateFailTest(userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.IsValidKey(key, userId), false);
    }

    void IsValidKeyQueryNullTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "IsValidKeyQueryNullTest userId: " << userId << ", key: " << key;
        GetStringValueQueryNullInnerTest(key, userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.IsValidKey(key, userId), false);
    }

    void IsValidKeyResultNotFoundTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "IsValidKeyResultNotFoundTest userId: " << userId << ", key: " << key;
        GetStringValueResultNotFoundInnerTest(key, userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.IsValidKey(key, userId), false);
    }

    void IsValidKeyResultGetStringFailTest(const std::string& key, const int32_t userId) const
    {
        GTEST_LOG_(INFO) << "IsValidKeyResultGetStringFailTest userId: " << userId << ", key: " << key;
        GetStringValueResultGetStringFailInnerTest(key, userId);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.IsValidKey(key, userId), false);
    }

    void IsValidKeyResultTest(const std::string& key, const int32_t userId,
        const bool valueExpect, const std::string& savedVal) const
    {
        GTEST_LOG_(INFO) << "IsValidKeyResultTest userId: " << userId << ", key: " << key << ", value: " <<
            savedVal;
        GetStringValueResultInnerTest(key, userId, savedVal);
        SettingDataManager& manager = SettingDataManager::GetInstance();
        EXPECT_EQ(manager.IsValidKey(key, userId), valueExpect);
    }

private:
    std::shared_ptr<DataShare::DataShareHelper> myHelper_ = std::make_shared<DataShare::DataShareHelper>();
    std::shared_ptr<DataShare::DataShareResultSet> myResult_ = std::make_shared<DataShare::DataShareResultSet>();

    void HelperCreateFailTest(const int32_t userId) const
    {
        auto* testInfo = UnitTest::GetInstance()->current_test_info();
        IPCSkeleton& instance = IPCSkeleton::GetInstance();
        ExpectationSet expectationSet = EXPECT_CALL(instance, MockResetCallingIdentity())
            .Times(1).WillOnce(Return(testInfo->name()));
        DataShare::DataShareHelper& helper = DataShare::DataShareHelper::GetInstance();
        auto checkMockCreate = [this, userId](
            const sptr<IRemoteObject>& token, const std::string& uri, const std::string& extUri, Unused) {
            EXPECT_EQ(token, object_);
            EXPECT_EQ(uri, GetUserUri(userId));
            EXPECT_EQ(extUri, SETTING_DATA_EXT_URI);
            return std::make_pair(DataShare::E_BASE, nullptr);
        };
        expectationSet += EXPECT_CALL(helper, MockCreate(_, _, _, _))
            .Times(1).After(expectationSet).WillOnce(checkMockCreate);
        EXPECT_CALL(instance, MockSetCallingIdentity(testInfo->name())).Times(1).After(expectationSet);
    }

    void HelperCreateTest(const int32_t userId, ExpectationSet& expectationSet) const
    {
        auto* testInfo = UnitTest::GetInstance()->current_test_info();
        IPCSkeleton& instance = IPCSkeleton::GetInstance();
        expectationSet += EXPECT_CALL(instance, MockResetCallingIdentity())
            .Times(1).WillOnce(Return(testInfo->name()));

        DataShare::DataShareHelper& helper = DataShare::DataShareHelper::GetInstance();
        auto checkMockCreate = [this, userId, weak = myHelper_->weak_from_this()](
            const sptr<IRemoteObject>& token, const std::string& uri, const std::string& extUri, Unused) {
            auto myHelper = weak.lock();
            EXPECT_EQ(token, object_);
            EXPECT_EQ(uri, GetUserUri(userId));
            EXPECT_EQ(extUri, SETTING_DATA_EXT_URI);
            return std::make_pair(DataShare::E_OK, myHelper);
        };
        expectationSet += EXPECT_CALL(helper, MockCreate(_, _, _, _))
            .Times(1).After(expectationSet).WillOnce(checkMockCreate);
    }

    void HelperReleaseTest(ExpectationSet& expectationSet) const
    {
        expectationSet += EXPECT_CALL(*myHelper_, Release())
            .Times(1).After(expectationSet).WillOnce(Return(true));

        auto* testInfo = UnitTest::GetInstance()->current_test_info();
        IPCSkeleton& instance = IPCSkeleton::GetInstance();
        EXPECT_CALL(instance, MockSetCallingIdentity(testInfo->name())).Times(1).After(expectationSet);
    }

    void HelperInsertTest(ExpectationSet& expectationSet, const std::string& key, const std::string& value,
        const int32_t userId, int32_t returnVal) const
    {
        auto checkInsert = [userId, key, value, returnVal](Uri& uri, const DataShare::DataShareValuesBucket& bucket) {
            EXPECT_EQ(uri.ToString(), GetUserUriWithKey(userId, key));
            bool isValid = false;
            auto obj = bucket.Get(SETTING_DATA_COLUMN_KEYWORD, isValid);
            EXPECT_TRUE(isValid);
            std::string keyObj = obj;
            EXPECT_EQ(keyObj, key);
            isValid = false;
            obj = bucket.Get(SETTING_DATA_COLUMN_VALUE, isValid);
            EXPECT_TRUE(isValid);
            std::string valObj = obj;
            EXPECT_EQ(valObj, value);
            return returnVal;
        };
        expectationSet += EXPECT_CALL(*myHelper_, Insert(_, _))
            .Times(1).After(expectationSet).WillOnce(Invoke(checkInsert));
    }

    void HelperUpdateTest(ExpectationSet& expectationSet, const std::string& key, const std::string& value,
        const int32_t userId, int32_t returnVal) const
    {
        auto checkUpdate = [userId, key, value, returnVal](Uri& uri,
            const DataShare::DataSharePredicates& predicates, const DataShare::DataShareValuesBucket& bucket) {
            EXPECT_EQ(uri.ToString(), GetUserUriWithKey(userId, key));

            auto& operationList = predicates.GetOperationList();
            EXPECT_EQ(operationList.size(), 1);
            EXPECT_EQ(operationList[0].operation, DataShare::EQUAL_TO);
            std::string predKey0 = operationList[0].GetSingle(0);
            std::string predKey1 = operationList[0].GetSingle(1);
            EXPECT_EQ(predKey0, SETTING_DATA_COLUMN_KEYWORD);
            EXPECT_EQ(predKey1, key);

            bool isValid = false;
            auto obj = bucket.Get(SETTING_DATA_COLUMN_KEYWORD, isValid);
            EXPECT_TRUE(isValid);
            std::string keyObj = obj;
            EXPECT_EQ(keyObj, key);
            isValid = false;
            obj = bucket.Get(SETTING_DATA_COLUMN_VALUE, isValid);
            EXPECT_TRUE(isValid);
            std::string valObj = obj;
            EXPECT_EQ(valObj, value);
            return returnVal;
        };
        expectationSet += EXPECT_CALL(*myHelper_, Update(_, _, _))
            .Times(1).After(expectationSet).WillOnce(Invoke(checkUpdate));
    }

    void HelperQueryNullTest(ExpectationSet& expectationSet, const std::string& key, const int32_t userId) const
    {
        auto checkQuery = [key, userId](Uri& uri, const DataShare::DataSharePredicates& predicates,
            std::vector<std::string>& columns) {
            EXPECT_EQ(uri.ToString(), GetUserUriWithKey(userId, key));

            auto& operationList = predicates.GetOperationList();
            EXPECT_EQ(operationList.size(), 1);
            EXPECT_EQ(operationList[0].operation, DataShare::EQUAL_TO);
            std::string predKey0 = operationList[0].GetSingle(0);
            std::string predKey1 = operationList[0].GetSingle(1);
            EXPECT_EQ(predKey0, SETTING_DATA_COLUMN_KEYWORD);
            EXPECT_EQ(predKey1, key);

            EXPECT_EQ(columns.size(), 1);
            EXPECT_EQ(columns[0], SETTING_DATA_COLUMN_VALUE);
            return nullptr;
        };
        expectationSet += EXPECT_CALL(*myHelper_, Query(_, _, _))
            .Times(1).After(expectationSet).WillOnce(Invoke(checkQuery));
    }

    void HelperQueryTest(ExpectationSet& expectationSet, const std::string& key, const int32_t userId) const
    {
        auto checkQuery = [this, key, userId](Uri& uri, const DataShare::DataSharePredicates& predicates,
            std::vector<std::string>& columns) {
            EXPECT_EQ(uri.ToString(), GetUserUriWithKey(userId, key));

            auto& operationList = predicates.GetOperationList();
            EXPECT_EQ(operationList.size(), 1);
            EXPECT_EQ(operationList[0].operation, DataShare::EQUAL_TO);
            std::string predKey0 = operationList[0].GetSingle(0);
            std::string predKey1 = operationList[0].GetSingle(1);
            EXPECT_EQ(predKey0, SETTING_DATA_COLUMN_KEYWORD);
            EXPECT_EQ(predKey1, key);

            EXPECT_EQ(columns.size(), 1);
            EXPECT_EQ(columns[0], SETTING_DATA_COLUMN_VALUE);
            return myResult_;
        };
        expectationSet += EXPECT_CALL(*myHelper_, Query(_, _, _))
            .Times(1).After(expectationSet).WillOnce(Invoke(checkQuery));
    }

    void ResultNotFoundTest(ExpectationSet& expectationSet, const std::string& key, const int32_t userId) const
    {
        auto checkGetRowCount = [](int32_t& count) {
            count = 0;
            return 0;
        };
        expectationSet += EXPECT_CALL(*myResult_, GetRowCount(_))
            .Times(1).After(expectationSet).WillOnce(Invoke(checkGetRowCount));
    }

    void ResultGetStringFailTest(ExpectationSet& expectationSet, const std::string& key, const int32_t userId) const
    {
        auto checkGetRowCount = [](int32_t& count) {
            count = 1;
            return 0;
        };
        expectationSet += EXPECT_CALL(*myResult_, GetRowCount(_))
            .Times(1).After(expectationSet).WillOnce(Invoke(checkGetRowCount));

        expectationSet += EXPECT_CALL(*myResult_, GoToRow(0))
            .Times(1).After(expectationSet).WillOnce(Return(0));

        expectationSet += EXPECT_CALL(*myResult_, GetString(0, _))
            .Times(1).After(expectationSet).WillOnce(Return(1));
    }

    void ResultTest(ExpectationSet& expectationSet, const std::string& key, const int32_t userId,
        const std::string& valueExpect) const
    {
        auto checkGetRowCount = [](int32_t& count) {
            count = 1;
            return 0;
        };
        expectationSet += EXPECT_CALL(*myResult_, GetRowCount(_))
            .Times(1).After(expectationSet).WillOnce(Invoke(checkGetRowCount));

        expectationSet += EXPECT_CALL(*myResult_, GoToRow(0))
            .Times(1).After(expectationSet).WillOnce(Return(0));

        auto checkGetString = [valueExpect](Unused, std::string& value) {
            value = valueExpect;
            return 0;
        };
        expectationSet += EXPECT_CALL(*myResult_, GetString(0, _))
            .Times(1).After(expectationSet).WillOnce(Invoke(checkGetString));
    }

    void ResultCloseTest(ExpectationSet& expectationSet) const
    {
        expectationSet += EXPECT_CALL(*myResult_, Close()).Times(1).After(expectationSet);
    }

    void HelperNotifyChangeTest(ExpectationSet& expectationSet, const std::string& key, const int32_t userId) const
    {
        auto checkNotifyChange = [userId, key](const Uri& uri) {
            EXPECT_EQ(uri.ToString(), GetUserUriWithKey(userId, key));
        };
        expectationSet += EXPECT_CALL(*myHelper_, NotifyChange(_))
            .Times(1).After(expectationSet).WillOnce(Invoke(checkNotifyChange));
    }

    void SetStringValueInsertAndUpdateFailInnerTest(const std::string& key,
        const std::string& value, const int32_t userId, const bool needNotify) const
    {
        ExpectationSet expectationSet;
        HelperCreateTest(userId, expectationSet);
        HelperUpdateTest(expectationSet, key, value, userId, 0);
        HelperInsertTest(expectationSet, key, value, userId, 0);
        HelperReleaseTest(expectationSet);
    }

    void SetStringValueUpdateInnerTest(const std::string& key,
        const std::string& value, const int32_t userId, const bool needNotify) const
    {
        ExpectationSet expectationSet;
        HelperCreateTest(userId, expectationSet);
        HelperUpdateTest(expectationSet, key, value, userId, 1);
        EXPECT_CALL(*myHelper_, Insert(_, _)).Times(0);
        if (needNotify) {
            HelperNotifyChangeTest(expectationSet, key, userId);
        }
        HelperReleaseTest(expectationSet);
    }

    void SetStringValueInsertInnerTest(const std::string& key,
        const std::string& value, const int32_t userId, const bool needNotify) const
    {
        ExpectationSet expectationSet;
        HelperCreateTest(userId, expectationSet);
        HelperUpdateTest(expectationSet, key, value, userId, 0);
        HelperInsertTest(expectationSet, key, value, userId, 1);
        if (needNotify) {
            HelperNotifyChangeTest(expectationSet, key, userId);
        }
        HelperReleaseTest(expectationSet);
    }

    void GetStringValueQueryNullInnerTest(const std::string& key, const int32_t userId) const
    {
        ExpectationSet expectationSet;
        HelperCreateTest(userId, expectationSet);
        HelperQueryNullTest(expectationSet, key, userId);
        HelperReleaseTest(expectationSet);
    }

    void GetStringValueResultNotFoundInnerTest(const std::string& key, const int32_t userId) const
    {
        ExpectationSet expectationSet;
        HelperCreateTest(userId, expectationSet);
        HelperQueryTest(expectationSet, key, userId);
        HelperReleaseTest(expectationSet);
        ResultNotFoundTest(expectationSet, key, userId);
        ResultCloseTest(expectationSet);
    }

    void GetStringValueResultGetStringFailInnerTest(const std::string& key, const int32_t userId) const
    {
        ExpectationSet expectationSet;
        HelperCreateTest(userId, expectationSet);
        HelperQueryTest(expectationSet, key, userId);
        HelperReleaseTest(expectationSet);
        ResultGetStringFailTest(expectationSet, key, userId);
        ResultCloseTest(expectationSet);
    }

    void GetStringValueResultInnerTest(const std::string& key, const int32_t userId,
        const std::string& valueExpect) const
    {
        ExpectationSet expectationSet;
        HelperCreateTest(userId, expectationSet);
        HelperQueryTest(expectationSet, key, userId);
        HelperReleaseTest(expectationSet);
        ResultTest(expectationSet, key, userId, valueExpect);
        ResultCloseTest(expectationSet);
    }
};

HWTEST_F(SettingDataManagerInitializeTest, InitializeTest_0100, TestSize.Level1)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    EXPECT_EQ(manager.IsInitialized(), false);

    SystemAbilityManagerClient& client = SystemAbilityManagerClient::GetInstance();
    EXPECT_CALL(client, GetSystemAbilityManager()).Times(1).WillOnce(Return(nullptr));
    EXPECT_EQ(manager.Initialize(), ERR_NO_INIT);
    EXPECT_EQ(manager.IsInitialized(), false);
}

HWTEST_F(SettingDataManagerInitializeTest, InitializeTest_0200, TestSize.Level1)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    EXPECT_EQ(manager.IsInitialized(), false);

    SystemAbilityManagerClient& client = SystemAbilityManagerClient::GetInstance();
    auto saMgr = sptr<ISystemAbilityManager>::MakeSptr();
    EXPECT_CALL(client, GetSystemAbilityManager()).Times(1).WillOnce(Return(saMgr));
    EXPECT_CALL(*saMgr, GetSystemAbility(ARKUI_UI_APPEARANCE_SERVICE_ID)).Times(1).WillOnce(Return(nullptr));
    EXPECT_EQ(manager.Initialize(), ERR_NO_INIT);
    EXPECT_EQ(manager.IsInitialized(), false);
}

HWTEST_F(SettingDataManagerInitializeTest, InitializeTest_0300, TestSize.Level1)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    EXPECT_EQ(manager.IsInitialized(), false);

    SystemAbilityManagerClient& client = SystemAbilityManagerClient::GetInstance();
    auto saMgr = sptr<ISystemAbilityManager>::MakeSptr();
    EXPECT_CALL(client, GetSystemAbilityManager()).Times(1).WillOnce(Return(saMgr));
    auto obj = sptr<IRemoteObject>::MakeSptr();
    EXPECT_CALL(*saMgr, GetSystemAbility(ARKUI_UI_APPEARANCE_SERVICE_ID)).Times(1).WillOnce(Return(obj));
    EXPECT_EQ(manager.Initialize(), ERR_OK);
    EXPECT_EQ(manager.IsInitialized(), true);
    EXPECT_EQ(manager.remoteObject_, obj);

    EXPECT_EQ(manager.Initialize(), ERR_OK);
    EXPECT_EQ(manager.IsInitialized(), true);
    EXPECT_EQ(manager.remoteObject_, obj);
}

HWTEST_F(SettingDataManagerInitializeTest, RegisterObserver_0100, TestSize.Level1)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    EXPECT_EQ(manager.IsInitialized(), false);
    EXPECT_EQ(manager.RegisterObserver(TEST_KEY1, nullptr, TEST_USER100), ERR_NO_INIT);
}

HWTEST_F(SettingDataManagerTest, RegisterObserver_0200, TestSize.Level1)
{
    RegisterObserverCreateFailTest(INVALID_USER_ID, TEST_KEY1);
    RegisterObserverCreateFailTest(INVALID_USER_ID, TEST_KEY2);
    RegisterObserverCreateFailTest(INVALID_USER_ID, TEST_KEY3);
}

HWTEST_F(SettingDataManagerTest, RegisterObserver_0300, TestSize.Level1)
{
    RegisterObserverCreateFailTest(INVALID_USER_ID, TEST_KEY1);
    RegisterObserverCreateFailTest(TEST_USER100, TEST_KEY1);
    RegisterObserverCreateFailTest(TEST_USER1, TEST_KEY1);
}

HWTEST_F(SettingDataManagerTest, RegisterObserver_0400, TestSize.Level1)
{
    RegisterObserverCreateFailTest(INVALID_USER_ID, TEST_KEY1);
    RegisterObserverCreateFailTest(TEST_USER100, TEST_KEY2);
    RegisterObserverCreateFailTest(TEST_USER1, TEST_KEY3);
}

HWTEST_F(SettingDataManagerTest, RegisterObserver_0500, TestSize.Level1)
{
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY1);
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY2);
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY3);
}

HWTEST_F(SettingDataManagerTest, RegisterObserver_0600, TestSize.Level1)
{
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY1);
    RegisterObserverTest(TEST_USER100, TEST_KEY1);
    RegisterObserverTest(TEST_USER1, TEST_KEY1);
}

HWTEST_F(SettingDataManagerTest, RegisterObserver_0700, TestSize.Level1)
{
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY1);
    RegisterObserverTest(TEST_USER100, TEST_KEY2);
    RegisterObserverTest(TEST_USER1, TEST_KEY3);
}

HWTEST_F(SettingDataManagerTest, UnregisterObserver_0100, TestSize.Level1)
{
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY1);
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY2);
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY3);
    UnregisterObserverCreateFailTest(INVALID_USER_ID, TEST_KEY1);
    UnregisterObserverCreateFailTest(INVALID_USER_ID, TEST_KEY2);
    UnregisterObserverCreateFailTest(INVALID_USER_ID, TEST_KEY3);
}

HWTEST_F(SettingDataManagerTest, UnregisterObserver_0200, TestSize.Level1)
{
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY1);
    RegisterObserverTest(TEST_USER100, TEST_KEY1);
    RegisterObserverTest(TEST_USER1, TEST_KEY1);
    UnregisterObserverCreateFailTest(INVALID_USER_ID, TEST_KEY1);
    UnregisterObserverCreateFailTest(TEST_USER100, TEST_KEY1);
    UnregisterObserverCreateFailTest(TEST_USER1, TEST_KEY1);
}

HWTEST_F(SettingDataManagerTest, UnregisterObserver_0300, TestSize.Level1)
{
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY1);
    RegisterObserverTest(TEST_USER100, TEST_KEY2);
    RegisterObserverTest(TEST_USER1, TEST_KEY3);
    UnregisterObserverCreateFailTest(INVALID_USER_ID, TEST_KEY1);
    UnregisterObserverCreateFailTest(TEST_USER100, TEST_KEY2);
    UnregisterObserverCreateFailTest(TEST_USER1, TEST_KEY3);
}

HWTEST_F(SettingDataManagerTest, UnregisterObserver_0400, TestSize.Level1)
{
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY1);
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY2);
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY3);
    UnregisterObserverTest(INVALID_USER_ID, TEST_KEY1);
    UnregisterObserverTest(INVALID_USER_ID, TEST_KEY2);
    UnregisterObserverTest(INVALID_USER_ID, TEST_KEY3);
}

HWTEST_F(SettingDataManagerTest, UnregisterObserver_0500, TestSize.Level1)
{
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY1);
    RegisterObserverTest(TEST_USER100, TEST_KEY1);
    RegisterObserverTest(TEST_USER1, TEST_KEY1);
    UnregisterObserverTest(INVALID_USER_ID, TEST_KEY1);
    UnregisterObserverTest(TEST_USER100, TEST_KEY1);
    UnregisterObserverTest(TEST_USER1, TEST_KEY1);
}

HWTEST_F(SettingDataManagerTest, UnregisterObserver_0600, TestSize.Level1)
{
    RegisterObserverTest(INVALID_USER_ID, TEST_KEY1);
    RegisterObserverTest(TEST_USER100, TEST_KEY2);
    RegisterObserverTest(TEST_USER1, TEST_KEY3);
    UnregisterObserverTest(INVALID_USER_ID, TEST_KEY1);
    UnregisterObserverTest(TEST_USER100, TEST_KEY2);
    UnregisterObserverTest(TEST_USER1, TEST_KEY3);
}

HWTEST_F(SettingDataManagerTest, UnregisterObserver_0700, TestSize.Level1)
{
    SettingDataManager& manager = SettingDataManager::GetInstance();
    size_t observerSize = manager.observers_.size();
    EXPECT_EQ(manager.observers_.size(), observerSize);
    EXPECT_EQ(manager.UnregisterObserver(TEST_KEY1, INVALID_USER_ID), ERR_INVALID_VALUE);
    EXPECT_EQ(manager.observers_.size(), observerSize);
    manager.observers_.emplace(GenerateObserverName(INVALID_USER_ID, TEST_KEY1), nullptr);
    EXPECT_EQ(manager.observers_.size(), observerSize + 1);
    EXPECT_EQ(manager.UnregisterObserver(TEST_KEY1, INVALID_USER_ID), ERR_INVALID_OPERATION);
    EXPECT_EQ(manager.observers_.size(), observerSize);

    observerSize = manager.observers_.size();
    EXPECT_EQ(manager.observers_.size(), observerSize);
    EXPECT_EQ(manager.UnregisterObserver(TEST_KEY2, TEST_USER100), ERR_INVALID_VALUE);
    EXPECT_EQ(manager.observers_.size(), observerSize);
    manager.observers_.emplace(GenerateObserverName(TEST_USER100, TEST_KEY2), nullptr);
    EXPECT_EQ(manager.observers_.size(), observerSize + 1);
    EXPECT_EQ(manager.UnregisterObserver(TEST_KEY2, TEST_USER100), ERR_INVALID_OPERATION);
    EXPECT_EQ(manager.observers_.size(), observerSize);
}

HWTEST_F(SettingDataManagerTest, SetStringValue_0100, TestSize.Level1)
{
    SetStringValueCreateFailTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, true);
    SetStringValueCreateFailTest(TEST_KEY2, TEST_VAL2, INVALID_USER_ID, true);
    SetStringValueCreateFailTest(TEST_KEY3, TEST_VAL3, INVALID_USER_ID, true);
    SetStringValueCreateFailTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, false);
    SetStringValueCreateFailTest(TEST_KEY2, TEST_VAL2, INVALID_USER_ID, false);
    SetStringValueCreateFailTest(TEST_KEY3, TEST_VAL3, INVALID_USER_ID, false);

    SetStringValueCreateFailTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, true);
    SetStringValueCreateFailTest(TEST_KEY1, TEST_VAL1, TEST_USER100, true);
    SetStringValueCreateFailTest(TEST_KEY1, TEST_VAL1, TEST_USER1, true);
    SetStringValueCreateFailTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, false);
    SetStringValueCreateFailTest(TEST_KEY1, TEST_VAL1, TEST_USER100, false);
    SetStringValueCreateFailTest(TEST_KEY1, TEST_VAL1, TEST_USER1, false);
}

HWTEST_F(SettingDataManagerTest, SetStringValue_0200, TestSize.Level1)
{
    SetStringValueInsertAndUpdateFailTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, true);
    SetStringValueInsertAndUpdateFailTest(TEST_KEY2, TEST_VAL2, INVALID_USER_ID, true);
    SetStringValueInsertAndUpdateFailTest(TEST_KEY3, TEST_VAL3, INVALID_USER_ID, true);
    SetStringValueInsertAndUpdateFailTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, false);
    SetStringValueInsertAndUpdateFailTest(TEST_KEY2, TEST_VAL2, INVALID_USER_ID, false);
    SetStringValueInsertAndUpdateFailTest(TEST_KEY3, TEST_VAL3, INVALID_USER_ID, false);

    SetStringValueInsertAndUpdateFailTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, true);
    SetStringValueInsertAndUpdateFailTest(TEST_KEY1, TEST_VAL1, TEST_USER100, true);
    SetStringValueInsertAndUpdateFailTest(TEST_KEY1, TEST_VAL1, TEST_USER1, true);
    SetStringValueInsertAndUpdateFailTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, false);
    SetStringValueInsertAndUpdateFailTest(TEST_KEY1, TEST_VAL1, TEST_USER100, false);
    SetStringValueInsertAndUpdateFailTest(TEST_KEY1, TEST_VAL1, TEST_USER1, false);
}

HWTEST_F(SettingDataManagerTest, SetStringValue_0300, TestSize.Level1)
{
    SetStringValueInsertTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, true);
    SetStringValueInsertTest(TEST_KEY2, TEST_VAL2, INVALID_USER_ID, true);
    SetStringValueInsertTest(TEST_KEY3, TEST_VAL3, INVALID_USER_ID, true);
    SetStringValueInsertTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, false);
    SetStringValueInsertTest(TEST_KEY2, TEST_VAL2, INVALID_USER_ID, false);
    SetStringValueInsertTest(TEST_KEY3, TEST_VAL3, INVALID_USER_ID, false);

    SetStringValueInsertTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, true);
    SetStringValueInsertTest(TEST_KEY1, TEST_VAL1, TEST_USER100, true);
    SetStringValueInsertTest(TEST_KEY1, TEST_VAL1, TEST_USER1, true);
    SetStringValueInsertTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, false);
    SetStringValueInsertTest(TEST_KEY1, TEST_VAL1, TEST_USER100, false);
    SetStringValueInsertTest(TEST_KEY1, TEST_VAL1, TEST_USER1, false);
}

HWTEST_F(SettingDataManagerTest, SetStringValue_0400, TestSize.Level1)
{
    SetStringValueUpdateTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, true);
    SetStringValueUpdateTest(TEST_KEY2, TEST_VAL2, INVALID_USER_ID, true);
    SetStringValueUpdateTest(TEST_KEY3, TEST_VAL3, INVALID_USER_ID, true);
    SetStringValueUpdateTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, false);
    SetStringValueUpdateTest(TEST_KEY2, TEST_VAL2, INVALID_USER_ID, false);
    SetStringValueUpdateTest(TEST_KEY3, TEST_VAL3, INVALID_USER_ID, false);

    SetStringValueUpdateTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, true);
    SetStringValueUpdateTest(TEST_KEY1, TEST_VAL1, TEST_USER100, true);
    SetStringValueUpdateTest(TEST_KEY1, TEST_VAL1, TEST_USER1, true);
    SetStringValueUpdateTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID, false);
    SetStringValueUpdateTest(TEST_KEY1, TEST_VAL1, TEST_USER100, false);
    SetStringValueUpdateTest(TEST_KEY1, TEST_VAL1, TEST_USER1, false);
}

HWTEST_F(SettingDataManagerTest, SetInt32Value_0100, TestSize.Level1)
{
    SetInt32ValueCreateFailTest(TEST_KEY1, 0, INVALID_USER_ID, true);
    SetInt32ValueCreateFailTest(TEST_KEY2, 1, INVALID_USER_ID, true);
    SetInt32ValueCreateFailTest(TEST_KEY3, -1, INVALID_USER_ID, true);
    SetInt32ValueCreateFailTest(TEST_KEY1, 0, INVALID_USER_ID, false);
    SetInt32ValueCreateFailTest(TEST_KEY2, 1, INVALID_USER_ID, false);
    SetInt32ValueCreateFailTest(TEST_KEY3, -1, INVALID_USER_ID, false);

    SetInt32ValueCreateFailTest(TEST_KEY1, 0, INVALID_USER_ID, true);
    SetInt32ValueCreateFailTest(TEST_KEY1, 0, TEST_USER100, true);
    SetInt32ValueCreateFailTest(TEST_KEY1, 0, TEST_USER1, true);
    SetInt32ValueCreateFailTest(TEST_KEY1, 0, INVALID_USER_ID, false);
    SetInt32ValueCreateFailTest(TEST_KEY1, 0, TEST_USER100, false);
    SetInt32ValueCreateFailTest(TEST_KEY1, 0, TEST_USER1, false);
}

HWTEST_F(SettingDataManagerTest, SetInt32Value_0200, TestSize.Level1)
{
    SetInt32ValueInsertAndUpdateFailTest(TEST_KEY1, 0, INVALID_USER_ID, true);
    SetInt32ValueInsertAndUpdateFailTest(TEST_KEY2, 1, INVALID_USER_ID, true);
    SetInt32ValueInsertAndUpdateFailTest(TEST_KEY3, -1, INVALID_USER_ID, true);
    SetInt32ValueInsertAndUpdateFailTest(TEST_KEY1, 0, INVALID_USER_ID, false);
    SetInt32ValueInsertAndUpdateFailTest(TEST_KEY2, 1, INVALID_USER_ID, false);
    SetInt32ValueInsertAndUpdateFailTest(TEST_KEY3, -1, INVALID_USER_ID, false);

    SetInt32ValueInsertAndUpdateFailTest(TEST_KEY1, 0, INVALID_USER_ID, true);
    SetInt32ValueInsertAndUpdateFailTest(TEST_KEY1, 0, TEST_USER100, true);
    SetInt32ValueInsertAndUpdateFailTest(TEST_KEY1, 0, TEST_USER1, true);
    SetInt32ValueInsertAndUpdateFailTest(TEST_KEY1, 0, INVALID_USER_ID, false);
    SetInt32ValueInsertAndUpdateFailTest(TEST_KEY1, 0, TEST_USER100, false);
    SetInt32ValueInsertAndUpdateFailTest(TEST_KEY1, 0, TEST_USER1, false);
}

HWTEST_F(SettingDataManagerTest, SetInt32Value_0300, TestSize.Level1)
{
    SetInt32ValueInsertTest(TEST_KEY1, 0, INVALID_USER_ID, true);
    SetInt32ValueInsertTest(TEST_KEY2, 1, INVALID_USER_ID, true);
    SetInt32ValueInsertTest(TEST_KEY3, -1, INVALID_USER_ID, true);
    SetInt32ValueInsertTest(TEST_KEY1, 0, INVALID_USER_ID, false);
    SetInt32ValueInsertTest(TEST_KEY2, 1, INVALID_USER_ID, false);
    SetInt32ValueInsertTest(TEST_KEY3, -1, INVALID_USER_ID, false);

    SetInt32ValueInsertTest(TEST_KEY1, 0, INVALID_USER_ID, true);
    SetInt32ValueInsertTest(TEST_KEY1, 0, TEST_USER100, true);
    SetInt32ValueInsertTest(TEST_KEY1, 0, TEST_USER1, true);
    SetInt32ValueInsertTest(TEST_KEY1, 0, INVALID_USER_ID, false);
    SetInt32ValueInsertTest(TEST_KEY1, 0, TEST_USER100, false);
    SetInt32ValueInsertTest(TEST_KEY1, 0, TEST_USER1, false);
}

HWTEST_F(SettingDataManagerTest, SetInt32Value_0400, TestSize.Level1)
{
    SetInt32ValueUpdateTest(TEST_KEY1, 0, INVALID_USER_ID, true);
    SetInt32ValueUpdateTest(TEST_KEY2, 1, INVALID_USER_ID, true);
    SetInt32ValueUpdateTest(TEST_KEY3, -1, INVALID_USER_ID, true);
    SetInt32ValueUpdateTest(TEST_KEY1, 0, INVALID_USER_ID, false);
    SetInt32ValueUpdateTest(TEST_KEY2, 1, INVALID_USER_ID, false);
    SetInt32ValueUpdateTest(TEST_KEY3, -1, INVALID_USER_ID, false);

    SetInt32ValueUpdateTest(TEST_KEY1, 0, INVALID_USER_ID, true);
    SetInt32ValueUpdateTest(TEST_KEY1, 0, TEST_USER100, true);
    SetInt32ValueUpdateTest(TEST_KEY1, 0, TEST_USER1, true);
    SetInt32ValueUpdateTest(TEST_KEY1, 0, INVALID_USER_ID, false);
    SetInt32ValueUpdateTest(TEST_KEY1, 0, TEST_USER100, false);
    SetInt32ValueUpdateTest(TEST_KEY1, 0, TEST_USER1, false);
}

HWTEST_F(SettingDataManagerTest, SetBoolValue_0100, TestSize.Level1)
{
    SetBoolValueCreateFailTest(TEST_KEY1, false, INVALID_USER_ID, true);
    SetBoolValueCreateFailTest(TEST_KEY2, true, INVALID_USER_ID, true);
    SetBoolValueCreateFailTest(TEST_KEY1, false, INVALID_USER_ID, false);
    SetBoolValueCreateFailTest(TEST_KEY2, true, INVALID_USER_ID, false);

    SetBoolValueCreateFailTest(TEST_KEY1, true, INVALID_USER_ID, true);
    SetBoolValueCreateFailTest(TEST_KEY1, true, TEST_USER100, true);
    SetBoolValueCreateFailTest(TEST_KEY1, true, TEST_USER1, true);
    SetBoolValueCreateFailTest(TEST_KEY1, true, INVALID_USER_ID, false);
    SetBoolValueCreateFailTest(TEST_KEY1, true, TEST_USER100, false);
    SetBoolValueCreateFailTest(TEST_KEY1, true, TEST_USER1, false);
}

HWTEST_F(SettingDataManagerTest, SetBoolValue_0200, TestSize.Level1)
{
    SetBoolValueInsertAndUpdateFailTest(TEST_KEY1, false, INVALID_USER_ID, true);
    SetBoolValueInsertAndUpdateFailTest(TEST_KEY2, true, INVALID_USER_ID, true);
    SetBoolValueInsertAndUpdateFailTest(TEST_KEY1, false, INVALID_USER_ID, false);
    SetBoolValueInsertAndUpdateFailTest(TEST_KEY2, true, INVALID_USER_ID, false);

    SetBoolValueInsertAndUpdateFailTest(TEST_KEY1, true, INVALID_USER_ID, true);
    SetBoolValueInsertAndUpdateFailTest(TEST_KEY1, true, TEST_USER100, true);
    SetBoolValueInsertAndUpdateFailTest(TEST_KEY1, true, TEST_USER1, true);
    SetBoolValueInsertAndUpdateFailTest(TEST_KEY1, true, INVALID_USER_ID, false);
    SetBoolValueInsertAndUpdateFailTest(TEST_KEY1, true, TEST_USER100, false);
    SetBoolValueInsertAndUpdateFailTest(TEST_KEY1, true, TEST_USER1, false);
}

HWTEST_F(SettingDataManagerTest, SetBoolValue_0300, TestSize.Level1)
{
    SetBoolValueInsertTest(TEST_KEY1, false, INVALID_USER_ID, true);
    SetBoolValueInsertTest(TEST_KEY2, true, INVALID_USER_ID, true);
    SetBoolValueInsertTest(TEST_KEY1, false, INVALID_USER_ID, false);
    SetBoolValueInsertTest(TEST_KEY2, true, INVALID_USER_ID, false);

    SetBoolValueInsertTest(TEST_KEY1, true, INVALID_USER_ID, true);
    SetBoolValueInsertTest(TEST_KEY1, true, TEST_USER100, true);
    SetBoolValueInsertTest(TEST_KEY1, true, TEST_USER1, true);
    SetBoolValueInsertTest(TEST_KEY1, true, INVALID_USER_ID, false);
    SetBoolValueInsertTest(TEST_KEY1, true, TEST_USER100, false);
    SetBoolValueInsertTest(TEST_KEY1, true, TEST_USER1, false);
}

HWTEST_F(SettingDataManagerTest, SetBoolValue_0400, TestSize.Level1)
{
    SetBoolValueUpdateTest(TEST_KEY1, false, INVALID_USER_ID, true);
    SetBoolValueUpdateTest(TEST_KEY2, true, INVALID_USER_ID, true);
    SetBoolValueUpdateTest(TEST_KEY1, false, INVALID_USER_ID, false);
    SetBoolValueUpdateTest(TEST_KEY2, true, INVALID_USER_ID, false);

    SetBoolValueUpdateTest(TEST_KEY1, true, INVALID_USER_ID, true);
    SetBoolValueUpdateTest(TEST_KEY1, true, TEST_USER100, true);
    SetBoolValueUpdateTest(TEST_KEY1, true, TEST_USER1, true);
    SetBoolValueUpdateTest(TEST_KEY1, true, INVALID_USER_ID, false);
    SetBoolValueUpdateTest(TEST_KEY1, true, TEST_USER100, false);
    SetBoolValueUpdateTest(TEST_KEY1, true, TEST_USER1, false);
}

HWTEST_F(SettingDataManagerTest, GetStringValue_0100, TestSize.Level1)
{
    GetStringValueCreateFailTest(TEST_KEY1, INVALID_USER_ID);
    GetStringValueCreateFailTest(TEST_KEY2, INVALID_USER_ID);
    GetStringValueCreateFailTest(TEST_KEY3, INVALID_USER_ID);
    GetStringValueCreateFailTest(TEST_KEY1, INVALID_USER_ID);
    GetStringValueCreateFailTest(TEST_KEY1, TEST_USER100);
    GetStringValueCreateFailTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, GetStringValue_0200, TestSize.Level1)
{
    GetStringValueQueryNullTest(TEST_KEY1, INVALID_USER_ID);
    GetStringValueQueryNullTest(TEST_KEY2, INVALID_USER_ID);
    GetStringValueQueryNullTest(TEST_KEY3, INVALID_USER_ID);
    GetStringValueQueryNullTest(TEST_KEY1, INVALID_USER_ID);
    GetStringValueQueryNullTest(TEST_KEY1, TEST_USER100);
    GetStringValueQueryNullTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, GetStringValue_0300, TestSize.Level1)
{
    GetStringValueResultNotFoundTest(TEST_KEY1, INVALID_USER_ID);
    GetStringValueResultNotFoundTest(TEST_KEY2, INVALID_USER_ID);
    GetStringValueResultNotFoundTest(TEST_KEY3, INVALID_USER_ID);
    GetStringValueResultNotFoundTest(TEST_KEY1, INVALID_USER_ID);
    GetStringValueResultNotFoundTest(TEST_KEY1, TEST_USER100);
    GetStringValueResultNotFoundTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, GetStringValue_0400, TestSize.Level1)
{
    GetStringValueResultGetStringFailTest(TEST_KEY1, INVALID_USER_ID);
    GetStringValueResultGetStringFailTest(TEST_KEY2, INVALID_USER_ID);
    GetStringValueResultGetStringFailTest(TEST_KEY3, INVALID_USER_ID);
    GetStringValueResultGetStringFailTest(TEST_KEY1, INVALID_USER_ID);
    GetStringValueResultGetStringFailTest(TEST_KEY1, TEST_USER100);
    GetStringValueResultGetStringFailTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, GetStringValue_0500, TestSize.Level1)
{
    GetStringValueResultTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID);
    GetStringValueResultTest(TEST_KEY2, TEST_VAL2, INVALID_USER_ID);
    GetStringValueResultTest(TEST_KEY3, TEST_VAL3, INVALID_USER_ID);
    GetStringValueResultTest(TEST_KEY1, TEST_VAL1, INVALID_USER_ID);
    GetStringValueResultTest(TEST_KEY1, TEST_VAL1, TEST_USER100);
    GetStringValueResultTest(TEST_KEY1, TEST_VAL1, TEST_USER1);

    GetStringValueResultTest(TEST_KEY1, "", INVALID_USER_ID);
}

HWTEST_F(SettingDataManagerTest, GetInt32Value_0100, TestSize.Level1)
{
    GetInt32ValueCreateFailTest(TEST_KEY1, INVALID_USER_ID);
    GetInt32ValueCreateFailTest(TEST_KEY2, INVALID_USER_ID);
    GetInt32ValueCreateFailTest(TEST_KEY3, INVALID_USER_ID);
    GetInt32ValueCreateFailTest(TEST_KEY1, INVALID_USER_ID);
    GetInt32ValueCreateFailTest(TEST_KEY1, TEST_USER100);
    GetInt32ValueCreateFailTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, GetInt32Value_0200, TestSize.Level1)
{
    GetInt32ValueQueryNullTest(TEST_KEY1, INVALID_USER_ID);
    GetInt32ValueQueryNullTest(TEST_KEY2, INVALID_USER_ID);
    GetInt32ValueQueryNullTest(TEST_KEY3, INVALID_USER_ID);
    GetInt32ValueQueryNullTest(TEST_KEY1, INVALID_USER_ID);
    GetInt32ValueQueryNullTest(TEST_KEY1, TEST_USER100);
    GetInt32ValueQueryNullTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, GetInt32Value_0300, TestSize.Level1)
{
    GetInt32ValueResultNotFoundTest(TEST_KEY1, INVALID_USER_ID);
    GetInt32ValueResultNotFoundTest(TEST_KEY2, INVALID_USER_ID);
    GetInt32ValueResultNotFoundTest(TEST_KEY3, INVALID_USER_ID);
    GetInt32ValueResultNotFoundTest(TEST_KEY1, INVALID_USER_ID);
    GetInt32ValueResultNotFoundTest(TEST_KEY1, TEST_USER100);
    GetInt32ValueResultNotFoundTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, GetInt32Value_0400, TestSize.Level1)
{
    GetInt32ValueResultGetStringFailTest(TEST_KEY1, INVALID_USER_ID);
    GetInt32ValueResultGetStringFailTest(TEST_KEY2, INVALID_USER_ID);
    GetInt32ValueResultGetStringFailTest(TEST_KEY3, INVALID_USER_ID);
    GetInt32ValueResultGetStringFailTest(TEST_KEY1, INVALID_USER_ID);
    GetInt32ValueResultGetStringFailTest(TEST_KEY1, TEST_USER100);
    GetInt32ValueResultGetStringFailTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, GetInt32Value_0500, TestSize.Level1)
{
    GetInt32ValueResultTest(TEST_KEY1, INVALID_USER_ID, 0, "0", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY2, INVALID_USER_ID, 1, "1", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY3, INVALID_USER_ID, -1, "-1", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, INVALID_USER_ID, 0, "0", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER100, 0, "0", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 0, "0", ERR_OK);

    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 0, "abcdefghijklmnopqrstuvwxyz", ERR_INVALID_VALUE);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 0, "!@#$%^&*()_+-={}[]<,>.?/:;", ERR_INVALID_VALUE);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 0, "-9999999999999999999999999", ERR_INVALID_VALUE);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 0, "99999999999999999999999999", ERR_INVALID_VALUE);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 0, "abcdefghijkl99999999999999", ERR_INVALID_VALUE);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 0, "abc999", ERR_INVALID_VALUE);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 0, "", ERR_INVALID_VALUE);

    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 999, "999abc", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 1, "1e10", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 1, "1e-10", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, -1, "-1e10", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, -1, "-1e-10", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 0, "0.000000", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 0, "0.000001", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 0, "0.5", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 0, "0.999999", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 1, "1.000001", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 1, "1.5", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 1, "1.999999", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 2, "2.000000", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 2, "2.000001", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 2, "2.5", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 2, "2.999999", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 3, "3.000000", ERR_OK);
    GetInt32ValueResultTest(TEST_KEY1, TEST_USER1, 3, "3.000001", ERR_OK);
}

HWTEST_F(SettingDataManagerTest, GetInt32Value_0600, TestSize.Level1)
{
    GetInt32ValueStrictlyResultTest(TEST_KEY1, INVALID_USER_ID, 0, "0", ERR_OK);
    GetInt32ValueStrictlyResultTest(TEST_KEY2, INVALID_USER_ID, 1, "1", ERR_OK);
    GetInt32ValueStrictlyResultTest(TEST_KEY3, INVALID_USER_ID, -1, "-1", ERR_OK);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, INVALID_USER_ID, 0, "0", ERR_OK);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER100, 0, "0", ERR_OK);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "0", ERR_OK);

    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "abcdefghijklmnopqrstuvwxyz", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "!@#$%^&*()_+-={}[]<,>.?/:;", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "-9999999999999999999999999", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "99999999999999999999999999", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "abcdefghijkl99999999999999", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "abc999", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "", ERR_INVALID_VALUE);

    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "999abc", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "1e10", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "1e-10", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "-1e10", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "-1e-10", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "0.000000", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "0.000001", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "0.5", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "0.999999", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "1.000001", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "1.5", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "1.999999", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "2.000000", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "2.000001", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "2.5", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "2.999999", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "3.000000", ERR_INVALID_VALUE);
    GetInt32ValueStrictlyResultTest(TEST_KEY1, TEST_USER1, 0, "3.000001", ERR_INVALID_VALUE);
}

HWTEST_F(SettingDataManagerTest, GetBoolValue_0100, TestSize.Level1)
{
    GetBoolValueCreateFailTest(TEST_KEY1, INVALID_USER_ID);
    GetBoolValueCreateFailTest(TEST_KEY2, INVALID_USER_ID);
    GetBoolValueCreateFailTest(TEST_KEY3, INVALID_USER_ID);
    GetBoolValueCreateFailTest(TEST_KEY1, INVALID_USER_ID);
    GetBoolValueCreateFailTest(TEST_KEY1, TEST_USER100);
    GetBoolValueCreateFailTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, GetBoolValue_0200, TestSize.Level1)
{
    GetBoolValueQueryNullTest(TEST_KEY1, INVALID_USER_ID);
    GetBoolValueQueryNullTest(TEST_KEY2, INVALID_USER_ID);
    GetBoolValueQueryNullTest(TEST_KEY3, INVALID_USER_ID);
    GetBoolValueQueryNullTest(TEST_KEY1, INVALID_USER_ID);
    GetBoolValueQueryNullTest(TEST_KEY1, TEST_USER100);
    GetBoolValueQueryNullTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, GetBoolValue_0300, TestSize.Level1)
{
    GetBoolValueResultNotFoundTest(TEST_KEY1, INVALID_USER_ID);
    GetBoolValueResultNotFoundTest(TEST_KEY2, INVALID_USER_ID);
    GetBoolValueResultNotFoundTest(TEST_KEY3, INVALID_USER_ID);
    GetBoolValueResultNotFoundTest(TEST_KEY1, INVALID_USER_ID);
    GetBoolValueResultNotFoundTest(TEST_KEY1, TEST_USER100);
    GetBoolValueResultNotFoundTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, GetBoolValue_0400, TestSize.Level1)
{
    GetBoolValueResultGetStringFailTest(TEST_KEY1, INVALID_USER_ID);
    GetBoolValueResultGetStringFailTest(TEST_KEY2, INVALID_USER_ID);
    GetBoolValueResultGetStringFailTest(TEST_KEY3, INVALID_USER_ID);
    GetBoolValueResultGetStringFailTest(TEST_KEY1, INVALID_USER_ID);
    GetBoolValueResultGetStringFailTest(TEST_KEY1, TEST_USER100);
    GetBoolValueResultGetStringFailTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, GetBoolValue_0500, TestSize.Level1)
{
    GetBoolValueResultTest(TEST_KEY1, INVALID_USER_ID, true, "true", ERR_OK);
    GetBoolValueResultTest(TEST_KEY2, INVALID_USER_ID, false, "false", ERR_OK);
    GetBoolValueResultTest(TEST_KEY3, INVALID_USER_ID, true, "true", ERR_OK);
    GetBoolValueResultTest(TEST_KEY1, INVALID_USER_ID, false, "false", ERR_OK);
    GetBoolValueResultTest(TEST_KEY1, TEST_USER100, true, "true", ERR_OK);
    GetBoolValueResultTest(TEST_KEY1, TEST_USER1, false, "false", ERR_OK);

    GetBoolValueResultTest(TEST_KEY1, TEST_USER1, false, "false1", ERR_INVALID_VALUE);
    GetBoolValueResultTest(TEST_KEY1, TEST_USER1, false, "fals", ERR_INVALID_VALUE);
    GetBoolValueResultTest(TEST_KEY1, TEST_USER1, false, "1false", ERR_INVALID_VALUE);
    GetBoolValueResultTest(TEST_KEY1, TEST_USER1, false, "true1", ERR_INVALID_VALUE);
    GetBoolValueResultTest(TEST_KEY1, TEST_USER1, false, "tru", ERR_INVALID_VALUE);
    GetBoolValueResultTest(TEST_KEY1, TEST_USER1, false, "1true", ERR_INVALID_VALUE);
    GetBoolValueResultTest(TEST_KEY1, TEST_USER1, false, "", ERR_INVALID_VALUE);
}

HWTEST_F(SettingDataManagerTest, IsValidKey_0100, TestSize.Level1)
{
    IsValidKeyCreateFailTest(TEST_KEY1, INVALID_USER_ID);
    IsValidKeyCreateFailTest(TEST_KEY2, INVALID_USER_ID);
    IsValidKeyCreateFailTest(TEST_KEY3, INVALID_USER_ID);
    IsValidKeyCreateFailTest(TEST_KEY1, INVALID_USER_ID);
    IsValidKeyCreateFailTest(TEST_KEY1, TEST_USER100);
    IsValidKeyCreateFailTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, IsValidKey_0200, TestSize.Level1)
{
    IsValidKeyQueryNullTest(TEST_KEY1, INVALID_USER_ID);
    IsValidKeyQueryNullTest(TEST_KEY2, INVALID_USER_ID);
    IsValidKeyQueryNullTest(TEST_KEY3, INVALID_USER_ID);
    IsValidKeyQueryNullTest(TEST_KEY1, INVALID_USER_ID);
    IsValidKeyQueryNullTest(TEST_KEY1, TEST_USER100);
    IsValidKeyQueryNullTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, IsValidKey_0300, TestSize.Level1)
{
    IsValidKeyResultNotFoundTest(TEST_KEY1, INVALID_USER_ID);
    IsValidKeyResultNotFoundTest(TEST_KEY2, INVALID_USER_ID);
    IsValidKeyResultNotFoundTest(TEST_KEY3, INVALID_USER_ID);
    IsValidKeyResultNotFoundTest(TEST_KEY1, INVALID_USER_ID);
    IsValidKeyResultNotFoundTest(TEST_KEY1, TEST_USER100);
    IsValidKeyResultNotFoundTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, IsValidKey_0400, TestSize.Level1)
{
    IsValidKeyResultGetStringFailTest(TEST_KEY1, INVALID_USER_ID);
    IsValidKeyResultGetStringFailTest(TEST_KEY2, INVALID_USER_ID);
    IsValidKeyResultGetStringFailTest(TEST_KEY3, INVALID_USER_ID);
    IsValidKeyResultGetStringFailTest(TEST_KEY1, INVALID_USER_ID);
    IsValidKeyResultGetStringFailTest(TEST_KEY1, TEST_USER100);
    IsValidKeyResultGetStringFailTest(TEST_KEY1, TEST_USER1);
}

HWTEST_F(SettingDataManagerTest, IsValidKey_0500, TestSize.Level1)
{
    IsValidKeyResultTest(TEST_KEY1, INVALID_USER_ID, true, TEST_VAL1);
    IsValidKeyResultTest(TEST_KEY2, INVALID_USER_ID, true, TEST_VAL2);
    IsValidKeyResultTest(TEST_KEY3, INVALID_USER_ID, true, TEST_VAL3);
    IsValidKeyResultTest(TEST_KEY1, INVALID_USER_ID, true, TEST_VAL1);
    IsValidKeyResultTest(TEST_KEY1, TEST_USER100, true, TEST_VAL1);
    IsValidKeyResultTest(TEST_KEY1, TEST_USER1, true, TEST_VAL1);
    IsValidKeyResultTest(TEST_KEY1, TEST_USER1, false, "");
}
} // namespace OHOS::ArkUi::UiAppearance
