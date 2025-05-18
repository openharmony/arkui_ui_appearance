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

#include "json_utils.h"
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <regex>

#include "ui_appearance_log.h"
#include "config_policy_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS::ArkUi::UiAppearance {
bool JsonUtils::LoadConfiguration(const std::string& path, nlohmann::json& jsonBuf,
    const std::string& defaultPath)
{
    std::string configPath = GetConfigPath(path, defaultPath);
    LOGD("config path is: %{public}s", configPath.c_str());
    if (!ReadFileInfoJson(configPath, jsonBuf)) {
        return false;
    }
    return true;
}

std::string JsonUtils::GetConfigPath(const std::string& path, const std::string& defaultPath)
{
    char buf[MAX_PATH_LEN] = { 0 };
    char *configPath = GetOneCfgFile(path.c_str(), buf, MAX_PATH_LEN);
    if (configPath == nullptr || configPath[0] == '\0' || strlen(configPath) > MAX_PATH_LEN) {
        return defaultPath;
    }
    return configPath;
}

bool JsonUtils::ReadFileInfoJson(const std::string &filePath, nlohmann::json &jsonBuf)
{
    if (filePath.empty()) {
        LOGE("filePath empty");
        return false;
    }
    
    if (access(filePath.c_str(), F_OK) != 0) {
        LOGE("deepLink config not exist");
        return false;
    }

    char path[PATH_MAX] = {0};
    if (realpath(filePath.c_str(), path) == nullptr) {
        LOGE("realpath error, errno: %{public}d", errno);
        return false;
    }

    std::fstream in;
    char errBuf[256];
    errBuf[0] = '\0';
    in.open(path, std::ios_base::in);
    if (!in.is_open()) {
        strerror_r(errno, errBuf, sizeof(errBuf));
        LOGE("file not open: %{public}s", errBuf);
        return false;
    }

    in.seekg(0, std::ios::end);
    int64_t size = in.tellg();
    if (size <= 0) {
        LOGE("empty file");
        in.close();
        return false;
    }

    in.seekg(0, std::ios::beg);
    jsonBuf = nlohmann::json::parse(in, nullptr, false);
    in.close();
    if (jsonBuf.is_discarded()) {
        LOGE("bad profile file");
        return false;
    }

    return true;
}

bool JsonUtils::IsEqual(nlohmann::json &jsonObject, const std::string &key, const std::string &value, bool checkEmpty)
{
    if (jsonObject.contains(key) && jsonObject[key].is_string()) {
        std::string  jsonValue = jsonObject.at(key).get<std::string>();
        if (checkEmpty && !jsonValue.empty() && jsonValue != value) {
            return false;
        } else if (value != jsonValue) {
            return false;
        }
    }
    return true;
}

bool JsonUtils::IsEqual(nlohmann::json &jsonObject, const std::string &key, int32_t value)
{
    if (jsonObject.contains(key) && jsonObject[key].is_number()) {
        if (value != jsonObject.at(key).get<int32_t>()) {
            return false;
        }
    }
    return true;
}

std::optional<bool> JsonUtils::JsonToOptionalBool(const nlohmann::json &jsonObject, const std::string &key)
{
    if (jsonObject.contains(key) && jsonObject[key].is_boolean()) {
        return jsonObject[key].get<bool>();
    }
    return std::nullopt;
}
} // namespace OHOS::ArkUi::UiAppearance
