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
            pg.on_push(1); // We will eat the value ourselves
        }
        // Assume we will need to eat the key ourselves
        // in case the function raises errors or breaks the loop
        pg.on_push(1);
        bool cont = func();
        if (!cont) {
            break;
        }
        // If we got to this point, the key no longer needs to be eaten by us
        pg.on_pop(1);
    }
}

/**
 * @brief Recursive helper for simple_deep_copy
 * @param l The Lua state to use
 * @param table_idx Absolute index for the table to be copied
 * @param dupe_idx Absolute index for a table that maps originals to copies
 */
void simple_deep_copy_helper(lua_State* l, int table_idx, int dupe_idx) {
    assert_balance(1);
    
    // Don't copy non-tables
    if (!lua_istable(l, table_idx)) {
        lua_pushvalue(l, table_idx);
        return;
    }
    
    // Check if we already made a copy before doing anything
    lua_pushvalue(l, table_idx);
    lua_rawget(l, dupe_idx);
    if (lua_istable(l, -1)) {
        return;
    }
    lua_pop(l, 1);
    
    lua_newtable(l); // TODO: preallocate enough space
    int copy_idx = lua_gettop(l);
    
    // Store this table as a copy of the table at table_idx, even though
    // we aren't finished making it yet. (If we try to wait until we
    // are finished, it's possible that we will be waiting forever.)
    lua_pushvalue(l, table_idx);
    lua_pushvalue(l, copy_idx);
    lua_rawset(l, dupe_idx);
    
    lua_pushnil(l);
    while (lua_next(l, table_idx) != 0) {
        int val_idx = lua_gettop(l);
        
        // Copy the reference to the key so there is one for iterating with
        lua_pushvalue(l, -2);
        
        simple_deep_copy_helper(l, val_idx, dupe_idx);
        
        // Set the key-value pair
        lua_settable(l, copy_idx);
        
        // Remove the original value reference
        lua_pop(l, 1);
        
        // (A reference to the key is still on the stack for iteration)
    }
}

void simple_deep_copy(int table_idx) {
    assert_balance(1);
    lua_State* l = Script::get_lua_state();
    table_idx = Script::absolute_idx(table_idx);
    
    /* This table maps original tables to the copied forms.
     * This allows for duplicate reference values to be preserved and 
     * avoids crashing for recursive cases.
     */
    lua_newtable(l);
    int dupe_idx = lua_gettop(l);
    
    simple_deep_copy_helper(l, table_idx, dupe_idx);
    
    lua_remove(l, dupe_idx);
}

void run_simple_function(Script::Regref ref, int nresults) {
    assert_balance(nresults);
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
        "[MAX tostring() RECURSION DEPTH EXCEEDED]";

} // namespace Helper
} // namespace Script
} // namespace pegr
