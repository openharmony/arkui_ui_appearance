/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include <chrono>
#include <cstdio>
#include <gtest/gtest.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <string>

#include "accesstoken_kit.h"
#include "syspara/parameter.h"
#include "system_ability_definition.h"
#include "ui_appearance_ability.h"
#include "ui_appearance_log.h"
#include "alarm_timer_manager.h"

using namespace testing::ext;
static constexpr int UISERVER_UID = 3050;

namespace OHOS {
namespace ArkUi::UiAppearance {

const int DAY_TO_SECOND = 24 * 60 * 60;
const int DAY_TO_MINUTE = 24 * 60;
const int SECOND_TO_MILLI = 1000;
const int MINUTE_TO_SECOND = 60;

class UiAppearanceAbilityTest : public UiAppearanceAbility {
public:
    UiAppearanceAbilityTest() : UiAppearanceAbility(ARKUI_UI_APPEARANCE_SERVICE_ID, true) {}
    ~UiAppearanceAbilityTest() {}
    void OnStart() override
    {
        return;
    }
};

class DarkModeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static sptr<UiAppearanceAbilityTest> GetUiAppearanceAbilityTest()
    {
        return new UiAppearanceAbilityTest;
    }

    static std::shared_ptr<AlarmTimerManager> GetAlarmTimerManager()
    {
        return std::make_shared<AlarmTimerManager>();
    }

private:
    int userId_;
};

void DarkModeTest::SetUpTestCase(void) {}

void DarkModeTest::TearDownTestCase(void) {}

void DarkModeTest::SetUp(void)
{
    userId_ = geteuid();
    seteuid(UISERVER_UID);
}

void DarkModeTest::TearDown(void)
{
    seteuid(userId_);
}

/**
 * @tc.name: ui_appearance_test_001
 * @tc.desc: Test SetDarkMode and GetDarkMode APIs when setting dark/light.
 * @tc.type: FUNC
 */
HWTEST_F(DarkModeTest, ui_appearance_test_001, TestSize.Level0)
{
    LOGI("Test SetDarkMode and GetDarkMode APIs when setting dark/light.");

    auto test = DarkModeTest::GetUiAppearanceAbilityTest();
    auto result = test->SetDarkMode(UiAppearanceAbilityInterface::DarkMode::ALWAYS_DARK);
    EXPECT_EQ(result, 0);
    auto mode = test->GetDarkMode();
    EXPECT_EQ(mode, UiAppearanceAbilityInterface::DarkMode::ALWAYS_DARK);

    result = test->SetDarkMode(UiAppearanceAbilityInterface::DarkMode::ALWAYS_LIGHT);
    EXPECT_EQ(result, 0);
    mode = test->GetDarkMode();
    EXPECT_EQ(mode, UiAppearanceAbilityInterface::DarkMode::ALWAYS_LIGHT);
}

/**
 * @tc.name: ui_appearance_test_002
 * @tc.desc: Test SetDarkMode and GetDarkMode APIs when repeatedly setting dark/light.
 * @tc.type: FUNC
 */
HWTEST_F(DarkModeTest, ui_appearance_test_002, TestSize.Level0)
{
    LOGI("Test SetDarkMode and GetDarkMode APIs when repeatedly setting dark/light.");

    auto test = DarkModeTest::GetUiAppearanceAbilityTest();
    auto result = test->SetDarkMode(UiAppearanceAbilityInterface::DarkMode::ALWAYS_DARK);
    EXPECT_EQ(result, UiAppearanceAbilityInterface::ErrCode::SUCCEEDED);
    auto mode = test->GetDarkMode();
    EXPECT_EQ(mode, UiAppearanceAbilityInterface::DarkMode::ALWAYS_DARK);

    result = test->SetDarkMode(UiAppearanceAbilityInterface::DarkMode::ALWAYS_DARK);
    EXPECT_EQ(result, UiAppearanceAbilityInterface::ErrCode::SYS_ERR);
    mode = test->GetDarkMode();
    EXPECT_EQ(mode, UiAppearanceAbilityInterface::DarkMode::ALWAYS_DARK);

    result = test->SetDarkMode(UiAppearanceAbilityInterface::DarkMode::ALWAYS_LIGHT);
    EXPECT_EQ(result, UiAppearanceAbilityInterface::ErrCode::SUCCEEDED);
    mode = test->GetDarkMode();
    EXPECT_EQ(mode, UiAppearanceAbilityInterface::DarkMode::ALWAYS_LIGHT);

    result = test->SetDarkMode(UiAppearanceAbilityInterface::DarkMode::ALWAYS_LIGHT);
    EXPECT_EQ(result, UiAppearanceAbilityInterface::ErrCode::SYS_ERR);
    mode = test->GetDarkMode();
    EXPECT_EQ(mode, UiAppearanceAbilityInterface::DarkMode::ALWAYS_LIGHT);
}

/**
 * @tc.name: ui_appearance_test_003
 * @tc.desc: Test the SetDarkMode API when setting an unexpected value
 * @tc.type: FUNC
 */
HWTEST_F(DarkModeTest, ui_appearance_test_003, TestSize.Level0)
{
    LOGI("Test the SetDarkMode API when setting an unexpected value.");

    int result =
        DarkModeTest::GetUiAppearanceAbilityTest()->SetDarkMode(UiAppearanceAbilityInterface::DarkMode::UNKNOWN);
    EXPECT_NE(result, 0);
}

/**
 * @tc.name: ui_appearance_test_004
 * @tc.desc: Test the font API
 * @tc.type: FUNC
 */
HWTEST_F(DarkModeTest, ui_appearance_test_004, TestSize.Level0)
{
    LOGI("Test the font API");

    std::string fontScale;
    int result =
        DarkModeTest::GetUiAppearanceAbilityTest()->GetFontScale(fontScale);
    EXPECT_EQ(result, 0);

    std::string fontWeightScale;
    result =
        DarkModeTest::GetUiAppearanceAbilityTest()->GetFontWeightScale(fontWeightScale);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: ui_appearance_test_005
 * @tc.desc: Test the alarm_timer_manager
 * @tc.type: FUNC
 */
HWTEST_F(DarkModeTest, ui_appearance_test_005, TestSize.Level0)
{
    LOGI("Test the alarm_timer_manager");

    bool result = AlarmTimerManager::IsValidScheduleTime(12, 10);
    EXPECT_EQ(result, false);

    result = AlarmTimerManager::IsValidScheduleTime(DAY_TO_MINUTE + 10, DAY_TO_MINUTE + 12);
    EXPECT_EQ(result, false);

    result = AlarmTimerManager::IsValidScheduleTime(10, DAY_TO_MINUTE + 12);
    EXPECT_EQ(result, false);

    result = AlarmTimerManager::IsValidScheduleTime(10, 12);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: ui_appearance_test_006
 * @tc.desc: Test the alarm_timer_manager
 * @tc.type: FUNC
 */
HWTEST_F(DarkModeTest, ui_appearance_test_006, TestSize.Level0)
{
    LOGI("Test the alarm_timer_manager");

    std::array<uint64_t, TRIGGER_ARRAY_SIZE> triggerTimeInterval = {0, 0};
    std::time_t timestamp = std::time(nullptr);
    if (timestamp == static_cast<std::time_t>(-1)) {
        LOGE("fail to get timestamp");
    }
    std::tm *nowTime = std::localtime(&timestamp);
    auto current = nowTime->tm_hour * 60 + nowTime->tm_min;
    if (nowTime != nullptr) {
        nowTime->tm_hour = 0;
        nowTime->tm_min = 0;
        nowTime->tm_sec = 0;
    }
    std::time_t now_zero = std::mktime(nowTime);
    uint64_t zeroTimestamp = static_cast<uint64_t>(now_zero * SECOND_TO_MILLI);

    uint64_t step = DAY_TO_SECOND * SECOND_TO_MILLI;

    uint64_t startTimestamp = zeroTimestamp + (current + 1) * MINUTE_TO_SECOND * SECOND_TO_MILLI;
    AlarmTimerManager::SetTimerTriggerTime(current + 1, current + 2, triggerTimeInterval);
    EXPECT_EQ(triggerTimeInterval[0], startTimestamp);

    startTimestamp = zeroTimestamp + (current - 1) * MINUTE_TO_SECOND * SECOND_TO_MILLI;
    AlarmTimerManager::SetTimerTriggerTime(current - 1, current + 2, triggerTimeInterval);
    EXPECT_EQ(triggerTimeInterval[0], startTimestamp + step);
}

/**
 * @tc.name: ui_appearance_test_007
 * @tc.desc: Test the alarm_timer_manager
 * @tc.type: FUNC
 */
HWTEST_F(DarkModeTest, ui_appearance_test_007, TestSize.Level0)
{
    LOGI("Test the alarm_timer_manager");

    auto uiAppearanceTimerManager = DarkModeTest::GetAlarmTimerManager();
    int res = uiAppearanceTimerManager->SetScheduleTime(
            10, 12, 100, [](){}, [](){});
    EXPECT_EQ(res, 0);

    res = uiAppearanceTimerManager->RestartAllTimer();
    EXPECT_EQ(res, 1);

    uiAppearanceTimerManager->ClearTimerByUserId(100);
}


} // namespace ArkUi::UiAppearance
} // namespace OHOS
