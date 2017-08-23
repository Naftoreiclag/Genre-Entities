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

#include "pegr/gensys/Events.hpp"

#include "pegr/engine/Engine.hpp"

namespace pegr {
namespace Gensys {
namespace Event {
    
Entity_Spawned_Event* n_spawned;
Entity_Tick_Event* n_tick;
Entity_Killed_Event* n_killed;

Entity_Spawned_Event* get_entity_spawned_event() {
    return n_spawned;
}
Entity_Tick_Event* get_entity_tick_event() {
    return n_tick;
}
Entity_Killed_Event* get_entity_killed_event() {
    return n_killed;
}

template<typename Event_T>
Event_T* reg_event(const char* name) {
    return static_cast<Event_T*>(Schedu::add_event(name, 
                std::make_unique<Event_T>()));
}

void initialize() {
    n_spawned = reg_event<Entity_Spawned_Event>("entity_spawned.ev");
    n_tick = reg_event<Entity_Tick_Event>("entity_tick.ev");
    n_killed = reg_event<Entity_Killed_Event>("entity_killed.ev");
}

void cleanup() {
    Entity_Spawned_Event* n_spawned = nullptr;
    Entity_Tick_Event* n_tick = nullptr;
    Entity_Killed_Event* n_killed = nullptr;
}

} // namespace Events
} // namespace Gensys
} // namespace pegr
