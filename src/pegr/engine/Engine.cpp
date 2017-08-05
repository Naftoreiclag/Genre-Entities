#include "pegr/engine/Engine.hpp"

#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <vector>

#include "pegr/gensys/Lua_Interf.hpp"
#include "pegr/gensys/Gensys.hpp"
#include "pegr/scheduler/Lua_Interf.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/winput/Winput.hpp"

namespace pegr {
namespace Engine {

const uint16_t INIT_FLAG_LOGGER = 0x0001;
const uint16_t INIT_FLAG_SCRIPT = 0x0002 | INIT_FLAG_LOGGER;
const uint16_t INIT_FLAG_GENSYS = 0x0004 | INIT_FLAG_SCRIPT;
const uint16_t INIT_FLAG_SCHED = 0x0008 | INIT_FLAG_SCRIPT;
const uint16_t INIT_FLAG_WINPUT = 0x0010 | INIT_FLAG_LOGGER;
const uint16_t INIT_FLAG_ALL = 0xFFFF;
const uint16_t INIT_FLAG_NONE = 0x0000;

uint16_t n_flags;

bool logger_used() {
    return (n_flags & INIT_FLAG_LOGGER) == INIT_FLAG_LOGGER;
}
bool script_used() {
    return (n_flags & INIT_FLAG_SCRIPT) == INIT_FLAG_SCRIPT;
}
bool gensys_used() {
    return (n_flags & INIT_FLAG_GENSYS) == INIT_FLAG_GENSYS;
}
bool sched_used() {
    return (n_flags & INIT_FLAG_SCHED) == INIT_FLAG_SCHED;
}
bool winput_used() {
    return (n_flags & INIT_FLAG_WINPUT) == INIT_FLAG_WINPUT;
}

bool n_main_loop_running = false;

typedef std::vector<std::unique_ptr<App_State> > App_State_Vector;
App_State_Vector n_state_pushdown;

void initialize(uint16_t flags) {
    n_flags = flags;
    
    if (logger_used()) {
        Logger::initialize();
        Logger::log()->info("Logger: %v", logger_used());
        Logger::log()->info("Script: %v", script_used());
        Logger::log()->info("Gensys: %v", gensys_used());
        Logger::log()->info("Scheduler: %v", sched_used());
        Logger::log()->info("Window/Input: %v", winput_used());
    }
    
    if (script_used()) {
        Script::initialize();
    }
    
    if (gensys_used()) {
        Gensys::initialize();
        Gensys::LI::initialize();
    }
    
    if (sched_used()) {
        Sched::LI::initialize();
    }
    
    if (winput_used()) {
        Winput::initialize();
    }
}

void push_state(std::unique_ptr<App_State>&& unique_state) {
    App_State* state = unique_state.get();
    if (n_state_pushdown.size() > 0) {
        App_State* back_state = n_state_pushdown.back().get();
        back_state->pause(state);
    }
    n_state_pushdown.emplace_back(std::move(unique_state));
    state->initialize();
}

std::unique_ptr<App_State> pop_state() {
    if (n_state_pushdown.size() == 0) {
        std::unique_ptr<App_State> none;
        return none;
    }
    std::unique_ptr<App_State>& unique_back_state = n_state_pushdown.back();
    App_State* affected_state = unique_back_state.get();
    affected_state->cleanup();
    std::unique_ptr<App_State> retval;
    std::swap(unique_back_state, retval);
    n_state_pushdown.pop_back();
    if (n_state_pushdown.size() > 0) {
        App_State* new_back_state = n_state_pushdown.back().get();
        new_back_state->unpause(affected_state);
    }
    return retval;
}

void run() {
    n_main_loop_running = true;
    while (n_main_loop_running) {
        if (n_state_pushdown.size() == 0) {
            quit();
            break;
        }
        if (winput_used()) {
            Winput::pollEvents();
        }
        App_State* back_state = n_state_pushdown.back().get();
        back_state->on_frame();
        if (n_state_pushdown.size() == 0) {
            quit();
            break;
        }
    }
}

void cleanup() {
    while (n_state_pushdown.size() > 0) {
        pop_state();
    }
    
    if (winput_used()) {
        Winput::cleanup();
    }
    
    if (sched_used()) {
        Sched::LI::cleanup();
    }
    
    if (gensys_used()) {
        Gensys::LI::cleanup();
        Gensys::cleanup();
    }
    
    if (script_used()) {
        Script::cleanup();
    }
    
    if (logger_used()) {
        Logger::cleanup();
    }
}

void quit() {
    n_main_loop_running = false;
}

} // namespace Engine
} // namespace pegr
