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

#include <sstream>
#include <iomanip>

#include "pegr/gensys/Interm_Types.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/script/Script_Util.hpp"
#include "pegr/except/Except.hpp"

namespace pegr {
namespace Test {

//@Test Single gensys primitive
void test_0030_gensys_primitive() {
    lua_State* l = Script::get_lua_state();
    int stack_size = lua_gettop(l);
    
    Gensys::Interm::Prim prim;
    
    if (!prim.is_error()) {
        throw Except::Runtime("Prim is not an error!");
    }
    
    prim.set_f32(0.5f);
    
    if (prim.get_type() != Gensys::Interm::Prim::Type::F32) {
        throw Except::Runtime("Type must be F32!");
    }
    
    double pi = 3.14159265358979323846264338328d;
    {    
        float pi_f = pi;
        
        std::stringstream ss1;
        ss1 << std::setprecision(100) << pi;
        Logger::log()->info("Double precision: %v", ss1.str());
        std::stringstream ss2;
        ss2 << std::setprecision(100) << pi_f;
        Logger::log()->info("Float precision: %v", ss2.str());
        
        Logger::log()->info("Size of float: %v", sizeof(float));
        Logger::log()->info("Size of double: %v", sizeof(double));
        Logger::log()->info("Size of Prim: %v", sizeof(Gensys::Interm::Prim));
    }
    prim.set_f64(pi);
    
    if (prim.get_type() != Gensys::Interm::Prim::Type::F64) {
        throw Except::Runtime("Type must be F64!");
    }
    
    if (prim.get_f64() != pi) {
        std::stringstream ss;
        ss << "Wrong double: ";
        ss << prim.get_f64();
        throw Except::Runtime(ss.str());
    }
    std::stringstream ss;
    ss << prim.get_f64();
    Logger::log()->info("Correct double: %v", ss.str());
    
    Script::Unique_Regref sandbox(Script::new_sandbox());
    Script::Regref table_fun = 
            Script::load_lua_function(
                    "test/common/simple_table.lua", sandbox.get()).release();
    
    prim.set_function(Script::make_shared(table_fun));
    
    if (prim.get_type() != Gensys::Interm::Prim::Type::FUNC) {
        throw Except::Runtime("Type must be function!");
    }
    
    Script::Util::run_simple_function(prim.get_function()->get(), 1);
    lua_getfield(l, -1, "a");
    
    std::size_t strsize;
    const char* strdata = lua_tolstring(l, -1, &strsize);
    
    std::string a_val(strdata, strsize);
    
    if (a_val != "apple") {
        std::stringstream ss;
        ss << "Loaded function failed: ";
        ss << a_val;
        throw Except::Runtime(ss.str());
    }
    Logger::log()->info("Correct function return val: %v", a_val);
    
    lua_pop(l, 1);
    lua_pop(l, 1);
    
    std::string str = "Hello world!";
    
    prim.set_string(str);
    
    if (prim.get_type() != Gensys::Interm::Prim::Type::STR) {
        throw Except::Runtime("Type must be string!");
    }
    
    if (prim.get_string() != str) {
        std::stringstream ss;
        ss << "Wrong string returned: ";
        ss << prim.get_string();
        throw Except::Runtime(ss.str());
    }
    Logger::log()->info("Correct string retrieved: %v", prim.get_string());
    
    if (lua_gettop(l) != stack_size) {
        throw Except::Runtime("Unbalanced!");
    }
}

//@Test Reassignment of gensys primitives
void test_0030_gensys_primitive_multiple() {
    lua_State* l = Script::get_lua_state();
    int stack_size = lua_gettop(l);
    
    Gensys::Interm::Prim prim_foo;
    Gensys::Interm::Prim prim_bar;
    
    Script::Unique_Regref sandbox(Script::new_sandbox());
    Script::Shared_Regref func_foo = Script::make_shared(
            Script::load_lua_function("test/common/return_foo.lua", 
                    sandbox.get()).release());
    Script::Shared_Regref func_bar = Script::make_shared(
            Script::load_lua_function("test/common/return_bar.lua", 
                    sandbox.get()).release());
    
    prim_foo.set_function(func_foo);
    prim_bar.set_function(func_bar);
    
    Script::Util::run_simple_function(prim_foo.get_function()->get(), 1);
    std::string str_foo = Script::Util::to_string(-1);
    lua_pop(l, 1);
    
    Script::Util::run_simple_function(prim_bar.get_function()->get(), 1);
    std::string str_bar = Script::Util::to_string(-1);
    lua_pop(l, 1);
    
    if (str_foo != "foo") {
        throw Except::Runtime("Expected \"foo\"");
    }
    
    if (str_bar != "bar") {
        throw Except::Runtime("Expected \"bar\"");
    }
    
    Gensys::Interm::Prim prim_maybe_foo;
    prim_maybe_foo.set_f32(2.718f);
    
    if (prim_maybe_foo.get_type() != Gensys::Interm::Prim::Type::F32) {
        throw Except::Runtime("Expected F32");
    }
    
    prim_maybe_foo = prim_foo;
    
    if (prim_maybe_foo.get_type() != Gensys::Interm::Prim::Type::FUNC) {
        throw Except::Runtime("Expected FUNC");
    }
    
    Script::Util::run_simple_function(prim_maybe_foo.get_function()->get(), 1);
    std::string str_maybe_foo = Script::Util::to_string(-1);
    lua_pop(l, 1);
    
    if (str_foo != str_maybe_foo) {
        throw Except::Runtime("Wrong assignment of functions!");
    }
    
    if (lua_gettop(l) != stack_size) {
        throw Except::Runtime("Unbalanced!");
    }
}

} // namespace Test
} // namespace pegr
