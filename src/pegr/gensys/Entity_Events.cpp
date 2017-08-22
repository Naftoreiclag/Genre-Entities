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

#include "pegr/gensys/Entity_Events.hpp"

#include "pegr/gensys/Runtime.hpp"

namespace pegr {
namespace Gensys {
namespace Event {

Entity_Listener::Entity_Listener(
        std::function<void(Runtime::Entity*)> func)
: m_func(func) {}

void Entity_Listener::call(Runtime::Entity* ent) {
    m_func(ent);
}

Entity_Tick_Event::Entity_Tick_Event()
: Schedu::Event() {}
Entity_Tick_Event::~Entity_Tick_Event() {}

void Entity_Tick_Event::add_listener(Arche_Entity_Listener listener) {
    m_arche_listeners.push_back(listener);
}
void Entity_Tick_Event::add_listener(Comp_Entity_Listener listener) {
    m_comp_listeners.push_back(listener);
}
void Entity_Tick_Event::add_listener(Genre_Entity_Listener listener) {
    m_genre_listeners.push_back(listener);
}

Schedu::Event::Type Entity_Tick_Event::get_type() const {
    return Schedu::Event::Type::ENTITY_TICK;
}

void Entity_Tick_Event::trigger() {
    for (Arche_Entity_Listener& listener : 
            m_arche_listeners) {
        Runtime::get_entities().for_each([&](Runtime::Entity* ent) {
            listener.call(ent);
        });
    }
    for (Comp_Entity_Listener& listener : 
            m_comp_listeners) {
        Runtime::get_entities().for_each([&](Runtime::Entity* ent) {
            listener.call(ent);
        });
    }
    for (Genre_Entity_Listener& listener : 
            m_genre_listeners) {
        Runtime::get_entities().for_each([&](Runtime::Entity* ent) {
            listener.call(ent);
        });
    }
}

} // namespace Event
} // namespace Gensys
} // namespace pegr
