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

#include "pegr/scheduler/Sched.hpp"

#include <cassert>
#include <map>
#include <sstream>

#include "pegr/except/Except.hpp"

namespace pegr {
namespace Schedu {

GlobalState m_global_state = GlobalState::UNINITIALIZED;

std::map<Resour::Oid, std::unique_ptr<Event> > n_events;

GlobalState get_global_state() {
    return m_global_state;
}

Event::Event() {}
Event::~Event() {}

Event* find_event(Resour::Oid oid) {
    auto iter = n_events.find(oid);
    if (iter == n_events.end()) {
        std::stringstream sss;
        sss << "Unable to find event, "
            << oid.get_dbg_string();
        throw Except::Runtime(sss.str());
    }
    
    return iter->second.get();
}

Event* add_event(Resour::Oid oid, std::unique_ptr<Event>&& ev) {
    Event* ptr = ev.get();
    n_events[oid] = std::move(ev);
    return ptr;
}

void initialize() {
    assert(m_global_state == GlobalState::UNINITIALIZED);
    m_global_state = GlobalState::MUTABLE;
}
void compile() {
    assert(m_global_state == GlobalState::MUTABLE);
    m_global_state = GlobalState::EXECUTABLE;
}
void cleanup() {
    assert(m_global_state != GlobalState::UNINITIALIZED);
    n_events.clear();
    m_global_state = GlobalState::UNINITIALIZED;
}

} // namespace Schedu
} // namespace pegr
