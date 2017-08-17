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

#include "pegr/script/Script_Util.hpp"

#include <algorithm>
#include <cassert>

#include "pegr/debug/Debug_Macros.hpp"
#include "pegr/except/Except.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/script/Script.hpp"

namespace pegr {
namespace Script {
namespace Util {

void for_pairs(int table_idx, std::function<bool()> func, bool pops_value) {
    assert_balance(0);
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
        if (!func()) {
            break;
        }
        // If we got to this point, the key no longer needs to be eaten by us
        pg.on_pop(1);
    }
}

std::vector<lua_Number> get_number_keys(int table_idx) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    table_idx = Script::absolute_idx(table_idx);
    lua_pushnil(l);
    std::vector<lua_Number> retval;
    while (lua_next(l, table_idx) != 0) {
        lua_pop(l, 1);
        if (!lua_isnumber(l, -1)) {
            continue;
        }
        lua_Number num = lua_tonumber(l, -1);
        retval.push_back(num);
    }
    return retval;
}

void for_number_pairs_sorted(int table_idx, std::function<bool()> func, 
        bool pops_value, bool reversed_order) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    table_idx = Script::absolute_idx(table_idx);
    std::vector<lua_Number> keys = get_number_keys(table_idx);
    if (reversed_order) {
        std::sort(keys.begin(), keys.end(), std::greater<lua_Number>());
    } else {
        std::sort(keys.begin(), keys.end());
    }
    for (lua_Number key : keys) {
        Script::Pop_Guard pg;
        lua_pushnumber(l, key);
        pg.on_push(1);
        lua_pushnumber(l, key);
        lua_gettable(l, table_idx);
        if (!pops_value) {
            pg.on_push(1); // We will eat the value ourselves
        }
        if (!func()) {
            break;
        }
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
    assert_balance(nresults, 1);
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
    throw Except::Runtime(
        "Maximum recusion depth reached when trying to convert a Lua "
        "value into a string through tostring()");
}

const char* GENERIC_TO_STRING_DEFAULT = 
        "[MAX tostring() RECURSION DEPTH EXCEEDED]";

bool to_number_safe(int idx, lua_Number& num) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    if (lua_isnumber(l, idx)) {
        num = lua_tonumber(l, idx);
        return true;
    }
    return false;
    /*
    // This is pretty much how Lua converts strings under the hood:
    int val_type = lua_type(l, idx);
    if (val_type == LUA_TNUMBER) {
        num = lua_tonumber(l, idx);
        return true;
    }
    if (val_type == LUA_TSTRING) {
        const char* str = lua_tostring(l, idx);
        Logger::log()->info(str);
        char* str_end;
        double c_result = std::strtod(str, &str_end);
        if (str_end == str) {
            return false;
        }
        // Hexadecimal
        if (*str_end == 'X' || *str_end == 'x') {
            c_result = std::strtoul(str, &str_end, 16);
        }
        while (std::isspace(*str_end)) {
            ++str_end;
        }
        if (*str_end != '\0') {
            return false;
        }
        num = c_result;
        return true;
    }
    */
}

void push_new_weak_table(const char* mode) {
    assert_balance(1);
    lua_State* l = Script::get_lua_state();
        
    // Create a new table
    lua_newtable(l); // +1
    
    // Create a metatable to indicate that its values are weak
    lua_createtable(l, 0, 1); // +1
    lua_pushstring(l, "__mode"); // +1
    lua_pushstring(l, mode); // +1
    lua_rawset(l, -3); // -2
    
    // Make table weak
    lua_setmetatable(l, -2); // -1
}


    
Unique_Regref_Manager::Unique_Regref_Manager() {
    lua_State* l = Script::get_lua_state();
    assert(l);
    assert_balance(0);
    lua_newtable(l);
    m_lua_uniques_lookup.reset(Script::grab_reference());
}

Script::Regref Unique_Regref_Manager::add_lua_value(Script::Regref val_ref) {
    lua_State* l = Script::get_lua_state();
    assert(l);
    assert_balance(0);
    
    Script::push_reference(m_lua_uniques_lookup.get());
    Script::push_reference(val_ref);
    lua_pushvalue(l, -1);
    lua_rawget(l, -3);
    
    // No unique regref assigned
    if (lua_isnil(l, -1)) {
        lua_pop(l, 1);
        
        /* At this point:
         * 
         * -1 val to add
         * -2 "uniques" table
         */
        
        lua_pushvalue(l, -1);
        Script::Unique_Regref unique_ref = Script::grab_reference();
        Script::Regref weak_ref = unique_ref.get();
        
        lua_pushinteger(l, weak_ref);
        lua_rawset(l, -3);
        
        lua_pop(l, 1);
        
        m_lua_uniques.emplace_back(std::move(unique_ref));
        
        return weak_ref;
    } else {
        assert(lua_isnumber(l, -1));
        Script::Regref weak_ref = lua_tointeger(l, -1);
        lua_pop(l, 3);
        return weak_ref;
    }
}

const std::vector<Script::Unique_Regref>& 
        Unique_Regref_Manager::get_lua_uniques() const {
    return m_lua_uniques;
}

std::vector<Script::Unique_Regref> Unique_Regref_Manager::release() {
    m_lua_uniques_lookup.reset();
    return std::move(m_lua_uniques);
}

void Unique_Regref_Manager::clear() {
    m_lua_uniques_lookup.reset();
    m_lua_uniques.clear();
}
    

} // namespace Helper
} // namespace Script
} // namespace pegr
