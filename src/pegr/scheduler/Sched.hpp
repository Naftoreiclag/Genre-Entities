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

#ifndef PEGR_SCHEDULER_SCHED_HPP
#define PEGR_SCHEDULER_SCHED_HPP

#include <string>
#include <cstdint>

#include "pegr/resource/Oid.hpp"
#include "pegr/script/Script.hpp"

namespace pegr {
namespace Schedu {

enum struct GlobalState {
    // Stage before initialize() is called for the first time
    UNINITIALIZED,
    
    // Stage where new elements can be added into the system
    MUTABLE,
    
    // Stage after "compiling," entities can be created and processed now
    EXECUTABLE,
    
    ENUM_SIZE /*Number of valid enum values*/
};

GlobalState get_global_state();

/**
 * @class Listener_Common
 * @brief Standard collection of 
 */
struct Listener_Common {
    int32_t m_delay;
    int32_t m_period;
    
    
    Script::Unique_Regref m_script_func;
};

/**
 * @class Event
 * @brief Superclass for all events. Subclasses of Event define their own
 * trigger logic and Listener type.
 */
class Event {
public:
    enum Type {
        ENTITY_TICK,
        SCRIPTED
    };
    
    Event();
    virtual ~Event();
    
    virtual Type get_type() const = 0;
    virtual void trigger() = 0;
};

void add_event(Resour::Oid oid, std::unique_ptr<Event>&& ev);
Event* find_event(Resour::Oid oid);

void initialize();
void compile();
void cleanup();

} // namespace Schedu
} // namespace pegr

#endif // PEGR_SCHEDULER_SCHED_HPP
