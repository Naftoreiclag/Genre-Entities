#include <cstddef>

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
    
    bool success = true;
    auto body = [l, &success]() {
        std::size_t strlen;
        const char* strdata;
        
        lua_pushvalue(l, -2);
        lua_pushvalue(l, -2);
        strdata = lua_tolstring(l, -2, &strlen);
        std::string key(strdata, strlen);
        strdata = lua_tolstring(l, -1, &strlen);
        std::string val(strdata, strlen);
        
        if (key == "a") {
            if (val != "apple") {
                success = false;
            }
        } else if (key == "b") {
            if (val != "banana") {
                success = false;
            }
        } else if (key == "c") {
            if (val != "cherry") {
                success = false;
            }
        }
        
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
    
    return success;
}

}
}
