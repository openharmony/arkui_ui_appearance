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

#ifndef UI_APPEARANCE_DARK_MODE_TEMP_STATE_MANAGER_H
#define UI_APPEARANCE_DARK_MODE_TEMP_STATE_MANAGER_H

#include <functional>
#include <vector>

#include "errors.h"
#include "map"
#include "mutex"

namespace OHOS::ArkUi::UiAppearance {
class TemporaryColorModeManager {
public:
    void InitData(const int32_t userId);
    bool IsColorModeTemporary(const int32_t userId);
    bool IsColorModeNormal(const int32_t userId);
    bool SetColorModeTemporary(const int32_t userId);
    bool SetColorModeNormal(const int32_t userId);
    bool CheckTemporaryStateEffective(const int32_t userId);

private:
    std::string TemporaryColorModeAssignUser(const int32_t userId);
    std::string TemporaryStateStartTimeAssignUser(const int32_t userId);
    std::string TemporaryStateEndTimeAssignUser(const int32_t userId);
    static bool IsWithInPreInterval(const int32_t startTime, const int32_t endTime);
    static void GetTempColorModeTimeInfo(const int32_t settingStartTime, const int32_t settingEndTime,
        int64_t& tempStateStartTime, int64_t& tempStateEndTime);
    void SaveTempColorModeInfo(const int32_t userId);
    enum class TempColorModeType {
        ColorModeNormal = 0,
        ColorModeTemp,
    };
    struct TempColorModeInfo {
        TempColorModeType tempColorMode = TempColorModeType::ColorModeNormal;
        int64_t keepTemporaryStateStartTime = 0;
        int64_t keepTemporaryStateEndTime = 0;
    };
    std::mutex multiUserTempColorModeMapMutex_;
    std::map<int32_t, TempColorModeInfo> multiUserTempColorModeMap_;
};
} // namespace OHOS::ArkUi::UiAppearance

#endif // UI_APPEARANCE_DARK_MODE_TEMP_STATE_MANAGER_H
