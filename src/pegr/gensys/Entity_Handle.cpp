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

#include "pegr/gensys/Entity_Handle.hpp"

#include "pegr/gensys/Entity_Collection.hpp"

namespace pegr {
namespace Gensys {
namespace Runtime {
    
extern Entity_Collection n_ent_collection;

Entity_Handle::Entity_Handle(uint64_t id)
: m_entity_id(id) {}

Entity_Handle::Entity_Handle()
: m_entity_id(-1) {}

uint64_t Entity_Handle::get_id() const {
    return m_entity_id;
}
bool Entity_Handle::does_exist() const {
    return n_ent_collection.does_exist(*this);
}
Entity* Entity_Handle::operator ->() const {
    return get_entity();
}

Entity_Handle::operator bool() const {
    return does_exist();
}
Entity_Handle::operator uint64_t() const {
    return get_id();
}
Entity* Entity_Handle::get_volatile_entity_ptr() const {
    return get_entity();
}

Entity* Entity_Handle::get_entity() const {
    return n_ent_collection.get_entity(*this);
}

} // namespace Runtime
} // namespace Gensys
} // namespace pegr
