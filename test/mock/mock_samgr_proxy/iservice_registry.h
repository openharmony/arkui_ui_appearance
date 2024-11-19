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

#ifndef UI_APPEARANCE_MOCK_ISERVICE_REGISTRY_H
#define UI_APPEARANCE_MOCK_ISERVICE_REGISTRY_H

#include <gmock/gmock.h>

#include "if_system_ability_manager.h"
#include "refbase.h"

namespace OHOS {
class SystemAbilityManagerClient {
public:
    static SystemAbilityManagerClient& GetInstance()
    {
        static SystemAbilityManagerClient instance;
        return instance;
    }

    MOCK_METHOD(sptr<ISystemAbilityManager>, GetSystemAbilityManager, ());

private:
    SystemAbilityManagerClient() = default;

    ~SystemAbilityManagerClient() = default;
};
} // namespace OHOS
#endif // UI_APPEARANCE_MOCK_ISERVICE_REGISTRY_H
