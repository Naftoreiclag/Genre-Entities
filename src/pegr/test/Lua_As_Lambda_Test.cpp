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
 
#include <functional>

#include "pegr/debug/Debug_Macros.hpp"
#include "pegr/except/Except.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/test/Test_Util.hpp"

namespace pegr {
namespace Test {

std::function<int()> make_counter() {
    int num = 0;
    return [=]() mutable {
        return ++num;
    };
}

std::function<int()> make_lua_counter() {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    lua_newtable(l);
    lua_pushinteger(l, 0);
    lua_setfield(l, -2, "x");
    Script::Shared_Regref regref = Script::make_shared(
            Script::grab_unique_reference().release());
    return [=]() {
        Script::push_reference(regref->get());
        lua_getfield(l, -1, "x");
        int val = lua_tonumber(l, -1);
        lua_pop(l, 1);
        ++val;
        lua_pushinteger(l, val);
        lua_setfield(l, -2, "x");
        lua_pop(l, 1);
        return val;
    };
}

void check_counters(std::function<int()> f, std::function<int()> f2) {
    verify_equals(1, f());
    verify_equals(2, f());
    verify_equals(3, f());
    verify_equals(4, f());
    
    verify_equals(1, f2());
    verify_equals(5, f());
    
    verify_equals(2, f2());
    verify_equals(6, f());
    
    verify_equals(3, f2());
    verify_equals(7, f());
    
    verify_equals(4, f2());
    verify_equals(8, f());
}

//@Test Lambda closure
void test_0003_lambda_closure() {
    std::function<int()> f = make_counter();
    std::function<int()> f2 = make_counter();
    check_counters(f, f2);
}

//@Test Lua as Lambda Test
void test_0029_lua_as_lambda() {
    std::function<int()> f = make_lua_counter();
    std::function<int()> f2 = make_lua_counter();
    check_counters(f, f2);
}

} // namespace Test
} // namespace pegr
