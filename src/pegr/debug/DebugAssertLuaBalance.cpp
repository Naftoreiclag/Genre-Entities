#include "pegr/debug/DebugAssertLuaBalance.hpp"

#include <cassert>
#include <sstream>

#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"

namespace pegr {
namespace Debug {

LuaBalanceGuard::LuaBalanceGuard(int delta, const char* msg, int line)
: m_delta(delta)
, m_msg(msg)
, m_line(line) {
    lua_State* l = Script::get_lua_state();
    m_original_size = lua_gettop(l);
}

LuaBalanceGuard::~LuaBalanceGuard() {
    lua_State* l = Script::get_lua_state();
    int got_delta = lua_gettop(l) - m_original_size;
    if (m_delta != got_delta) {
        std::stringstream sss;
        sss << "Balance contract defied! Expected/Got: ";
        if (m_delta >= 0) { sss << '+'; }
        sss << m_delta
            << '/';
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
