#include <cassert>
#include <stdexcept>

#include "pegr/gensys/Compiler.hpp"
#include "pegr/gensys/Lua_Interf.hpp"
#include "pegr/gensys/Gensys.hpp"
#include "pegr/scheduler/SchedLuaInterface.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/script/ScriptHelper.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/debug/DebugMacros.hpp"

using namespace pegr;

void setup() {
    Logger::initialize();
    Script::initialize();
    Gensys::initialize();
    Gensys::LI::initialize();
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
    Gensys::LI::stage_all();
    Gensys::Compiler::compile();
    try {
        Script::Helper::run_simple_function(postinit_fun, 0);
    } catch (std::runtime_error e) {
        Logger::log()->warn(e.what());
    }
}

void cleanup() {
    Gensys::LI::cleanup();
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
