/*
 * Copyright (c) 2021-2022 Samsung Electronics Co., Ltd All Rights Reserved
 *
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
#pragma once

#include <exception>
#include <string>
#include <vector>

namespace aitt {

class AittException : public std::exception {
  public:
    enum ErrCode {
        INVALID_ARG,
        NO_MEMORY_ERR,
        OPERATION_FAILED,
        SYSTEM_ERR,
        MQTT_ERR,
        NO_DATA_ERR,
    };

    AittException(ErrCode err_code);
    AittException(ErrCode err_code, const std::string& custom_err_msg);

    ErrCode getErrCode();
    virtual const char* what() const throw() override;

  private:
    ErrCode err_code;
    std::string err_msg;

    std::string getErrString() const;
};

}  // namespace aitt
