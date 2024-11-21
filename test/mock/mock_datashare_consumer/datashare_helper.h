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

#ifndef UI_APPEARANCE_MOCK_DATASHARE_HELPER_H
#define UI_APPEARANCE_MOCK_DATASHARE_HELPER_H

#include <vector>
#include <gmock/gmock.h>

#include "datashare_errno.h"
#include "datashare_result_set.h"
#include "datashare_predicates.h"
#include "datashare_values_bucket.h"
#include "data_ability_observer_stub.h"
#include "iremote_object.h"
#include "refbase.h"
#include "uri.h"

namespace OHOS::DataShare {
class DataShareHelper : public std::enable_shared_from_this<DataShareHelper> {
public:
    static DataShareHelper &GetInstance()
    {
        static DataShareHelper instance_;
        return instance_;
    }

    DataShareHelper() = default;

    ~DataShareHelper() = default;

    static std::pair<int, std::shared_ptr<DataShareHelper>> Create(const sptr<IRemoteObject>& token,
        const std::string& strUri, const std::string& extUri, const int waitTime = 2)
    {
        return GetInstance().MockCreate(token, strUri, extUri, waitTime);
    }

    MOCK_METHOD((std::pair<int, std::shared_ptr<DataShareHelper>>), MockCreate,
        (const sptr<IRemoteObject>&, const std::string&, const std::string&, const int));
    MOCK_METHOD(std::shared_ptr<DataShareResultSet>, Query,
        (Uri&, const DataSharePredicates&, std::vector<std::string>&));
    MOCK_METHOD(int, Insert, (Uri&, const DataShareValuesBucket&));
    MOCK_METHOD(int, Update, (Uri&, const DataSharePredicates&, const DataShareValuesBucket &));
    MOCK_METHOD(bool, Release, ());
    MOCK_METHOD(void, NotifyChange, (const Uri&));
    MOCK_METHOD(void, RegisterObserver, (const Uri&, const sptr<AAFwk::IDataAbilityObserver>&));
    MOCK_METHOD(void, UnregisterObserver, (const Uri&, const sptr<AAFwk::IDataAbilityObserver>&));
};
} // namespace OHOS::DataShare
#endif // UI_APPEARANCE_MOCK_DATASHARE_HELPER_H
