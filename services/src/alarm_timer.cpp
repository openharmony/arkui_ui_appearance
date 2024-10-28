/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "alarm_timer.h"
#include "ui_appearance_log.h"

namespace OHOS {
namespace ArkUi::UiAppearance {

void AlarmTimer::OnTrigger()
{
    LOGI("UiAppearanceAlarmTimer::OnTrigger called");
    if (callBack_) {
        LOGI("callBack_ called");
        callBack_();
    }
}

void AlarmTimer::SetCallbackInfo(std::function<void()> callBack)
{
    callBack_ = callBack;
}

void AlarmTimer::SetType(const int &type)
{
    this->type = type;
}

void AlarmTimer::SetRepeat(bool repeat)
{
    this->repeat = repeat;
}

void AlarmTimer::SetInterval(const uint64_t &interval)
{
    this->interval = interval;
}

void AlarmTimer::SetWantAgent(std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent)
{
    this->wantAgent = wantAgent;
}

} // namespace ArkUi::UiAppearance
} // namespace OHOS