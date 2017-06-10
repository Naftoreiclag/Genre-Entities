#include <iostream>

#include "Gensys.hpp"
#include "Script.hpp"

using namespace pegr;

int test(lua_State* l) {
    std::cout << "Hello from C++" << std::endl;
    return 0;
}

int restricted(lua_State* l) {
    std::cout << "You can't use this function" << std::endl;
    return 0;
}

int main() {
    Script::initialize();
    const luaL_Reg safe_api[] = {
        {"test", test}
    };
    Script::multi_expose_c_functions(safe_api, 
            sizeof(safe_api) / sizeof(luaL_Reg));
    const luaL_Reg restricted_api[] = {
        {"restricted", restricted}
    };
    Script::multi_expose_c_functions(restricted_api, 
            sizeof(restricted_api) / sizeof(luaL_Reg), false);
    
    Script::run_function(Script::load_lua_function("malicious.lua", Script::new_sandbox()));
    Script::run_function(Script::load_lua_function("test.lua", Script::new_sandbox()));
    Script::run_function(Script::load_lua_function("elevated.lua", Script::NO_SANDBOX));
    
    Script::cleanup();
    return 0;
}
