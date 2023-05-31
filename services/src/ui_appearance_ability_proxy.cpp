/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ui_appearance_ability_proxy.h"

#include "message_parcel.h"
#include "ui_appearance_log.h"

namespace OHOS {
namespace ArkUi::UiAppearance {
int32_t UiAppearanceAbilityProxy::SetDarkMode(UiAppearanceAbilityInterface::DarkMode mode)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR("Write descriptor failed!");
        return SYS_ERR;
    }
    if (!data.WriteInt32(mode)) {
        HILOG_ERROR("Write mode failed!");
        return SYS_ERR;
    }

    auto res = Remote()->SendRequest(SET_DARK_MODE, data, reply, option);
    if (res != ERR_NONE) {
        HILOG_ERROR("SendRequest failed.");
        return SYS_ERR;
    }

    return reply.ReadInt32();
}

UiAppearanceAbilityInterface::DarkMode UiAppearanceAbilityProxy::GetDarkMode()
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR("Write descriptor failed!");
        return UNKNOWN;
    }

    auto res = Remote()->SendRequest(GET_DARK_MODE, data, reply, option);
    if (res != ERR_NONE) {
        HILOG_ERROR("SendRequest failed.");
        return UNKNOWN;
    }

    return static_cast<UiAppearanceAbilityInterface::DarkMode>(reply.ReadInt32());
}
} // namespace ArkUi::UiAppearance
} // namespace OHOS