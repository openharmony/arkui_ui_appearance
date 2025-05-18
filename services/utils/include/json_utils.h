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

#ifndef UI_APPEARANCE_JSON_UTILS_H
#define UI_APPEARANCE_JSON_UTILS_H

#include <string>
#include <map>
#include <optional>

#include "nlohmann/json.hpp"
#include "singleton.h"

namespace OHOS::ArkUi::UiAppearance {
/**
 * @class JsonUtils
 * provides json utilities.
 */
class JsonUtils {
public:
    /**
     * GetInstance, get an instance of JsonUtils.
     *
     * @return an instance of JsonUtils.
     */
    static JsonUtils &GetInstance()
    {
        static JsonUtils instance;
        return instance;
    }

    /**
     * JsonUtils, destructor.
     *
     */
    ~JsonUtils() = default;

    /**
     * LoadConfiguration, load configuration.
     *
     * @param path The json file path.
     * @param jsonBuf The json buffer to be returned.
     * @param defaultPath The default output path.
     * @return Whether or not the load operation succeeds.
     */
    bool LoadConfiguration(const std::string& path, nlohmann::json& jsonBuf, const std::string& defaultPath = "");

    /**
     * IsEqual, check if json object contains certain key.
     *
     * @param jsonObject The json object.
     * @param key The key.
     * @param value The string value.
     * @param checkEmpty The flag indicates whether the value can be empty.
     * @return Whether or not the json object contains certain key.
     */
    bool IsEqual(nlohmann::json &jsonObject, const std::string &key,
        const std::string &value, bool checkEmpty = false);
    
    /**
     * IsEqual, check if json object contains certain key.
     *
     * @param jsonObject The json object.
     * @param key The key.
     * @param value The int32_t value.
     * @return Whether or not the json object contains certain key.
     */
    bool IsEqual(nlohmann::json &jsonObject, const std::string &key, int32_t value);

    /**
     * parse json to optional bool.
     *
     * @param jsonObject The json object.
     * @param key The key.
     * @return optional boolean value.
     */
    std::optional<bool> JsonToOptionalBool(const nlohmann::json &jsonObject, const std::string &key);

private:
    std::string GetConfigPath(const std::string& path, const std::string& defaultPath);
    bool ReadFileInfoJson(const std::string &filePath, nlohmann::json &jsonBuf);
    JsonUtils() = default;
    DISALLOW_COPY_AND_MOVE(JsonUtils);
};
} // namespace OHOS::ArkUi::UiAppearance
#endif // UI_APPEARANCE_JSON_UTILS_H