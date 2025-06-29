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
#include <unistd.h>
#include <sys/stat.h>
#include "config_policy_utils.h"
#include "ui_appearance_log.h"

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

    std::string dataBuffer;
    bool res = ReadFileToBuffer(path, dataBuffer);
    if (!res) {
        LOGE("ReadFileToBuffer failed");
        return false;
    }
    jsonBuf = nlohmann::json::parse(dataBuffer, nullptr, false);
    if (jsonBuf.is_discarded()) {
        LOGE("bad profile file");
        return false;
    }

    return true;
}

bool JsonUtils::ReadFileToBuffer(const std::string &filePath, std::string &dataBuffer)
{
    struct stat statbuf;
    int ret = stat(filePath.c_str(), &statbuf);
    if (ret != 0) {
        LOGE("fail, ret:%{public}d", ret);
        return false;
    }
    size_t bufferSize = static_cast<size_t>(statbuf.st_size);

    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(bufferSize);
    if (buffer == nullptr) {
        LOGE("buffer null");
        return false;
    }

    FILE *fp = fopen(filePath.c_str(), "rb");
    if (fp == nullptr) {
        LOGE("fail, real path=%{public}s", filePath.c_str());
        return false;
    }

    ret = fseek(fp, 0, SEEK_END);
    if (ret != 0) {
        LOGE("fseek fail, ret=%{public}d", ret);
        fclose(fp);
        return false;
    }

    size_t fileSize = static_cast<size_t>(ftell(fp));

    ret = fseek(fp, 0, SEEK_SET);
    if (ret != 0) {
        LOGE("fseek fail, ret=%{public}d", ret);
        fclose(fp);
        return false;
    }
    
    if (bufferSize < fileSize) {
        LOGE("buffer size:(%{public}zu) is smaller than file size:(%{public}zu)", bufferSize,
            fileSize);
        fclose(fp);
        return false;
    }
    size_t retSize = std::fread(buffer.get(), 1, fileSize, fp);
    if (retSize != fileSize) {
        LOGE("read file result size = %{public}zu, size = %{public}zu",
            retSize, fileSize);
        fclose(fp);
        return false;
    }
    (void)fclose(fp);
    dataBuffer = std::string(reinterpret_cast<char *>(buffer.get()), bufferSize);
    return true;
}
} // namespace OHOS::ArkUi::UiAppearance
