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

namespace pegr {
namespace Gensys {
/*
namespace Runtime {

extern std::vector<Entity> n_entities;

} // namespace Runtime
*/

Entity_Tick_Event::Entity_Tick_Event()
: Schedu::Event() {}
Entity_Tick_Event::~Entity_Tick_Event() {}

Schedu::Event::Type Entity_Tick_Event::get_type() const {
    return Schedu::Event::Type::ENTITY_TICK;
}
void Entity_Tick_Event::trigger() {
    /*
    for (Runtime::Entity& ent : Runtime::n_entities) {
        for (Ete_Listener& listener : m_listeners) {
            switch (listener.m_selector) {
                case Ete_Listener::Selector_Type::COMP: {
                    Runtime::Comp* comp = listener.m_selector_union.m_comp;
                    Cview cview = comp->match()
                }
            }
        }
    }*/
}

} // namespace Gensys
} // namespace pegr
