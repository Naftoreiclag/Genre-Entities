#include <cstddef>
#include <sstream>
#include <map>

#include "pegr/script/ScriptHelper.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"

namespace pegr {
namespace Test {

typedef Script::Regref_Guard RG;
    
//@Test Script Helper for_pairs
void test_0028_for_pairs() {
    lua_State* l = Script::get_lua_state();
    int original_size = lua_gettop(l);
    
    RG sandbox(Script::new_sandbox());
    RG table_fun(Script::load_lua_function("test/simple_table.lua", sandbox));
    
    Script::Helper::run_simple_function(table_fun, 1);
    
    std::map<std::string, std::string> expected = {
        {"a", "apple"},
        {"b", "banana"},
        {"c", "cherry"}
    };
    
    std::map<std::string, std::string> got;
    
    bool success = true;
    auto body = [l, &got, &expected]() {
        std::size_t strlen;
        const char* strdata;
        
        lua_pushvalue(l, -2);
        lua_pushvalue(l, -2);
        strdata = lua_tolstring(l, -2, &strlen);
        std::string key(strdata, strlen);
        strdata = lua_tolstring(l, -1, &strlen);
        std::string val(strdata, strlen);
        
        got[key] = val;
        
        Logger::log()->verbose(1, "%v\t%v", key, val);
        
        lua_pop(l, 2);
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
    lua_pop(l, 1);
    
    if (original_size != lua_gettop(l)) {
        throw std::runtime_error("Unbalanced");
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

} // namespace Test
} // namespace pegr
