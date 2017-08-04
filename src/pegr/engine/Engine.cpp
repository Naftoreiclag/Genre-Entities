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

bool n_main_loop_running = false;

typedef std::vector<std::unique_ptr<App_State> > App_State_Vector;
App_State_Vector n_state_pushdown;

void initialize() {
    Logger::initialize();
    
    Script::initialize();
    
    Gensys::LI::initialize();
    Sched::LI::initialize();
    
    Gensys::initialize();
    
    Winput::initialize();
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
        Winput::pollEvents();
        if (n_state_pushdown.size() > 0) {
            App_State* back_state = n_state_pushdown.back().get();
            back_state->on_frame();
        }
    }
}

void cleanup() {
    while (n_state_pushdown.size() > 0) {
        pop_state();
    }
    
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
