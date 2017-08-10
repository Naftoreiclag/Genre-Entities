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
#include <algorithm>
#include <memory>

#include "pegr/test/Test_Util.hpp"

namespace pegr {
namespace Test {

int foo_num = 0;
    
struct Foo {
    Foo() {
        ++foo_num;
    }
    Foo(const Foo& foo) {
        ++foo_num;
    }
    Foo(Foo&& foo) {
        ++foo_num;
    }
    Foo& operator =(const Foo& foo) = default;
    Foo& operator =(Foo&& foo) = default;
    ~Foo() {
        --foo_num;
    }
};
    
//@Test Unique Ptr Test
void test_0002_unique_ptr_test() {
    
    std::vector<std::unique_ptr<Foo> > vec;
    vec.emplace_back(std::make_unique<Foo>());
    
    verify_equals(1, foo_num);
    for (int i = 0; i < 10; ++i) {
        vec.emplace_back(std::make_unique<Foo>());
    }
    verify_equals(11, foo_num);
    
    vec.clear();
    
    verify_equals(0, foo_num);
}

} // namespace Test
} // namespace pegr
