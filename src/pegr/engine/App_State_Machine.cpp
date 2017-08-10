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

#include "pegr/engine/App_State_Machine.hpp"

#include <algorithm>
#include <vector>

namespace pegr {
namespace Engine {
    
App_State_Machine::~App_State_Machine() {
    clear_all();
}

void App_State_Machine::push_state(std::unique_ptr<App_State>&& unique_state) {
    App_State* state = unique_state.get();
    if (m_state_pushdown.size() > 0) {
        App_State* back_state = m_state_pushdown.back().get();
        back_state->pause(state);
    }
    m_state_pushdown.emplace_back(std::move(unique_state));
    state->initialize();
}

std::unique_ptr<App_State> App_State_Machine::pop_state() {
    if (m_state_pushdown.size() == 0) {
        std::unique_ptr<App_State> none;
        return none;
    }
    std::unique_ptr<App_State>& unique_back_state = m_state_pushdown.back();
    App_State* affected_state = unique_back_state.get();
    affected_state->cleanup();
    std::unique_ptr<App_State> retval(std::move(unique_back_state));
    m_state_pushdown.pop_back();
    if (m_state_pushdown.size() > 0) {
        App_State* new_back_state = m_state_pushdown.back().get();
        new_back_state->unpause(affected_state);
    }
    return retval;
}

std::unique_ptr<App_State> App_State_Machine::swap_state(
        std::unique_ptr<App_State>&& u_state) {
    if (m_state_pushdown.size() == 0) {
        push_state(std::move(u_state));
        std::unique_ptr<App_State> none;
        return none;
    }
    
    App_State* state = u_state.get();
    std::unique_ptr<App_State>& u_top_state = m_state_pushdown.back();
    App_State* top_state = u_top_state.get();
    top_state->cleanup();
    std::unique_ptr<App_State> retval(std::move(u_top_state));
    u_top_state = std::move(u_state);
    state->initialize();
    return retval;
}

void App_State_Machine::clear_all() {
    while (m_state_pushdown.size() > 0) {
        pop_state();
    }
}

App_State* App_State_Machine::get_active() {
    if (m_state_pushdown.size() == 0) {
        return nullptr;
    }
    return m_state_pushdown.back().get();
}

} // namespace Engine
} // namespace pegr
