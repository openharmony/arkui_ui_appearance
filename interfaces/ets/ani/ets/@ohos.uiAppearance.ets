/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { BusinessError, AsyncCallback } from '@ohos.base';

export enum ErrCode {
    SUCCEEDED = 0,
    PERMISSION_ERR = 201,
    INVALID_ARG = 401,
    SYS_ERR = 500001,
};
export default namespace uiAppearance {
    loadLibrary("ui_appearance_ani");
    export enum DarkMode  {
        ALWAYS_DARK = 0,
        ALWAYS_LIGHT = 1
    };
    export native function setDarkMode(mode: DarkMode, callback: AsyncCallback<void>): void;
    export native function setDarkMode(mode: DarkMode): Promise<void>;
    export native function getDarkMode(): DarkMode;
    export native function setFontScale(fontScale: number): Promise<void>;
    export native function getFontScale(): number;
    export native function setFontWeightScale(fontWeightScale: number): Promise<void>;
    export native function getFontWeightScale(): number;
}
