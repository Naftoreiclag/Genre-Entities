#include "pegr/gensys/Gensys.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"

using namespace pegr;

void setup_logger() {
    Logger::initialize();
}

void setup_scripting() {
    Script::initialize();
}

void setup_gensys() {
    Gensys::initialize();
    const luaL_Reg api_safe[] = {
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
}

void setup() {
    setup_logger();
    setup_scripting();
    setup_gensys();
}

void run() {
    Script::Regref_Guard sandbox(Script::new_sandbox());
    Script::Regref_Guard init_fun(
            Script::load_lua_function("init.lua", sandbox.regref()));
    Script::Regref_Guard postinit_fun(
            Script::load_lua_function("postinit.lua", sandbox.regref()));
    
    Script::run_function(init_fun.regref());
    Gensys::compile();
    Script::run_function(postinit_fun.regref());
    
}

void cleanup() {
    Script::cleanup();
}

int main() {
    setup();
    run();
    cleanup();
    return 0;
}
