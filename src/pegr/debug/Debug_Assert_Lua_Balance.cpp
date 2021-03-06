/*
 *  Copyright 2017 James Fong
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

#include "pegr/debug/Debug_Assert_Lua_Balance.hpp"

#include <cassert>
#include <sstream>

#include "pegr/logger/Logger.hpp"
#include "pegr/script/Script.hpp"

namespace pegr {
namespace Debug {

LuaBalanceGuard::LuaBalanceGuard(std::vector<int> deltas,
        const char* msg, int line)
: m_deltas(deltas)
, m_msg(msg)
, m_line(line) {
    if (m_deltas.size() == 0) {
        m_deltas.push_back(0);
    }
    
    lua_State* l = Script::get_lua_state();
    m_original_size = lua_gettop(l);
}

LuaBalanceGuard::~LuaBalanceGuard() {
    lua_State* l = Script::get_lua_state();
    int got_delta = lua_gettop(l) - m_original_size;
    
    bool found = false;
    for (int delta : m_deltas) {
        if (delta == got_delta) {
            found = true;
            break;
        }
    }
    
    if (!found) {
        std::stringstream sss;
        sss << "Balance contract defied! Expected/Got: ";
        bool delim = false;
        if (m_deltas.size() > 1) {
            sss << '[';
        }
        for (int delta : m_deltas) {
            if (delim) sss << '|';
            delim = true;
            if (delta >= 0) sss << '+';
            sss << delta;
        }
        if (m_deltas.size() > 1) {
            sss << ']';
        }
        sss << '/';
        if (got_delta >= 0) { sss << '+'; }
        sss << got_delta;
        if (m_msg) {
            if (m_line < 0) {
                sss << " msg: "
                    << m_msg;
            } else {
                sss << " file: "
                    << m_msg
                    << " line: "
                    << m_line;
            }
        }
        Logger::log()->fatal(sss.str());
        assert(false && "Balance contract defied!");
    }
}

} // namespace Debug
} // namespace pegr
