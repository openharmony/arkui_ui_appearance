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

#ifndef UI_APPEARANCE_UTILS_IPC_SKELETON_UTILS_H
#define UI_APPEARANCE_UTILS_IPC_SKELETON_UTILS_H

#include "ipc_skeleton.h"

namespace OHOS::ArkUi::UiAppearance {
class ResetCallingIdentityScope {
public:
    ResetCallingIdentityScope()
    {
        callingIdentity_ = IPCSkeleton::ResetCallingIdentity();
    }

    ~ResetCallingIdentityScope()
    {
        IPCSkeleton::SetCallingIdentity(callingIdentity_);
    }

private:
    std::string callingIdentity_;
};
} // namespace OHOS::ArkUi::UiAppearance
#endif // UI_APPEARANCE_UTILS_IPC_SKELETON_UTILS_H
