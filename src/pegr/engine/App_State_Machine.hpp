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

class App_State_Machine {
public:
    ~App_State_Machine();

    void push_state(std::unique_ptr<App_State>&& state);
    
    std::unique_ptr<App_State> pop_state();
    std::unique_ptr<App_State> swap_state(std::unique_ptr<App_State>&& state);
    
    App_State* get_active();
    
    void clear_all();

private:
    std::vector<std::unique_ptr<App_State> > m_state_pushdown;
};

} // namespace Engine
} // namespace pegr

#endif // PEGR_ENGINE_APPSTATEMACHINE_HPP
