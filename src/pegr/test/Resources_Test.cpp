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

#include "pegr/resource/Resources.hpp"

#include "pegr/test/Test_Util.hpp"
#include "pegr/text/Text.hpp"

namespace pegr {
namespace Test {

//@Test Resource OID test
void test_0101_resource_oid_test() {
    Resour::find_object("hello_world.txt");
    
    Text::Text_Res_Cptr res = Text::find_text_resource("hello_world.txt");
    verify_equals(std::string("Hello, world!"), res->m_string);
}

} // namespace Test
} // namespace pegr
