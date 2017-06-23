#include "pegr/script/ScriptHelper.hpp"

#include <stdexcept>
#include <cassert>

#include "pegr/debug/DebugMacros.hpp"
#include "pegr/script/Script.hpp"

namespace pegr {
namespace Script {
namespace Helper {

void for_pairs(int table_idx, std::function<bool()> func, bool pops_value) {
    lua_State* l = Script::get_lua_state();
    table_idx = Script::absolute_idx(table_idx);
    lua_pushnil(l);
    while (lua_next(l, table_idx) != 0) {
        Script::Pop_Guard pg;
        if (!pops_value) {
            pg.m_n = 1; // Eat value ourselves
        }
        // Assume we will need to eat the key ourselves
        // in case the function rasies errors or breaks the loop
        pg.m_n += 1;
        bool cont = func();
        if (!cont) {
            break;
        }
        // If we got to this point, the key no longer needs to be eaten by us
        pg.m_n -= 1;
    }
}

void simple_deep_copy(int table_idx) {
    assert_balance(1);
    
    lua_State* l = Script::get_lua_state();
    
    table_idx = Script::absolute_idx(table_idx);
    lua_newtable(l); // TODO: preallocate enough space
    lua_pushnil(l);
    while (lua_next(l, table_idx) != 0) {
        // Copy the reference to the key so there is one for iterating with
        lua_pushvalue(l, -2);
        
        // Copy the value if it is a table
        if (lua_type(l, -2) == LUA_TTABLE) {
            simple_deep_copy(-2);
        } else {
            lua_pushvalue(l, -2);
        }
        
        // Set the key-value pair
        lua_settable(l, -5);
        
        // Remove the original value reference
        lua_pop(l, 1);
        
        // (A reference to the key is still on the stack for iteration)
    }
}

void run_simple_function(Script::Regref ref, int nresults) {
    Script::push_reference(ref);
    Script::run_function(0, nresults);
}

std::string to_string(int idx, const char* def, int max_recusions) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    idx = Script::absolute_idx(idx);
    
    Script::push_reference(Script::m_luaglob_tostring);
    
    lua_pushvalue(l, idx);
    for (int count = 0; count <= max_recusions; ++count) {
        std::size_t strsize;
        const char* strdata = lua_tolstring(l, -1, &strsize);
        
        if (!strdata) {
            lua_pushvalue(l, -2); // tostring() function
            lua_pushvalue(l, -2); // the pesky string
            Script::run_function(1, 1);
            lua_remove(l, -2); // pop off the old pesky string
            continue;
        }
        
        lua_pop(l, 2); // remove cached tostring and the value
        return std::string(strdata, strsize);
    }
    lua_pop(l, 2); // remove cached tostring and the value
    if (def) {
        return def;
    }
    throw std::runtime_error(
        "Maximum recusion depth reached when trying to convert a Lua "
        "value into a string through tostring()");
}

const char* GENERIC_TO_STRING_DEFAULT = 
        "[MAX TOSTRING() RECURSION DEPTH EXCEEDED]";

} // namespace Helper
} // namespace Script
} // namespace pegr
