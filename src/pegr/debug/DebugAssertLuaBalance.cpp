#include "pegr/debug/DebugAssertLuaBalance.hpp"

#include <cassert>
#include <sstream>

#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"

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
