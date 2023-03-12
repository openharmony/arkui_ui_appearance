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

#ifndef UI_APPEARANCE_LOG_H
#define UI_APPEARANCE_LOG_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG
#include "hilog/log.h"

#ifdef HILOG_FATAL
#undef HILOG_FATAL
#endif

#ifdef HILOG_ERROR
#undef HILOG_ERROR
#endif

#ifdef HILOG_WARN
#undef HILOG_WARN
#endif

#ifdef HILOG_INFO
#undef HILOG_INFO
#endif

#ifdef HILOG_DEBUG
#undef HILOG_DEBUG
#endif

#ifdef LOG_LABEL
#undef LOG_LABEL
#endif

static constexpr unsigned int FRAMEWORK_DOMAIN = 0xD003900;
static constexpr unsigned int JS_DOMAIN = 0xD003B00;
static constexpr OHOS::HiviewDFX::HiLogLabel LOG_LABEL_FW = { LOG_CORE, FRAMEWORK_DOMAIN, "UiAppearance" };
static constexpr OHOS::HiviewDFX::HiLogLabel LOG_LABEL_JS = { LOG_CORE, JS_DOMAIN, "JSApp" };

#define UIFILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define HILOG_FATAL(fmt, ...)            \
    (void)OHOS::HiviewDFX::HiLog::Fatal( \
        LOG_LABEL_FW, "[%{public}s(%{public}s:%{public}d)]" fmt, UIFILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define HILOG_ERROR(fmt, ...)            \
    (void)OHOS::HiviewDFX::HiLog::Error( \
        LOG_LABEL_FW, "[%{public}s(%{public}s:%{public}d)]" fmt, UIFILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define HILOG_WARN(fmt, ...)            \
    (void)OHOS::HiviewDFX::HiLog::Warn( \
        LOG_LABEL_FW, "[%{public}s(%{public}s:%{public}d)]" fmt, UIFILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define HILOG_INFO(fmt, ...)            \
    (void)OHOS::HiviewDFX::HiLog::Info( \
        LOG_LABEL_FW, "[%{public}s(%{public}s:%{public}d)]" fmt, UIFILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define HILOG_DEBUG(fmt, ...)            \
    (void)OHOS::HiviewDFX::HiLog::Debug( \
        LOG_LABEL_FW, "[%{public}s(%{public}s:%{public}d)]" fmt, UIFILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define JS_HILOG_FATAL(fmt, ...)         \
    (void)OHOS::HiviewDFX::HiLog::Fatal( \
        LOG_LABEL_JS, "[%{public}s(%{public}s:%{public}d)]" fmt, UIFILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define JS_HILOG_ERROR(fmt, ...)         \
    (void)OHOS::HiviewDFX::HiLog::Error( \
        LOG_LABEL_JS, "[%{public}s(%{public}s:%{public}d)]" fmt, UIFILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define JS_HILOG_WARN(fmt, ...)         \
    (void)OHOS::HiviewDFX::HiLog::Warn( \
        LOG_LABEL_JS, "[%{public}s(%{public}s:%{public}d)]" fmt, UIFILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define JS_HILOG_INFO(fmt, ...)         \
    (void)OHOS::HiviewDFX::HiLog::Info( \
        LOG_LABEL_JS, "[%{public}s(%{public}s:%{public}d)]" fmt, UIFILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define JS_HILOG_DEBUG(fmt, ...)         \
    (void)OHOS::HiviewDFX::HiLog::Debug( \
        LOG_LABEL_JS, "[%{public}s(%{public}s:%{public}d)]" fmt, UIFILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else

#define HILOG_FATAL(...)
#define HILOG_ERROR(...)
#define HILOG_WARN(...)
#define HILOG_INFO(...)
#define HILOG_DEBUG(...)

#define JS_HILOG_FATAL(...)
#define JS_HILOG_ERROR(...)
#define JS_HILOG_WARN(...)
#define JS_HILOG_INFO(...)
#define JS_HILOG_DEBUG(...)

#endif // CONFIG_HILOG

#endif // UI_APPEARANCE_LOG_H