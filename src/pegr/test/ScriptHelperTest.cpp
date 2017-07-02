#include <cstddef>
#include <sstream>
#include <map>
#include <cassert>

#include "pegr/script/ScriptHelper.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"

namespace pegr {
namespace Test {

typedef Script::Regref_Guard RG;

void assert_expected_string_table(
        lua_State* l,
        std::vector<std::pair<std::string, std::string> >& expected) {
    for (auto const& iter : expected) {
        lua_getfield(l, -1, iter.first.c_str());
        Script::Pop_Guard pop_guard(1);
        if (lua_isnil(l, -1)) {
            std::stringstream sss;
            sss << "No value for key \"" << iter.first << '"';
            throw std::runtime_error(sss.str());
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
            throw std::runtime_error(sss.str());
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
        bool success = Script::Helper::to_number_safe(-1, result);
        
        assert(expected != 0);
        
        if (!success) {
            std::stringstream sss;
            sss << "Fail to parse: " << str;
            throw std::runtime_error(sss.str());
        }
        
        if (expected != result) {
            std::stringstream sss;
            sss << "Expected: " << expected << " Got: " << result;
            throw std::runtime_error(sss.str());
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
        bool success = Script::Helper::to_number_safe(-1, result);
        
        assert(expected == 0);
        
        if (success) {
            std::stringstream sss;
            sss << "Supposed to fail, but didnt't: " << str
                << " got: " << result
                << " (Note, if it says \"12345\""
                    "then something is really wrong.)";
            throw std::runtime_error(sss.str());
        }
    }
}

//@Test Script simple_deep_copy
void test_0028_simple_deep_copy() {
    lua_State* l = Script::get_lua_state();
    
    RG sandbox(Script::new_sandbox());
    RG table_fun(Script::load_lua_function("test/simple_table.lua", sandbox));
    
    std::vector<std::pair<std::string, std::string> > expected = {
        {"a", "apple"},
        {"b", "banana"},
        {"c", "cherry"},
        {"d", "durian"}
    };
    
    Script::Helper::run_simple_function(table_fun, 1);
    Script::Pop_Guard pop_guard(1);
    
    assert_expected_string_table(l, expected);
    
    Script::Helper::simple_deep_copy(-1);
    pop_guard.on_push(1);
    
    assert_expected_string_table(l, expected);
}

//@Test Script simple_deep_copy_recursive
void test_0028_simple_deep_copy_recursive() {
    lua_State* l = Script::get_lua_state();
    RG sandbox(Script::new_sandbox());
    RG table_fun(
            Script::load_lua_function("test/recursive_table.lua", sandbox));
    Script::Helper::run_simple_function(table_fun, 1);
    Script::Pop_Guard pop_guard(1);
    Logger::log()->info("Beginning copy...");
    Script::Helper::simple_deep_copy(-1);
    pop_guard.on_push(1);
    Logger::log()->info("Copy completed in finite time");
}

//@Test Script Helper for_pairs
void test_0028_for_pairs() {
    lua_State* l = Script::get_lua_state();
    
    RG sandbox(Script::new_sandbox());
    RG table_fun(Script::load_lua_function("test/simple_table.lua", sandbox));
    
    Script::Helper::run_simple_function(table_fun, 1);
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
    
    Script::Helper::for_pairs(-1, [body, l]()->bool {
        body();
        return true;
    }, false);
    Script::Helper::for_pairs(-1, [body, l]()->bool {
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
            throw std::runtime_error(ss.str());
        }
        if (key_value.second != (*iter).second) {
            std::stringstream ss;
            ss << "Resulting table has wrong value for ";
            ss << key_value.first;
            ss << ": Expected: ";
            ss << key_value.second;
            ss << ": Got: ";
            ss << (*iter).second;
            throw std::runtime_error(ss.str());
        }
    }
}

//@Test Script Helper for_pairs with exception
void test_0028_for_pairs_exception() {
    lua_State* l = Script::get_lua_state();
    
    RG sandbox(Script::new_sandbox());
    RG table_fun(Script::load_lua_function("test/simple_table.lua", sandbox));
    
    Script::Helper::run_simple_function(table_fun, 1);
    Script::Pop_Guard pop_guard(1);
    
    try {
        Script::Helper::for_pairs(-1, []()->bool {
            throw std::runtime_error("orange juice");
        }, false);
    }
    catch (std::runtime_error e) {
        if (std::string(e.what()) != "orange juice") {
            throw e;
        }
        return;
    }
    throw std::runtime_error("Error did not bubble up!");
}
//@Test Script Helper to_string
void test_0028_to_string() {
    lua_State* l = Script::get_lua_state();
    int original_size = lua_gettop(l);
    
    RG sandbox(Script::new_sandbox());
    RG table_fun(
            Script::load_lua_function("test/complex_tostring.lua", sandbox));
            
    Script::Helper::run_simple_function(table_fun, 1);
    std::string resp = Script::Helper::to_string(-1);
    lua_pop(l, 1);
    
    if (resp != "bottom") {
        throw std::runtime_error("Did not reach the bottom of tostring");
    }
    
    if (original_size != lua_gettop(l)) {
        throw std::runtime_error("Unbalanced");
    }
}

//@Test Script Helper for_number_pairs_sorted
void test_0028_for_pairs_number_sorted() {
    lua_State* l = Script::get_lua_state();
    
    RG sandbox(Script::new_sandbox());
    RG table_fun(Script::load_lua_function("test/sparse_array.lua", sandbox));
    
    Script::Helper::run_simple_function(table_fun, 1);
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
    
    Script::Helper::for_number_pairs_sorted(-1, body, false);
    
    Logger::log()->info(expected);
    
    if (expected != got.str()) {
        std::stringstream sss;
        sss << "Expected shakespeare, got: \""
            << got.str()
            << '"';
        throw std::runtime_error(sss.str());
    }
    
    expected = "day?summer's a to thee compare I Shall ";
    got = std::stringstream();
    
    Script::Helper::for_number_pairs_sorted(-1, body, false, true);
    
    Logger::log()->info(expected);
    
    if (expected != got.str()) {
        std::stringstream sss;
        sss << "Expected shakespeare, got: \""
            << got.str()
            << '"';
        throw std::runtime_error(sss.str());
    }
}

} // namespace Test
} // namespace pegr
