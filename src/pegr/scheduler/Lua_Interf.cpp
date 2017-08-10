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

#include "pegr/scheduler/Lua_Interf.hpp"

#include <cassert>

#include "pegr/debug/Debug_Macros.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/script/Lua_Interf_Util.hpp"

namespace pegr {
namespace Sched {
namespace LI {

Script::Regref n_tasks_table = LUA_REFNIL;

void initialize_tables(lua_State* l) {
    assert_balance(0);
    lua_newtable(l);
    n_tasks_table = Script::grab_reference();
}

void cleanup_tables(lua_State* l) {
    assert_balance(0);
    Script::drop_reference(n_tasks_table);
    n_tasks_table = LUA_REFNIL;
}

const luaL_Reg n_api_safe[] = {
    {"schedule_task", li_schedule_task},
    
    // End of the list
    {nullptr, nullptr}
};

void initialize_expose_global_functions(lua_State* l) {
    assert_balance(0);
    Script::multi_expose_c_functions(n_api_safe);
}

void initialize() {
    assert(Script::is_initialized());
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    initialize_tables(l);
    initialize_expose_global_functions(l);
}

void clear() {
    assert(Script::is_initialized());
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    cleanup_tables(l);
    initialize_tables(l);
}

void cleanup() {
    assert(Script::is_initialized());
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    cleanup_tables(l);
}
    
int li_schedule_task(lua_State* l) {
    Script::Util::generic_li_add_to_res_table(l, n_tasks_table);
    return 0;
}

int li_edit_task(lua_State* l) {
    Script::Util::generic_li_edit_from_res_table(l, n_tasks_table);
    return 1;
}
    
} // namspace LI
} // namespace Sched
} // namespace pegr
