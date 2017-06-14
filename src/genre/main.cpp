#include <iostream>

#include "Gensys.hpp"
#include "Script.hpp"
#include "Logger.hpp"

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
    Logger::initialize();
    Script::initialize();
    Gensys::initialize();
    
    const luaL_Reg api_safe[] = {
        {"test", test},
        {"add_archetype", Gensys::li_add_archetype},
        {"edit_archetype", Gensys::li_edit_archetype},
        {"add_genre", Gensys::li_add_genre},
        {"edit_genre", Gensys::li_edit_genre},
        {"add_component", Gensys::li_add_component},
        {"edit_component", Gensys::li_edit_component},
        
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
    {
        Script::Regref_Guard sandbox(Script::new_sandbox());
        Script::Regref_Guard init_fun(
                Script::load_lua_function("init.lua", sandbox.regref()));
        Script::Regref_Guard postinit_fun(
                Script::load_lua_function("postinit.lua", sandbox.regref()));
        
        Script::run_function(init_fun.regref());
        Gensys::parse_all();
        Script::run_function(postinit_fun.regref());
        
    }
    Script::cleanup();
    return 0;
}
