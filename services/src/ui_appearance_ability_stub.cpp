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

#include "ui_appearance_ability_stub.h"

#include <string>
#include "ui_appearance_ipc_interface_code.h"
#include "ui_appearance_log.h"

namespace OHOS {
namespace ArkUi::UiAppearance {
int32_t UiAppearanceAbilityStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    bool ret = false;
    std::u16string interfaceToken = data.ReadInterfaceToken();
    if (interfaceToken != GetDescriptor()) {
        LOGE("error permission denied.");
        return -1;
    }
    switch (code) {
        case static_cast<uint32_t>(UiAppearanceInterfaceCode::SET_DARK_MODE): {
            UiAppearanceAbilityInterface::DarkMode mode =
                static_cast<UiAppearanceAbilityInterface::DarkMode>(data.ReadInt32());
            ret = reply.WriteInt32(SetDarkMode(mode));
            return (ret ? 0 : -1);
        }
        case static_cast<uint32_t>(UiAppearanceInterfaceCode::GET_DARK_MODE): {
            ret = reply.WriteInt32(GetDarkMode());
            return (ret ? 0 : -1);
        }
        case static_cast<uint32_t>(UiAppearanceInterfaceCode::GET_FONT_SCALE): {
            return OnGetFontScaleInner(data, reply, option);
        }
        case static_cast<uint32_t>(UiAppearanceInterfaceCode::SET_FONT_SCALE): {
            return OnSetFontScaleInner(data, reply, option);
        }
        case static_cast<uint32_t>(UiAppearanceInterfaceCode::GET_FONT_Weight_SCALE): {
            return OnGetFontWeightScaleInner(data, reply, option);
        }
        case static_cast<uint32_t>(UiAppearanceInterfaceCode::SET_FONT_Weight_SCALE): {
            return OnSetFontWeightScaleInner(data, reply, option);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t UiAppearanceAbilityStub::OnGetFontScaleInner(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::string fontScale;
    auto state = GetFontScale(fontScale);
    auto ret = reply.WriteInt32(state);
    if (!ret) {
        LOGE("ui_appearance GetFontScale write state fail");
        return -1;
    }
    ret = reply.WriteString(fontScale);
    if (!ret) {
        LOGE("ui_appearance GetFontScale write fontScale fail");
        return -1;
    }
    return 0;
}

int32_t UiAppearanceAbilityStub::OnSetFontScaleInner(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::string fontScale;
    auto ret = data.ReadString(fontScale);
    if (!ret) {
        LOGE("ui_appearance SetFontScale Read fontScale fail");
        return -1;
    }
    ret = reply.WriteInt32(SetFontScale(fontScale));
    return (ret ? 0 : -1);
}

int32_t UiAppearanceAbilityStub::OnGetFontWeightScaleInner(
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::string fontWeightScale;
    auto state = GetFontWeightScale(fontWeightScale);
    auto ret = reply.WriteInt32(state);
    if (!ret) {
        LOGE("ui_appearance GetFontWeightScale write state fail");
        return -1;
    }
    ret = reply.WriteString(fontWeightScale);
    if (!ret) {
        LOGE("ui_appearance GetFontWeightScale write fontScale fail");
        return -1;
    }
    return 0;
}

int32_t UiAppearanceAbilityStub::OnSetFontWeightScaleInner(
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::string fontWeightScale;
    auto ret = data.ReadString(fontWeightScale);
    if (!ret) {
        LOGE("ui_appearance SetFontWeightScale Read fontScale fail");
        return -1;
    }
    ret = reply.WriteInt32(SetFontWeightScale(fontWeightScale));
    return (ret ? 0 : -1);
}

} // namespace ArkUi::UiAppearance
} // namespace OHOS
