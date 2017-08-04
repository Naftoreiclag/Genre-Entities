#include "pegr/engine/Engine.hpp"

#include <cassert>
#include <stdexcept>

#include "pegr/gensys/Lua_Interf.hpp"
#include "pegr/gensys/Gensys.hpp"
#include "pegr/scheduler/Lua_Interf.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/winput/Winput.hpp"

#include "pegr/script/Script_Helper.hpp"
#include "pegr/logger/Logger.hpp"

namespace pegr {
namespace Engine {

bool n_main_loop_running = false;
    
void initialize() {
    Logger::initialize();
    
    Script::initialize();
    
    Gensys::LI::initialize();
    Sched::LI::initialize();
    
    Gensys::initialize();
    
    Winput::initialize();
}

void run() {
    Script::Unique_Regref sandbox(Script::new_sandbox());
    Script::Unique_Regref init_fun;
    Script::Unique_Regref postinit_fun;
    try {
        init_fun = Script::load_lua_function("init.lua", sandbox);
        postinit_fun = Script::load_lua_function("postinit.lua", sandbox);
    } catch (std::runtime_error e) {
        Logger::log()->warn(e.what());
    }
    
    try {
        Script::Helper::run_simple_function(init_fun, 0);
    } catch (std::runtime_error e) {
        Logger::log()->warn(e.what());
    }
    Gensys::LI::stage_all();
    Gensys::compile();
    try {
        Script::Helper::run_simple_function(postinit_fun, 0);
    } catch (std::runtime_error e) {
        Logger::log()->warn(e.what());
    }
    
    n_main_loop_running = true;
    while (n_main_loop_running) {
        Winput::pollEvents();
    }
}

void cleanup() {
    Winput::cleanup();
    
    Gensys::cleanup();
    
    Sched::LI::cleanup();
    Gensys::LI::cleanup();
    
    Script::cleanup();
    
    Logger::cleanup();
}

void quit() {
    n_main_loop_running = false;
}

} // namespace Engine
} // namespace pegr
