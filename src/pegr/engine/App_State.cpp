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

#include "pegr/engine/App_State.hpp"

namespace pegr {
namespace Engine {

App_State::App_State(const char* dbg_name)
: m_dbg_name(dbg_name) {}
App_State::~App_State() {}

void App_State::initialize() {}

void App_State::pause(App_State* to) {}
void App_State::unpause(App_State* from) {}

void App_State::on_frame() {}
void App_State::on_window_resize(int32_t width, int32_t height) {}

void App_State::cleanup() {}

const char* App_State::get_dbg_name() {
    return m_dbg_name;
}
    
} // namespace pegr
} // namespace Engine

