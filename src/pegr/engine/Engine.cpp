/*
 *  Copyright 2017 James Fong
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

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
#include "pegr/engine/App_State_Machine.hpp"

namespace pegr {
namespace Engine {

const uint16_t INIT_FLAG_LOGGER = 0x0001;
const uint16_t INIT_FLAG_SCRIPT = 0x0002 | INIT_FLAG_LOGGER;
const uint16_t INIT_FLAG_GENSYS = 0x0004 | INIT_FLAG_SCRIPT;
const uint16_t INIT_FLAG_SCHEDU = 0x0008 | INIT_FLAG_SCRIPT;
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
bool schedu_used() {
    return (n_flags & INIT_FLAG_SCHEDU) == INIT_FLAG_SCHEDU;
}
bool winput_used() {
    return (n_flags & INIT_FLAG_WINPUT) == INIT_FLAG_WINPUT;
}

bool n_main_loop_running = false;

App_State_Machine n_app_state_machine;

void initialize(uint16_t flags) {
    n_flags = flags;
    
    if (logger_used()) {
        Logger::initialize();
        Logger::log()->info("Logger: %v", logger_used());
        Logger::log()->info("Script: %v", script_used());
        Logger::log()->info("Gensys: %v", gensys_used());
        Logger::log()->info("Scheduler: %v", schedu_used());
        Logger::log()->info("Window/Input: %v", winput_used());
    }
    
    if (script_used()) {
        Script::initialize();
    }
    
    if (gensys_used()) {
        Gensys::initialize();
        Gensys::LI::initialize();
    }
    
    if (schedu_used()) {
        Schedu::LI::initialize();
    }
    
    if (winput_used()) {
        Winput::initialize();
    }
}

void push_state(std::unique_ptr<App_State>&& unique_state) {
    n_app_state_machine.push_state(std::move(unique_state));
}

std::unique_ptr<App_State> pop_state() {
    return n_app_state_machine.pop_state();
}

std::unique_ptr<App_State> swap_state(std::unique_ptr<App_State>&& u_state) {
    return n_app_state_machine.swap_state(std::move(u_state));
}

void on_window_resize(int32_t width, int32_t height) {
    App_State* app_state = n_app_state_machine.get_active();
    if (!app_state) return;
    app_state->on_window_resize(width, height);
}

void run() {
    n_main_loop_running = true;
    while (n_main_loop_running) {
        App_State* app_state = n_app_state_machine.get_active();
        if (!app_state) {
            quit();
            break;
        }
        if (winput_used()) {
            Winput::pollEvents();
        }
        app_state->on_frame();
        if (!app_state) {
            quit();
            break;
        }
    }
}

void cleanup() {
    n_app_state_machine.clear_all();
    
    if (winput_used()) {
        Winput::cleanup();
    }
    
    if (schedu_used()) {
        Schedu::LI::cleanup();
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
