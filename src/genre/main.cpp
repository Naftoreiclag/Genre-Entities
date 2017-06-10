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
        {"add_component", Gensys::li_add_component},
        
        // End of the list
        {nullptr, nullptr}
    };
    Script::multi_expose_c_functions(api_safe);
    const luaL_Reg api_restricted[] = {
        {"restricted", restricted},
        
        // End of the list
        {nullptr, nullptr}
    };
    Script::multi_expose_c_functions(api_restricted, false);
    
    Script::run_function(Script::load_lua_function("test.lua", Script::new_sandbox()));
    
    Script::cleanup();
    return 0;
}
