#include <cstddef>
#include <map>

#include "pegr/script/ScriptHelper.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"

namespace pegr {
namespace Test {

typedef Script::Regref_Guard RG;
    
//@Test Script Helper
bool script_helper() {
    RG sandbox(Script::new_sandbox());
    RG table_fun(Script::load_lua_function("test/simple_table.lua", sandbox));
    
    Script::run_function(table_fun, 0, 1);
    
    lua_State* l = Script::get_lua_state();
    
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
    
    return expected == got;
}

}
}
