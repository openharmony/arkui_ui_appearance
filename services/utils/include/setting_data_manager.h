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

#ifndef UI_APPEARANCE_UTILS_SETTING_DATA_MANAGER_H
#define UI_APPEARANCE_UTILS_SETTING_DATA_MANAGER_H

#include "datashare_helper.h"
#include "errors.h"
#include "nocopyable.h"
#include "setting_data_observer.h"

namespace OHOS::ArkUi::UiAppearance {
class SettingDataManager final : public NoCopyable {
public:
    static SettingDataManager &GetInstance();

    ErrCode Initialize();

    bool IsInitialized() const;

    ErrCode RegisterObserver(const std::string& key, const SettingDataObserver::UpdateFunc& updateFunc,
        int32_t userId = INVALID_USER_ID);

    ErrCode UnregisterObserver(const std::string& key, int32_t userId = INVALID_USER_ID);

    ErrCode GetStringValue(const std::string& key, std::string& value, int32_t userId = INVALID_USER_ID) const;

    ErrCode GetInt32Value(const std::string& key, int32_t& value, int32_t userId = INVALID_USER_ID) const;

    ErrCode GetInt32ValueStrictly(const std::string& key, int32_t& value, int32_t userId = INVALID_USER_ID) const;

    ErrCode GetBoolValue(const std::string& key, bool& value, int32_t userId = INVALID_USER_ID) const;

    ErrCode SetStringValue(const std::string& key, const std::string& value, int32_t userId = INVALID_USER_ID,
        bool needNotify = true) const;

    ErrCode SetInt32Value(const std::string& key, int32_t value, int32_t userId = INVALID_USER_ID,
        bool needNotify = true) const;

    ErrCode SetBoolValue(const std::string& key, bool value, int32_t userId = INVALID_USER_ID,
        bool needNotify = true) const;

    bool IsValidKey(const std::string& key, int32_t userId = INVALID_USER_ID) const;

private:
    std::mutex initializeMutex_;
    bool isInitialized_ = false;
    sptr<IRemoteObject> remoteObject_;

    std::mutex observersMutex_;
    std::map<std::string, sptr<SettingDataObserver>> observers_;

    ErrCode RegisterObserverInner(const sptr<SettingDataObserver>& observer) const;

    ErrCode UnregisterObserverInner(const sptr<SettingDataObserver>& observer) const;

    void CreateDataShareHelperAndUri(int32_t userId, const std::string& key,
        std::string& uri, std::shared_ptr<DataShare::DataShareHelper>& helper) const;

    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper() const;

    std::shared_ptr<DataShare::DataShareHelper> CreateUserDbDataShareHelper(int32_t userId) const;

    static bool ReleaseDataShareHelper(const std::shared_ptr<DataShare::DataShareHelper>& helper);

    static inline std::string GenerateObserverName(const std::string& key, int32_t userId);

    static sptr<SettingDataObserver> CreateObserver(const std::string& key,
        const SettingDataObserver::UpdateFunc& updateFunc, int32_t userId = INVALID_USER_ID);

    static std::string AssembleUri(const std::string& key);

    static std::string AssembleUserDbUri(int32_t userId, const std::string& key);
};
} // namespace OHOS::ArkUi::UiAppearance

#endif // UI_APPEARANCE_UTILS_SETTING_DATA_MANAGER_H
