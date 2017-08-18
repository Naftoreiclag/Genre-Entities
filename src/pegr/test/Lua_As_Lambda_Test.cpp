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

//@Test Lambda closure
void test_0003_lambda_closure() {
    std::function<int()> f = make_counter();
    
    verify_equals(1, f());
    verify_equals(2, f());
    verify_equals(3, f());
    verify_equals(4, f());
}

// Lua as Lambda Test?
void test_0029_lua_as_lambda() {
}

} // namespace Test
} // namespace pegr
