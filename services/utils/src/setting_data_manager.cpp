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

#include "setting_data_manager.h"

#include <charconv>

#include "ipc_skeleton_utils.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ui_appearance_log.h"

namespace OHOS::ArkUi::UiAppearance {
namespace {
constexpr const char* SETTING_DATA_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
constexpr const char* SETTING_DATA_USER_URI_PREFIX =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_";
constexpr const char* SETTING_DATA_USER_URI_SUFFIX = "?Proxy=true";
constexpr const char* SETTING_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
constexpr const char* SETTING_DATA_KEY_URI = "&key=";
constexpr const char* SETTING_DATA_COLUMN_KEYWORD = "KEYWORD";
constexpr const char* SETTING_DATA_COLUMN_VALUE = "VALUE";
constexpr int32_t INDEX0 = 0;
}

SettingDataManager &SettingDataManager::GetInstance()
{
    static SettingDataManager instance;
    return instance;
}

ErrCode SettingDataManager::Initialize()
{
    std::lock_guard guard(initializeMutex_);
    if (isInitialized_) {
        LOGD("already initialized");
        return ERR_OK;
    }
    const auto saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgr == nullptr) {
        LOGE("saMgr is null");
        return ERR_NO_INIT;
    }
    const auto remoteObj = saMgr->GetSystemAbility(ARKUI_UI_APPEARANCE_SERVICE_ID);
    if (remoteObj == nullptr) {
        LOGE("remoteObj is null");
        return ERR_NO_INIT;
    }
    remoteObject_ = remoteObj;
    isInitialized_ = true;
    return ERR_OK;
}

bool SettingDataManager::IsInitialized() const
{
    return isInitialized_;
}

ErrCode SettingDataManager::RegisterObserver(const std::string& key, const SettingDataObserver::UpdateFunc& updateFunc,
    const int32_t userId)
{
    if (!isInitialized_) {
        LOGE("SettingDataManager not initialized");
        return ERR_NO_INIT;
    }

    std::lock_guard guard(observersMutex_);
    const std::string observerName = GenerateObserverName(key, userId);
    const auto& iter = observers_.find(observerName);
    if (iter != observers_.end()) {
        LOGD("observerName: %{public}s is exist", observerName.c_str());
        return ERR_OK;
    }

    sptr<SettingDataObserver> observer = CreateObserver(key, updateFunc, userId);
    ErrCode code = RegisterObserverInner(observer);
    if (code != ERR_OK) {
        return code;
    }
    observers_.emplace(observerName, observer);
    return ERR_OK;
}

ErrCode SettingDataManager::UnregisterObserver(const std::string& key, const int32_t userId)
{
    const std::string observerName = GenerateObserverName(key, userId);
    std::lock_guard guard(observersMutex_);
    const auto& iter = observers_.find(observerName);
    if (iter == observers_.end()) {
        LOGE("observerName: %{public}s is not found", observerName.c_str());
        return ERR_INVALID_VALUE;
    }
    if (iter->second == nullptr) {
        LOGE("observerName: %{public}s observer is null", observerName.c_str());
        observers_.erase(iter);
        return ERR_INVALID_OPERATION;
    }

    ErrCode code = UnregisterObserverInner(iter->second);
    observers_.erase(iter);
    return code;
}

ErrCode SettingDataManager::GetStringValue(const std::string& key, std::string& value, const int32_t userId) const
{
    std::string uriString;
    ResetCallingIdentityScope scope;
    std::shared_ptr<DataShare::DataShareHelper> helper;
    CreateDataShareHelperAndUri(userId, key, uriString, helper);
    if (helper == nullptr) {
        LOGE("helper is null, userId: %{public}d", userId);
        return ERR_NO_INIT;
    }

    std::vector<std::string> columns = { SETTING_DATA_COLUMN_VALUE };
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_DATA_COLUMN_KEYWORD, key);
    Uri uri(uriString);
    auto result = helper->Query(uri, predicates, columns);
    ReleaseDataShareHelper(helper);
    if (result == nullptr) {
        LOGE("query return null, key: %{public}s, userId: %{public}d", key.c_str(), userId);
        return ERR_INVALID_OPERATION;
    }
    int32_t count = 0;
    result->GetRowCount(count);
    if (count == 0) {
        LOGI("not found, key: %{public}s, userId: %{public}d", key.c_str(), userId);
        result->Close();
        return ERR_NAME_NOT_FOUND;
    }

    result->GoToRow(INDEX0);
    int32_t code = result->GetString(INDEX0, value);
    if (code != 0) {
        LOGE("get string failed, key: %{public}s, userId: %{public}d, ret: %{public}d", key.c_str(), userId, code);
        result->Close();
        return ERR_INVALID_VALUE;
    }
    result->Close();
    LOGD("Get key: %{public}s, userId: %{public}d, value: %{public}s", key.c_str(), userId, value.c_str());
    return ERR_OK;
}

