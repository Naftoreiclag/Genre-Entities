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

#ifndef PEGR_GENSYS_EVENTS_HPP
#define PEGR_GENSYS_EVENTS_HPP

#include "pegr/gensys/Entity_Events.hpp"

namespace pegr {
namespace Gensys {
namespace Event {

Entity_Spawned_Event* get_entity_spawned_event();
Entity_Tick_Event* get_entity_tick_event();
Entity_Killed_Event* get_entity_killed_event();
    
void initialize();
void cleanup();

} // namespace Events
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_EVENTS_HPP
