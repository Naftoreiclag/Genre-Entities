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

#include "pegr/except/Except.hpp"
#include "pegr/gensys/Compiler.hpp"
#include "pegr/gensys/Interm_Types.hpp"
#include "pegr/gensys/Lua_Interf.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/script/Script_Util.hpp"

namespace pegr {
namespace Test {

//@Test Gensys primitive from Lua values
void test_0080_00_gensys_primitive() {
    Gensys::Interm::Prim prim;
    
    lua_State* l = Script::get_lua_state();
    
    Script::Unique_Regref sandbox(Script::new_sandbox());
    Script::Unique_Regref func(
        Script::load_lua_function("test/common/prim_cstr_table.lua", 
                sandbox.get()));
        
    Script::Util::run_simple_function(func.get(), 1);
    
    prim = Gensys::LI::parse_primitive(-1);
    lua_pop(l, 1);
    
    if (prim.get_type() != Gensys::Interm::Prim::Type::FUNC) {
        throw Except::Runtime("Expected FUNC!");
    }
}

} // namespace Test
} // namespace pegr
