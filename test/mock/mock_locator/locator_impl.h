/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef UI_APPEARANCE_MOCK_LOCATOR_LOCATOR_IMPL_H
#define UI_APPEARANCE_MOCK_LOCATOR_LOCATOR_IMPL_H

#include <memory>
#include <gmock/gmock.h>

namespace OHOS {
namespace Location {

class Location {
public:
    virtual ~Location() = default;
    MOCK_METHOD(double, GetLatitude, (), (const));
    MOCK_METHOD(double, GetLongitude, (), (const));
    MOCK_METHOD(double, GetAccuracy, (), (const));
    MOCK_METHOD(int64_t, GetTimeStamp, (), (const));
};

class LocatorImpl {
public:
    static std::shared_ptr<LocatorImpl> GetInstance()
    {
        static auto instance = std::make_shared<LocatorImpl>();
        return instance;
    }

    virtual ~LocatorImpl() = default;

    std::unique_ptr<Location> GetCachedLocation()
    {
        auto loc = GetCachedLocationInner();
        if (loc == nullptr) {
            return nullptr;
        }
        return std::unique_ptr<Location>(loc.release());
    }

    MOCK_METHOD(bool, IsLocationEnabled, (), ());
    MOCK_METHOD(std::unique_ptr<Location>, GetCachedLocationInner, ());
};

} // namespace Location
} // namespace OHOS

#endif // UI_APPEARANCE_MOCK_LOCATOR_LOCATOR_IMPL_H
