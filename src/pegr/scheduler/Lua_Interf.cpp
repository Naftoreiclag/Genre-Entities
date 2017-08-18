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
#include <map>
#include <string>

#include "pegr/debug/Debug_Macros.hpp"
#include "pegr/script/Lua_Interf_Util.hpp"
#include "pegr/script/Script.hpp"

namespace pegr {
namespace Schedu {
namespace LI {

Scripted_Event::Scripted_Event()
: Event() {}
Scripted_Event::~Scripted_Event() {}

Event::Type Scripted_Event::get_type() const {
    return Event::Type::SCRIPTED;
}

void Scripted_Event::trigger() {}

Script::Unique_Regref n_staged_events;

void initialize_tables(lua_State* l) {
    assert_balance(0);
    lua_newtable(l);
    n_staged_events = Script::grab_unique_reference();
}

void cleanup_tables(lua_State* l) {
    assert_balance(0);
    n_staged_events.reset();
}

const luaL_Reg n_api_safe[] = {
    {"add_event", li_add_event},
    {"edit_event", li_edit_event},
    {"hook_listener", li_hook_listener},
    {"call_event", li_call_event},
    
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

int li_add_event(lua_State* l) {
    if (Schedu::get_global_state() != GlobalState::MUTABLE) {
        luaL_error(l, "add_event is only available during setup");
    }
    Script::Util::generic_li_add_to_res_table(l, n_staged_events.get());
    return 0;
}

int li_edit_event(lua_State* l) {
    if (Schedu::get_global_state() != GlobalState::MUTABLE) {
        luaL_error(l, "edit_event is only available during setup");
    }
    Script::Util::generic_li_edit_from_res_table(l, n_staged_events.get());
    return 1;
}
    
int li_call_event(lua_State* l) {
    //luaL_error(l, "Not implemented");
    return 0;
}
int li_hook_listener(lua_State* l) {
    //luaL_error(l, "Not implemented");
    return 0;
}
    
} // namspace LI
} // namespace Schedu
} // namespace pegr
