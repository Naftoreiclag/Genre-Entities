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

#include <cstddef>
#include <sstream>
#include <map>
#include <cassert>

#include "pegr/script/Script_Util.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/test/Test_Util.hpp"

namespace pegr {
namespace Test {

typedef Script::Unique_Regref RG;

void assert_expected_string_table(
        lua_State* l,
        std::vector<std::pair<std::string, std::string> >& expected) {
    for (auto const& iter : expected) {
        lua_getfield(l, -1, iter.first.c_str());
        Script::Pop_Guard pop_guard(1);
        if (lua_isnil(l, -1)) {
            std::stringstream sss;
            sss << "No value for key \"" << iter.first << '"';
            throw Except::Runtime(sss.str());
        }
        
        std::size_t strlen;
        const char* strdata = lua_tolstring(l, -1, &strlen);
        std::string val(strdata, strlen);
        if (iter.second != val) {
            std::stringstream sss;
            sss << "Wrong value for key \""
                << iter.first
                << "\": Expected: \""
                << iter.second
                << "\" got: \""
                << val
                << '"';
            throw Except::Runtime(sss.str());
        }
    }
}

//@Test Script to_number_safe
void test_0028_to_number_safe() {
    lua_State* l = Script::get_lua_state();
    
    std::vector<const char*> success_tests = {
        
        // DO NOT PUT 0 IN THIS LIST
        // zero is indistinguishable from error
        "1",
        "2",
        "-4",
        "3.5",
        "2.718281828459",
        "0xDAFF0D11",
        "60",
        "315"
    };
    
    for (const char* str : success_tests) {
        lua_pushstring(l, str);
        Script::Pop_Guard pg(1);
        lua_Number expected = lua_tonumber(l, -1);
        lua_Number result;
        bool success = Script::Util::to_number_safe(-1, result);
        
        assert(expected != 0);
        
        if (!success) {
            std::stringstream sss;
            sss << "Fail to parse: " << str;
            throw Except::Runtime(sss.str());
        }
        
        if (expected != result) {
            std::stringstream sss;
            sss << "Expected: " << expected << " Got: " << result;
            throw Except::Runtime(sss.str());
        }
    }
    
    std::vector<const char*> fail_tests = {
        "Garlic",
        "four",
        "......",
        "1.2.3.4.5",
        "xXxXxXx"
    };
    
    for (const char* str : fail_tests) {
        lua_pushstring(l, str);
        Script::Pop_Guard pg(1);
        lua_Number expected = lua_tonumber(l, -1);
        lua_Number result = 12345;
        bool success = Script::Util::to_number_safe(-1, result);
        
        assert(expected == 0);
        
        if (success) {
            std::stringstream sss;
            sss << "Supposed to fail, but didnt't: " << str
                << " got: " << result
                << " (Note, if it says \"12345\""
                    "then something is really wrong.)";
            throw Except::Runtime(sss.str());
        }
    }
}

//@Test Script simple_deep_copy
void test_0028_simple_deep_copy() {
    lua_State* l = Script::get_lua_state();
    
    RG sandbox(Script::new_sandbox());
    RG table_fun(Script::load_lua_function("test/common/simple_table.lua", sandbox));
    
    std::vector<std::pair<std::string, std::string> > expected = {
        {"a", "apple"},
        {"b", "banana"},
        {"c", "cherry"},
        {"d", "durian"}
    };
    
    Script::Util::run_simple_function(table_fun, 1);
    Script::Pop_Guard pop_guard(1);
    
    assert_expected_string_table(l, expected);
    
    Script::Util::simple_deep_copy(-1);
    pop_guard.on_push(1);
    
    assert_expected_string_table(l, expected);
}

//@Test Script simple_deep_copy_recursive
void test_0028_simple_deep_copy_recursive() {
    lua_State* l = Script::get_lua_state();
    RG sandbox(Script::new_sandbox());
    RG table_fun(
            Script::load_lua_function("test/common/recursive_table.lua", sandbox));
    Script::Util::run_simple_function(table_fun, 1);
    Script::Pop_Guard pop_guard(1);
    Logger::log()->info("Beginning copy...");
    Script::Util::simple_deep_copy(-1);
    pop_guard.on_push(1);
    Logger::log()->info("Copy completed in finite time");
}

//@Test Script Helper for_pairs
void test_0028_for_pairs() {
    lua_State* l = Script::get_lua_state();
    
    RG sandbox(Script::new_sandbox());
    RG table_fun(Script::load_lua_function("test/common/simple_table.lua", sandbox));
    
    Script::Util::run_simple_function(table_fun, 1);
    Script::Pop_Guard pop_guard(1);
    
    std::map<std::string, std::string> expected = {
        {"a", "apple"},
        {"b", "banana"},
        {"c", "cherry"},
        {"d", "durian"}
    };
    
    std::map<std::string, std::string> got;
    
    bool success = true;
    auto body = [l, &got, &expected]() {
        std::size_t strlen;
        const char* strdata;
        
        lua_pushvalue(l, -2);
        lua_pushvalue(l, -2);
        Script::Pop_Guard pg2(2);
        strdata = lua_tolstring(l, -2, &strlen);
        std::string key(strdata, strlen);
        strdata = lua_tolstring(l, -1, &strlen);
        std::string val(strdata, strlen);
        
        got[key] = val;
        
        Logger::log()->verbose(1, "%v\t%v", key, val);
    };
    
    Script::Util::for_pairs(-1, [body, l]()->bool {
        body();
        return true;
    }, false);
    Script::Util::for_pairs(-1, [body, l]()->bool {
        body();
        lua_pop(l, 1);
        return true;
    }, true);
    
    for (auto key_value : expected) {
        auto iter = got.find(key_value.first);
        if (iter == got.end()) {
            std::stringstream ss;
            ss << "Resulting table missing key: ";
            ss << key_value.first;
            throw Except::Runtime(ss.str());
        }
        if (key_value.second != (*iter).second) {
            std::stringstream ss;
            ss << "Resulting table has wrong value for ";
            ss << key_value.first;
            ss << ": Expected: ";
            ss << key_value.second;
            ss << ": Got: ";
            ss << (*iter).second;
            throw Except::Runtime(ss.str());
        }
    }
}

//@Test Script Helper for_pairs with exception
void test_0028_for_pairs_exception() {
    lua_State* l = Script::get_lua_state();
    
    RG sandbox(Script::new_sandbox());
    RG table_fun(Script::load_lua_function("test/common/simple_table.lua", sandbox));
    
    Script::Util::run_simple_function(table_fun, 1);
    Script::Pop_Guard pop_guard(1);
    
    try {
        Script::Util::for_pairs(-1, []()->bool {
            throw Except::Runtime("orange juice");
        }, false);
    }
    catch (Except::Runtime& e) {
        if (std::string(e.what()) != "orange juice") {
            throw e;
        }
        return;
    }
    throw Except::Runtime("Error did not bubble up!");
}
//@Test Script Helper to_string
void test_0028_to_string() {
    lua_State* l = Script::get_lua_state();
    int original_size = lua_gettop(l);
    
    RG sandbox(Script::new_sandbox());
    RG table_fun(
            Script::load_lua_function("test/common/complex_tostring.lua", sandbox));
            
    Script::Util::run_simple_function(table_fun, 1);
    std::string resp = Script::Util::to_string(-1);
    lua_pop(l, 1);
    
    if (resp != "bottom") {
        throw Except::Runtime("Did not reach the bottom of tostring");
    }
    
    if (original_size != lua_gettop(l)) {
        throw Except::Runtime("Unbalanced");
    }
}

//@Test Script Helper for_number_pairs_sorted
void test_0028_for_pairs_number_sorted() {
    lua_State* l = Script::get_lua_state();
    
    RG sandbox(Script::new_sandbox());
    RG table_fun(Script::load_lua_function(
            "test/common/sparse_array.lua", sandbox));
    
    Script::Util::run_simple_function(table_fun, 1);
    Script::Pop_Guard pop_guard(1);
    
    std::string expected = "Shall I compare thee to a summer's day?";
    
    std::stringstream got;
    
    auto body = [&]()->bool {
        std::size_t strlen;
        const char* strdata = lua_tolstring(l, -1, &strlen);
        std::string str(strdata, strlen);
        got << str;
        return true;
    };
    
    Script::Util::for_number_pairs_sorted(-1, body, false);
    
    Logger::log()->info(expected);
    
    if (expected != got.str()) {
        std::stringstream sss;
        sss << "Expected shakespeare, got: \""
            << got.str()
            << '"';
        throw Except::Runtime(sss.str());
    }
    
    expected = "day?summer's a to thee compare I Shall ";
    got = std::stringstream();
    
    Script::Util::for_number_pairs_sorted(-1, body, false, true);
    
    Logger::log()->info(expected);
    
    if (expected != got.str()) {
        std::stringstream sss;
        sss << "Expected shakespeare, got: \""
            << got.str()
            << '"';
        throw Except::Runtime(sss.str());
    }
}

//@Test Script Helper unique regref manager
void test_0028_unique_regref_manager() {
    lua_State* l = Script::get_lua_state();
    
    lua_newtable(l);
    Script::Unique_Regref ref(Script::grab_reference());
    
    Script::Util::Unique_Regref_Manager urm;
    
    Script::Regref wref1 = urm.add_lua_value(ref.get());
    
    verify_not_equals(ref.get(), wref1);
    
    Script::Regref wref2 = urm.add_lua_value(ref.get());
    
    verify_equals(wref1, wref2);
}

} // namespace Test
} // namespace pegr
