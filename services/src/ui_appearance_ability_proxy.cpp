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
#include "ui_appearance_ipc_interface_code.h"
#include "ui_appearance_log.h"

namespace OHOS {
namespace ArkUi::UiAppearance {
int32_t UiAppearanceAbilityProxy::SetDarkMode(UiAppearanceAbilityInterface::DarkMode mode)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Write descriptor failed!");
        return SYS_ERR;
    }
    if (!data.WriteInt32(mode)) {
        LOGE("Write mode failed!");
        return SYS_ERR;
    }

    auto res =
        Remote()->SendRequest(static_cast<uint32_t>(UiAppearanceInterfaceCode::SET_DARK_MODE), data, reply, option);
    if (res != ERR_NONE) {
        LOGE("SendRequest failed.");
        return SYS_ERR;
    }

    return reply.ReadInt32();
}

int32_t UiAppearanceAbilityProxy::GetDarkMode()
{
    MessageParcel data, reply;
    MessageOption option;

    if (UNLIKELY(!data.WriteInterfaceToken(GetDescriptor()))) {
        LOGE("Write descriptor failed!");
        return SYS_ERR;
    }

    auto res =
        Remote()->SendRequest(static_cast<uint32_t>(UiAppearanceInterfaceCode::GET_DARK_MODE), data, reply, option);
    if (UNLIKELY(res != ERR_NONE)) {
        LOGE("SendRequest failed.");
        return SYS_ERR;
    }

    return reply.ReadInt32();
}
int32_t UiAppearanceAbilityProxy::SetFontScale(std::string &fontScale)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Write descriptor failed!");
        return SYS_ERR;
    }
    if (!data.WriteString(fontScale)) {
        LOGE("Write fontScale failed!");
        return SYS_ERR;
    }

    auto res =
        Remote()->SendRequest(static_cast<uint32_t>(UiAppearanceInterfaceCode::SET_FONT_SCALE), data, reply, option);
    if (res != ERR_NONE) {
        LOGE("SendRequest failed.");
        return SYS_ERR;
    }

    return reply.ReadInt32();
}

int32_t UiAppearanceAbilityProxy::GetFontScale(std::string &fontScale)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Write descriptor failed!");
        return SYS_ERR;
    }

    auto res =
        Remote()->SendRequest(static_cast<uint32_t>(UiAppearanceInterfaceCode::GET_FONT_SCALE), data, reply, option);
    if (res != ERR_NONE) {
        LOGE("SendRequest failed.");
        return SYS_ERR;
    }

    res = reply.ReadInt32();

    if (!reply.ReadString(fontScale)) {
        LOGE("Read FontScale failed!");
        return SYS_ERR;
    }

    return res;
}

int32_t UiAppearanceAbilityProxy::SetFontWeightScale(std::string& fontWeightScale)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Write descriptor failed!");
        return SYS_ERR;
    }
    if (!data.WriteString(fontWeightScale)) {
        LOGE("Write fontScale failed!");
        return SYS_ERR;
    }

    auto res =
        Remote()->SendRequest(static_cast<uint32_t>(UiAppearanceInterfaceCode::SET_FONT_Weight_SCALE),
            data, reply, option);
    if (res != ERR_NONE) {
        LOGE("SendRequest failed.");
        return SYS_ERR;
    }

    return reply.ReadInt32();
}

int32_t UiAppearanceAbilityProxy::GetFontWeightScale(std::string &fontWeightScale)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("##Write descriptor failed!");
        return SYS_ERR;
    }

    auto res =
        Remote()->SendRequest(static_cast<uint32_t>(UiAppearanceInterfaceCode::GET_FONT_Weight_SCALE),
            data, reply, option);
    if (res != ERR_NONE) {
        LOGE("SendRequest failed.");
        return SYS_ERR;
    }

    res = reply.ReadInt32();

    if (!reply.ReadString(fontWeightScale)) {
        LOGE("Read FontScale failed!");
        return SYS_ERR;
    }

    return res;
}
} // namespace ArkUi::UiAppearance
} // namespace OHOS
