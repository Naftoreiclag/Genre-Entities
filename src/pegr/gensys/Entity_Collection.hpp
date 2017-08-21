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

#ifndef PEGR_GENSYS_ENTITYCOLLECTION_HPP
#define PEGR_GENSYS_ENTITYCOLLECTION_HPP

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "pegr/gensys/Runtime_Types.hpp"

namespace pegr {
namespace Gensys {
namespace Runtime {

class Entity_Collection {
public:
    
    Entity* get_entity(Entity_Handle handle);
    void clear();
    
    /**
     * @brief Create a new entity: "nonexistent" -> "can_be_spawned"
     * @param arche The archetype to use
     * @return the entity
     */
    Entity_Handle new_entity(Arche* arche);
    
    /**
     * @brief Delete the entity: "killed" -> "nonexistent"
     *                              or "can_be_spawned" -> "nonexistent"
     * Note: if the entity is in the "alive" state, this automatically calls
     * kill_entity() first.
     * @param handle The handle of the entity
     */
    void delete_entity(Entity_Handle handle);

    bool does_exist(Entity_Handle handle);
    void for_each(std::function<void(Entity*)> for_body);
    
private:

    std::uint64_t m_next_handle = 0;
    std::unordered_map<std::uint64_t, std::size_t> m_handle_to_index;
    std::vector<Entity> m_vector;
    
    /* Deferred mode is used during a call to for_each()
     * When active, removals and additions are queued. From the user's
     * perspective, however, these removals and additions really do take place.
     * 
     * Internally, no modifications to the length of m_vector are allowed
     * when deferred mode is active.
     */
    bool m_deferred_mode = false;
    std::unordered_set<std::uint64_t> m_queued_removals;
    std::unordered_map<std::uint64_t, std::size_t> m_queued_handle_to_index;
    std::vector<Entity> m_queued_vector;
    
    void enable_deferred();
    void disable_deferred();
    
    Entity_Handle emplace_into(Arche* arche, 
            std::unordered_map<std::uint64_t, std::size_t>& hti,
            std::vector<Entity>& vec);
    
    Entity* get_entity_inside(Entity_Handle handle, 
            std::unordered_map<std::uint64_t, std::size_t>& hti,
            std::vector<Entity>& vec);
};
    
} // namespace Runtime
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_ENTITYCOLLECTION_HPP
