#include <cassert>
#include <stdexcept>

#include "pegr/gensys/Gensys.hpp"
#include "pegr/gensys/GensysLuaInterface.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/script/ScriptHelper.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/debug/DebugMacros.hpp"

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
        {"add_archetype", Gensys::LI::add_archetype},
        {"edit_archetype", Gensys::LI::edit_archetype},
        {"add_genre", Gensys::LI::add_genre},
        {"add_component", Gensys::LI::add_component},
        {"edit_component", Gensys::LI::edit_component},
        
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
            Script::load_lua_function("init.lua", sandbox));
    Script::Regref_Guard postinit_fun(
            Script::load_lua_function("postinit.lua", sandbox));
    
    try {
        Script::Helper::run_simple_function(init_fun, 0);
    } catch (std::runtime_error e) {
        Logger::log()->warn(e.what());
    }
    Gensys::compile();
    try {
        Script::Helper::run_simple_function(postinit_fun, 0);
    } catch (std::runtime_error e) {
        Logger::log()->warn(e.what());
    }
}

void cleanup() {
    Gensys::cleanup();
    Script::cleanup();
    Logger::cleanup();
}

int main() {
    setup();
    run();
    cleanup();
    return 0;
}
