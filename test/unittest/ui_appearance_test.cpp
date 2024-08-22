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

using namespace testing::ext;
static constexpr int UISERVER_UID = 3050;

namespace OHOS {
namespace ArkUi::UiAppearance {
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
} // namespace ArkUi::UiAppearance
} // namespace OHOS
