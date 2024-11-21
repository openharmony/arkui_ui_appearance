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

#include "setting_data_observer.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::ArkUi::UiAppearance {
namespace {
constexpr const char* TEST_KEY1 = "testKey1";
constexpr const char* TEST_KEY2 = "testKey2";
constexpr int32_t TEST_USER100 = 100;
constexpr int32_t TEST_USER1 = 1;
}

class SettingDataObserverTest : public Test {
};

HWTEST_F(SettingDataObserverTest, KeyTest_0100, TestSize.Level1)
{
    SettingDataObserver observer;
    EXPECT_EQ(observer.GetKey(), "");
    observer.SetKey(TEST_KEY1);
    EXPECT_EQ(observer.GetKey(), TEST_KEY1);
    observer.SetKey(TEST_KEY1);
    EXPECT_EQ(observer.GetKey(), TEST_KEY1);
    observer.SetKey(TEST_KEY2);
    EXPECT_EQ(observer.GetKey(), TEST_KEY2);
    observer.SetKey("");
    EXPECT_EQ(observer.GetKey(), "");
}

HWTEST_F(SettingDataObserverTest, UserIdTest_0100, TestSize.Level1)
{
    SettingDataObserver observer;
    EXPECT_EQ(observer.GetUserId(), INVALID_USER_ID);
    observer.SetUserId(TEST_USER100);
    EXPECT_EQ(observer.GetUserId(), TEST_USER100);
    observer.SetUserId(TEST_USER100);
    EXPECT_EQ(observer.GetUserId(), TEST_USER100);
    observer.SetUserId(TEST_USER1);
    EXPECT_EQ(observer.GetUserId(), TEST_USER1);
    observer.SetUserId(INVALID_USER_ID);
    EXPECT_EQ(observer.GetUserId(), INVALID_USER_ID);
}

HWTEST_F(SettingDataObserverTest, UpdateFuncTest_0100, TestSize.Level1)
{
    SettingDataObserver observer;
    observer.OnChange();

    int32_t userIdGet = TEST_USER100;
    std::string keyGet;
    bool isCalled = false;

    observer.SetUpdateFunc([&](const std::string& key, int32_t userId) {
        keyGet = key;
        userIdGet = userId;
        isCalled = true;
    });
    EXPECT_EQ(userIdGet, TEST_USER100);
    EXPECT_EQ(keyGet, "");
    EXPECT_EQ(isCalled, false);

    observer.OnChange();
    EXPECT_EQ(userIdGet, INVALID_USER_ID);
    EXPECT_EQ(keyGet, "");
    EXPECT_EQ(isCalled, true);
}

HWTEST_F(SettingDataObserverTest, UpdateFuncTest_0200, TestSize.Level1)
{
    SettingDataObserver observer;
    int32_t userIdGet = TEST_USER100;
    std::string keyGet;
    bool isCalled = false;

    observer.SetUpdateFunc([&](const std::string& key, int32_t userId) {
        keyGet = key;
        userIdGet = userId;
        isCalled = true;
    });
    observer.SetKey(TEST_KEY1);
    observer.SetUserId(TEST_USER1);
    EXPECT_EQ(userIdGet, TEST_USER100);
    EXPECT_EQ(keyGet, "");
    EXPECT_EQ(isCalled, false);

    observer.OnChange();
    EXPECT_EQ(userIdGet, TEST_USER1);
    EXPECT_EQ(keyGet, TEST_KEY1);
    EXPECT_EQ(isCalled, true);

    userIdGet = INVALID_USER_ID;
    keyGet = "";
    isCalled = false;
    observer.SetKey(TEST_KEY2);
    observer.SetUserId(TEST_USER100);
    observer.OnChange();
    EXPECT_EQ(userIdGet, TEST_USER100);
    EXPECT_EQ(keyGet, TEST_KEY2);
    EXPECT_EQ(isCalled, true);
}

HWTEST_F(SettingDataObserverTest, UpdateFuncTest_0300, TestSize.Level1)
{
    SettingDataObserver observer;
    observer.OnChange();

    int32_t userIdGet = TEST_USER100;
    std::string keyGet;
    bool isCalled = false;

    observer.SetUpdateFunc([&](const std::string& key, int32_t userId) {
        keyGet = key;
        userIdGet = userId;
        isCalled = true;
    });
    observer.SetKey(TEST_KEY1);
    observer.SetUserId(TEST_USER1);
    EXPECT_EQ(userIdGet, TEST_USER100);
    EXPECT_EQ(keyGet, "");
    EXPECT_EQ(isCalled, false);

    observer.SetUpdateFunc([&](const std::string& key, int32_t userId) {
        isCalled = true;
    });
    observer.OnChange();
    EXPECT_EQ(userIdGet, TEST_USER100);
    EXPECT_EQ(keyGet, "");
    EXPECT_EQ(isCalled, true);
}
} // namespace OHOS::ArkUi::UiAppearance
