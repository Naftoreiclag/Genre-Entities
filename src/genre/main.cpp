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
    Gensys::initialize();
    
    const luaL_Reg api_safe[] = {
        {"test", test},
        {"add_archetype", Gensys::li_add_archetype},
        {"find_archetype", Gensys::li_get_archetype},
        {"add_genre", Gensys::li_add_genre},
        {"find_genre", Gensys::li_get_genre},
        {"add_component", Gensys::li_add_component},
        {"find_component", Gensys::li_get_component},
        
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
