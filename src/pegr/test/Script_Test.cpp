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

#include <vector>

#include "pegr/script/Script_Util.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/test/Test_Util.hpp"
#include "pegr/except/Except.hpp"

namespace pegr {
namespace Test {

//@Test Identifying syntax errors
void test_0010_check_script_loading() {
    lua_State* l = Script::get_lua_state();
    Script::Unique_Regref sandbox(Script::new_sandbox());
    try {
        Script::Unique_Regref error(
            Script::load_lua_function("test/common/error_syntax.lua", sandbox));
    }
    catch (Except::Runtime& e) {
        lua_pop(l, 1);
        return;
    }
    lua_pop(l, 1);
    throw Except::Runtime("No syntax error");
}

//@Test Script Pop_Guard memory leaks
void test_0010_check_pop_guard() {
    lua_State* l = Script::get_lua_state();
    lua_pushnil(l);
    Script::Pop_Guard pg(1);
}

//@Test Script Unique_Regref memory leaks
void test_0010_check_guard_memory_leaks() {
    lua_State* l = Script::get_lua_state();
    Logger::log()->info("Testing explicit...");
    
    Script::Regref rr = Script::new_sandbox();
    Logger::log()->info("Should drop reference...");
    Script::drop_reference(rr);
    Script::push_reference(rr);
    if (!lua_isnil(l, -1)) {
        lua_pop(l, 1);
        throw Except::Runtime("Could not release reference!");
    }
    lua_pop(l, 1);
    Logger::log()->info("Testing guard...");
    
    Script::Regref ref;
    {
        Script::Unique_Regref sandbox(Script::new_sandbox());
        ref = sandbox;
        Logger::log()->info("Should drop reference...");
    }
    lua_gc(l, LUA_GCCOLLECT, 0);
    Script::push_reference(ref);
    if (!lua_isnil(l, -1)) {
        lua_pop(l, 1);
        throw Except::Runtime("Guard did not release reference!");
    }
    lua_pop(l, 1);
}

//@Test Script Shared_Regref memory leaks
void test_0010_check_guard_memory_leaks_shared() {
    lua_State* l = Script::get_lua_state();
    Script::Regref ref;
    ref = Script::new_sandbox();
    
    {
        Script::Shared_Regref shared1;
        {
            Script::Shared_Regref shared2 = Script::make_shared(ref);
            shared1 = shared2;
        }
    }
    
    Script::push_reference(ref);
    if (!lua_isnil(l, -1)) {
        lua_pop(l, 1);
        throw Except::Runtime("Shared guard did not release reference!");
    }
    lua_pop(l, 1);
    
    
}

//@Test More Unique_Regref tests
void test_0010_check_unique_regref() {
    int orig = Script::debug_get_total_grab_delta();
    
    Logger::log()->info("Original: %v", orig);
    Script::Unique_Regref rg;
    verify_equals(LUA_REFNIL, rg.get());
    
    lua_State* l = Script::get_lua_state();
    lua_newtable(l);
    Script::Regref raw = Script::grab_reference();
    
    Logger::log()->info("Assign %v", raw);
    rg = raw;
    Logger::log()->info("Pass");
    verify_equals(raw, rg.get());
    verify_equals(orig + 1, Script::debug_get_total_grab_delta());
    
    Logger::log()->info("Reset");
    rg.reset();
    verify_equals(orig, Script::debug_get_total_grab_delta());
    
    Logger::log()->info("Should be nil", raw);
    verify_equals(LUA_REFNIL, rg.get());
    
    Logger::log()->info("Making a vector of reg refs", orig);
    std::vector<Script::Unique_Regref> reg_refs;
    verify_equals(orig, Script::debug_get_total_grab_delta());
    
    Logger::log()->info("10 more", orig);
    for (int i = 0; i < 10; ++i) {
        lua_newtable(l);
        reg_refs.emplace_back(Script::grab_reference());
    }
    Logger::log()->info("10 more", orig);
    
    verify_equals(orig + 10, Script::debug_get_total_grab_delta());
    
}

} // namespace Test
} // namespace pegr
