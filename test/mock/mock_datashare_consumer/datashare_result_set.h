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

#ifndef UI_APPEARANCE_MOCK_DATASHARE_RESULT_SET_H
#define UI_APPEARANCE_MOCK_DATASHARE_RESULT_SET_H

#include <gmock/gmock.h>

namespace OHOS::DataShare {
class DataShareResultSet {
public:
    DataShareResultSet() = default;

    ~DataShareResultSet() = default;

    MOCK_METHOD(int, GetRowCount, (int&));
    MOCK_METHOD(int, GoToRow, (int));
    MOCK_METHOD(int, GetString, (int, std::string&));
    MOCK_METHOD(int, Close, ());
};
} // namespace OHOS::DataShare
#endif // UI_APPEARANCE_MOCK_DATASHARE_RESULT_SET_H