ErrCode SettingDataManager::GetInt32Value(const std::string& key, int32_t& value, const int32_t userId) const
{
    std::string valueString;
    ErrCode code = GetStringValue(key, valueString, userId);
    if (code != ERR_OK) {
        return code;
    }

    auto res = std::from_chars(valueString.c_str(), valueString.c_str() + valueString.size(), value);
    if (res.ec != std::errc()) {
        LOGE("key: %{public}s, userId: %{public}d, value: %{public}s failed to convert to int",
            key.c_str(), userId, valueString.c_str());
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

ErrCode SettingDataManager::GetInt32ValueStrictly(const std::string& key, int32_t& value, const int32_t userId) const
{
    std::string valueString;
    ErrCode code = GetStringValue(key, valueString, userId);
    if (code != ERR_OK) {
        return code;
    }
    int32_t convertedValue;
    auto res = std::from_chars(valueString.c_str(), valueString.c_str() + valueString.size(), convertedValue);
    if (res.ec != std::errc()) {
        LOGE("key: %{public}s, userId: %{public}d, value: %{public}s failed to convert to int",
            key.c_str(), userId, valueString.c_str());
        return ERR_INVALID_VALUE;
    }
    if (std::to_string(convertedValue) != valueString) {
        LOGE("key: %{public}s, userId: %{public}d, value: %{public}s is not strict int",
            key.c_str(), userId, valueString.c_str());
        return ERR_INVALID_VALUE;
    }
    value = convertedValue;
    return ERR_OK;
}

ErrCode SettingDataManager::GetBoolValue(const std::string& key, bool& value, const int32_t userId) const
{
    std::string valueString;
    ErrCode code = GetStringValue(key, valueString, userId);
    if (code != ERR_OK) {
        return code;
    }
    if (valueString == "true") {
        value = true;
        return ERR_OK;
    } else if (valueString == "false") {
        value = false;
        return ERR_OK;
    } else {
        LOGE("key: %{public}s, userId: %{public}d, value: %{public}s failed to convert to bool",
            key.c_str(), userId, valueString.c_str());
        return ERR_INVALID_VALUE;
    }
}

ErrCode SettingDataManager::SetStringValue(const std::string& key, const std::string& value, int32_t userId,
    bool needNotify) const
{
    std::string uriString;
    ResetCallingIdentityScope scope;
    std::shared_ptr<DataShare::DataShareHelper> helper;
    CreateDataShareHelperAndUri(userId, key, uriString, helper);
    if (helper == nullptr) {
        LOGE("helper is null, userId: %{public}d", userId);
        return ERR_NO_INIT;
    }

    DataShare::DataShareValueObject keyObj(key);
    DataShare::DataShareValueObject valueObj(value);
    DataShare::DataShareValuesBucket bucket;
    bucket.Put(SETTING_DATA_COLUMN_KEYWORD, keyObj);
    bucket.Put(SETTING_DATA_COLUMN_VALUE, valueObj);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_DATA_COLUMN_KEYWORD, key);
    Uri uri(uriString);
    int32_t result = helper->Update(uri, predicates, bucket);
    if (result <= 0) {
        LOGD("data not exist, key: %{public}s, userId: %{public}d", key.c_str(), userId);
        result = helper->Insert(uri, bucket);
        if (result <= 0) {
            ReleaseDataShareHelper(helper);
            LOGE("put string failed, key: %{public}s, userId: %{public}d, result: %{public}d",
                key.c_str(), userId, result);
            return ERR_INVALID_OPERATION;
        }
    }
    if (needNotify) {
        helper->NotifyChange(uri);
    }
    ReleaseDataShareHelper(helper);
    LOGD("put key: %{public}s, userId: %{public}d, value: %{public}s", key.c_str(), userId, value.c_str());
    return ERR_OK;
}

ErrCode SettingDataManager::SetInt32Value(const std::string& key, const int32_t value, const int32_t userId,
    const bool needNotify) const
{
    const std::string valueString = std::to_string(value);
    return SetStringValue(key, valueString, userId, needNotify);
}

ErrCode SettingDataManager::SetBoolValue(const std::string& key, const bool value, const int32_t userId,
    const bool needNotify) const
{
    const std::string valueString = value ? "true" : "false";
    return SetStringValue(key, valueString, userId, needNotify);
}

bool SettingDataManager::IsValidKey(const std::string& key, const int32_t userId) const
{
    std::string value;
    return (ERR_OK == GetStringValue(key, value, userId)) && (!value.empty());
}

ErrCode SettingDataManager::RegisterObserverInner(const sptr<SettingDataObserver>& observer) const
{
    if (observer == nullptr) {
        LOGE("observer is null");
        return ERR_INVALID_OPERATION;
    }

    ResetCallingIdentityScope scope;
    std::string uriString;
    std::shared_ptr<DataShare::DataShareHelper> helper;
    CreateDataShareHelperAndUri(observer->GetUserId(), observer->GetKey(), uriString, helper);
    if (helper == nullptr) {
        LOGE("helper is null, userId: %{public}d", observer->GetUserId());
        return ERR_NO_INIT;
    }

    const Uri uri(uriString);
    helper->RegisterObserver(uri, observer);
    helper->NotifyChange(uri);
    ReleaseDataShareHelper(helper);
    LOGD("register observer key: %{public}s, userId: %{public}d", observer->GetKey().c_str(), observer->GetUserId());
    return ERR_OK;
}

ErrCode SettingDataManager::UnregisterObserverInner(const sptr<SettingDataObserver>& observer) const
{
    if (observer == nullptr) {
        LOGE("observer is null");
        return ERR_INVALID_OPERATION;
    }

    ResetCallingIdentityScope scope;
    std::string uriString;
    std::shared_ptr<DataShare::DataShareHelper> helper;
    CreateDataShareHelperAndUri(observer->GetUserId(), observer->GetKey(), uriString, helper);
    if (helper == nullptr) {
        LOGE("helper is null");
        return ERR_NO_INIT;
    }

    const Uri uri(uriString);
    helper->UnregisterObserver(uri, observer);
    ReleaseDataShareHelper(helper);
    LOGD("unregister observer key: %{public}s, userId: %{public}d", observer->GetKey().c_str(), observer->GetUserId());
    return ERR_OK;
}

void SettingDataManager::CreateDataShareHelperAndUri(const int32_t userId, const std::string& key,
    std::string& uri, std::shared_ptr<DataShare::DataShareHelper>& helper) const
{
    if (userId == INVALID_USER_ID) {
        uri = AssembleUri(key);
        helper = CreateDataShareHelper();
    } else {
        uri = AssembleUserDbUri(userId, key);
        helper = CreateUserDbDataShareHelper(userId);
    }
}

std::shared_ptr<DataShare::DataShareHelper> SettingDataManager::CreateDataShareHelper() const
{
    if (remoteObject_ == nullptr) {
        LOGE("remoteObject_ is null");
        return nullptr;
    }
    const auto [errCode, helper] =
        DataShare::DataShareHelper::Create(remoteObject_, SETTING_DATA_URI, SETTING_DATA_EXT_URI);
    if (errCode != DataShare::E_OK) {
        LOGE("create data share helper failed: %{public}d", errCode);
        return nullptr;
    }
    return helper;
}

std::shared_ptr<DataShare::DataShareHelper> SettingDataManager::CreateUserDbDataShareHelper(const int32_t userId) const
{
    if (remoteObject_ == nullptr) {
        LOGE("remoteObject_ is null");
        return nullptr;
    }
    std::string userUri = SETTING_DATA_USER_URI_PREFIX + std::to_string(userId) + SETTING_DATA_USER_URI_SUFFIX;
    const auto [errCode, helper] =
        DataShare::DataShareHelper::Create(remoteObject_, userUri, SETTING_DATA_EXT_URI);
    if (errCode != DataShare::E_OK) {
        LOGE("create user data share helper failed: %{public}d, userId: %{public}d", errCode, userId);
        return nullptr;
    }
    return helper;
}

bool SettingDataManager::ReleaseDataShareHelper(const std::shared_ptr<DataShare::DataShareHelper>& helper)
{
    if (helper == nullptr) {
        LOGE("helper is null");
        return false;
    }
    if (!helper->Release()) {
        LOGE("release data share helper failed");
        return false;
    }
    return true;
}

std::string SettingDataManager::GenerateObserverName(const std::string& key, const int32_t userId)
{
    return key + "::" + std::to_string(userId);
}

sptr<SettingDataObserver> SettingDataManager::CreateObserver(const std::string& key,
    const SettingDataObserver::UpdateFunc& updateFunc, const int32_t userId)
{
    sptr<SettingDataObserver> observer = sptr<SettingDataObserver>::MakeSptr();
    if (observer == nullptr) {
        LOGE("observer is null");
        return observer;
    }
    observer->SetKey(key);
    observer->SetUpdateFunc(updateFunc);
    observer->SetUserId(userId);
    return observer;
}

std::string SettingDataManager::AssembleUri(const std::string& key)
{
    std::string uriString = SETTING_DATA_URI;
    uriString.append(SETTING_DATA_KEY_URI + key);
    return uriString;
}

std::string SettingDataManager::AssembleUserDbUri(const int32_t userId, const std::string& key)
{
    std::string uriString = SETTING_DATA_USER_URI_PREFIX;
    uriString.append(std::to_string(userId) + SETTING_DATA_USER_URI_SUFFIX);
    uriString.append(SETTING_DATA_KEY_URI + key);
    return uriString;
}
} // namespace OHOS::ArkUi::UiAppearance
