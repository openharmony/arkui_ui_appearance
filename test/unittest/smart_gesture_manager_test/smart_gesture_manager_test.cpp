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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

// mock
#include "alarm_timer_manager.h"
#include "setting_data_manager.h"

#define private public
#include "smart_gesture_manager.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS::ArkUi::UiAppearance {
namespace {
const std::string SETTING_SMART_GESTURE_SWITCH_KEY = "persist.gesture.smart_gesture_enable";
}

class SmartGestureManagerTest : public Test {
protected:
    void SetUp() override
    {
        SmartGestureManager& manager = SmartGestureManager::GetInstance();
        EXPECT_EQ(manager.Initialize([this](const bool isAutoMode, const int32_t userId) {
            UpdateCallback(isAutoMode, userId);
            }), ERR_OK);
    }

    void TearDown() override
    {
        SmartGestureManager& manager = SmartGestureManager::GetInstance();
        manager.updateCallback_ = nullptr;
    }

    void UpdateCallback(bool isAutoMode, int32_t userId) {}
};

/**
 * @tc.name: LoadSettingDataObserver_0100
 * @tc.desc: Test LoadSettingDataObserversCallback
 * @tc.type: FUNC
 */
HWTEST_F(SmartGestureManagerTest, LoadSettingDataObserver_0100, TestSize.Level1)
{
    SmartGestureManager& manager = SmartGestureManager::GetInstance();
    manager.observer_.first = "";
    manager.LoadSettingDataObserversCallback();
    manager.UpdateSmartGestureInitialValue();
    manager.UpdateSmartGestureValue(SETTING_SMART_GESTURE_SWITCH_KEY, INVALID_USER_ID);
    EXPECT_EQ(manager.observer_.first, SETTING_SMART_GESTURE_SWITCH_KEY);
}

/**
 * @tc.name: RegisterSettingDataObserver_0100
 * @tc.desc: Test OnChangeSmartGestureMode after register observer
 * @tc.type: FUNC
 */
HWTEST_F(SmartGestureManagerTest, RegisterSettingDataObserver_0100, TestSize.Level1)
{
    SmartGestureManager& manager = SmartGestureManager::GetInstance();
    auto result = manager.RegisterSettingDataObserver();
    manager.OnChangeSmartGestureMode(SmartGestureManager::SmartGestureMode::SMART_GESTURE_DISABLE, INVALID_USER_ID);
    manager.updateCallback_ = nullptr;
    manager.OnChangeSmartGestureMode(SmartGestureManager::SmartGestureMode::SMART_GESTURE_DISABLE, INVALID_USER_ID);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: RegisterSettingDataObserver_0100
 * @tc.desc: Test register setting data observer registration failure exception
 * @tc.type: FUNC
 */
HWTEST_F(SmartGestureManagerTest, RegisterSettingDataObserver_0200, TestSize.Level1)
{
    SettingDataManager& settingDataManager = SettingDataManager::GetInstance();
    ExpectationSet expectSet;
    expectSet += EXPECT_CALL(
        settingDataManager, MockRegisterObserver(SETTING_SMART_GESTURE_SWITCH_KEY, _, INVALID_USER_ID))
        .Times(1).After(expectSet).WillOnce(Return(ERR_INVALID_OPERATION));
    SmartGestureManager& manager = SmartGestureManager::GetInstance();
    auto result = manager.RegisterSettingDataObserver();
    EXPECT_EQ(result, ERR_NO_INIT);
}
} // namespace OHOS::ArkUi::UiAppearance
