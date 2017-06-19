#include "pegr/script/ScriptHelper.hpp"

#include <cassert>

#include "pegr/script/Script.hpp"

namespace pegr {
namespace Script {
namespace Helper {

void for_pairs(int table_idx, std::function<bool()> func, bool pops_value) {
    lua_State* l = Script::get_lua_state();
    int original_size; assert(original_size = lua_gettop(l)); // Balance sanity
    
    table_idx = Script::absolute_idx(table_idx);
    
    lua_pushnil(l);
    while (lua_next(l, table_idx) != 0) {
        bool cont = func();
        if (!pops_value) {
            lua_pop(l, 1); // Eat value
        }
        if (!cont) {
            lua_pop(l, 1); // Eat key
            break;
        }
    }
    
    assert(original_size == lua_gettop(l)); // Balance sanity
}

void simple_deep_copy(int table_idx) {
    lua_State* l = Script::get_lua_state();
    int original_size; assert(original_size = lua_gettop(l)); // Balance sanity
    
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
    
    assert(original_size + 1 == lua_gettop(l)); // Balance sanity
}

std::string to_string(int idx, int max_recusions) {
    lua_State* l = Script::get_lua_state();
    int original_size; assert(original_size = lua_gettop(l)); // Balance sanity
    idx = Script::absolute_idx(idx);
    
    Script::push_reference(Script::m_luaglob_tostring);
    lua_pushvalue(l, idx);
    
    std::size_t strsize;
    const char* strdata = lua_tolstring(l, -1, &strsize);
    
    if (!strdata) {
    }
    
    assert(original_size + 1 == lua_gettop(l)); // Balance sanity
}

} // namespace Helper
} // namespace Script
} // namespace pegr
