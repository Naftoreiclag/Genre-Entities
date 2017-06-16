/*
 *  Copyright 2016-2017 James Fong
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "pegr/logger/Logger.hpp"

#include <cassert>

#include <easylogging++.h>

INITIALIZE_EASYLOGGINGPP

namespace pegr {
namespace Logger {

el::Logger* m_common_logger = nullptr;

void initialize() {
    assert(!m_common_logger);
    m_common_logger = el::Loggers::getLogger("pegr");
}

// TODO: constexpr?
el::Logger* log() {
    assert(m_common_logger);
    return m_common_logger;
}

el::Logger* alog(const char* addon_name) {
    return el::Loggers::getLogger(addon_name);
}

} // namespace Logger
} // namespace pegr
