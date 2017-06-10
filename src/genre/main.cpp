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
    const luaL_Reg api_safe[] = {
        {"test", test},
        
        // End of the list
        {nullptr, nullptr}
    };
    const luaL_Reg api_restricted[] = {
        {"restricted", restricted},
        
        // End of the list
        {nullptr, nullptr}
    };
    Script::multi_expose_c_functions(api_safe);
    Script::multi_expose_c_functions(api_restricted, false);
    
    Script::run_function(Script::load_lua_function("malicious.lua", Script::new_sandbox()));
    Script::run_function(Script::load_lua_function("test.lua", Script::new_sandbox()));
    Script::run_function(Script::load_lua_function("elevated.lua", Script::NO_SANDBOX));
    
    Script::cleanup();
    return 0;
}
