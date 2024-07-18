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

#ifndef UI_APPEARANCE_ABILITY_STUB_H
#define UI_APPEARANCE_ABILITY_STUB_H

#include "iremote_stub.h"
#include "ui_appearance_ability_interface.h"
#include "message_parcel.h"

namespace OHOS {
namespace ArkUi::UiAppearance {
class UiAppearanceAbilityStub : public IRemoteStub<UiAppearanceAbilityInterface> {
public:
    UiAppearanceAbilityStub(bool serialInvokeFlag = true) : IRemoteStub(serialInvokeFlag) {};
    ~UiAppearanceAbilityStub() override = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

private:
    int32_t OnGetFontScaleInner(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t OnSetFontScaleInner(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t OnGetFontWeightScaleInner(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t OnSetFontWeightScaleInner(MessageParcel& data, MessageParcel& reply, MessageOption& option);
};
} // namespace ArkUi::UiAppearance
} // namespace OHOS
#endif // UI_APPEARANCE_ABILITY_STUB_H