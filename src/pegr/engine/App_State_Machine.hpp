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

#ifndef PEGR_ENGINE_APPSTATEMACHINE_HPP
#define PEGR_ENGINE_APPSTATEMACHINE_HPP

#include <memory>
#include <vector>

#include "pegr/engine/App_State.hpp"

namespace pegr {
namespace Engine {

class Dummy_App_State : public App_State {
public:
    Dummy_App_State();
    virtual ~Dummy_App_State();
};

class App_State_Machine {
public:
    App_State_Machine();
    ~App_State_Machine();

    /**
     * @brief Pushes a new state to the top of the stack, activating it
     */
    void push_state(std::unique_ptr<App_State>&& state);
    
    /**
     * @brief Removes the top state from the stack, returns it
     * @return The top state
     */
    std::unique_ptr<App_State> pop_state();
    
    /**
     * @brief Replaces the top app state with another. This differs from simply
     * popping and pushing a new state by not wastefully calling the pause and
     * unpause methods of the second-to-top state.
     * @return The state which previously held the top position
     */
    std::unique_ptr<App_State> swap_state(std::unique_ptr<App_State>&& state);
    
    App_State* get_active() const;
    App_State* operator ->() const;
    bool has_active() const;
    
    void clear_all();
    
    std::size_t count() const;

private:
    std::vector<std::unique_ptr<App_State> > m_state_pushdown;
    std::unique_ptr<App_State> m_dummy;
};

} // namespace Engine
} // namespace pegr

#endif // PEGR_ENGINE_APPSTATEMACHINE_HPP
