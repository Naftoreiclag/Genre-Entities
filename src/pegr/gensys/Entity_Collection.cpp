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

#include "pegr/gensys/Entity_Collection.hpp"

#include <cassert>

namespace pegr {
namespace Gensys {
namespace Runtime {
    
Entity* Entity_Collection::get_entity(Entity_Handle handle) {
    if (handle == -1) return nullptr;
    auto iter = m_handle_to_index.find(handle);
    if (iter == m_handle_to_index.end()) {
        return nullptr;
    }
    std::size_t idx = iter->second;
    assert(idx >= 0 && idx < m_vector.size());
    return &(m_vector[idx]);
}

void Entity_Collection::clear() {
    m_next_handle = 0;
    m_vector.clear();
    // Very important: otherwise entity handles may wrongly report existence.
    m_handle_to_index.clear();
}

Entity_Handle Entity_Collection::emplace(Arche* arche) {
    // Record what the entity's index in the vector will be
    std::size_t index = m_vector.size();
    
    Entity_Handle hand(m_next_handle);
    
    // Emplace-back a new entity
    m_vector.emplace_back(arche, hand);
    
    // Increase handle now, otherwise there could be issues with exceptions
    ++m_next_handle;
    
    // Get the entity we just created by reference
    Entity& ent = m_vector.back();
    
    // Map the entity handle to that index in the vector
    m_handle_to_index[hand] = index;
    
    // Return handle to newly created entity
    return hand;
}

void Entity_Collection::remove(Entity_Handle handle_a) {
    /* Delete the entity given by the handle (entity "A") by swapping it with
     * the last entity in the vector (entity "B"), then popping off the last
     * entity (A). Must also update B's index (set B's index to A's old index).
     * 
     * The special case where A and B are the same entity is handled implicitly.
     */
    
    assert(m_vector.size() > 0);
    
    // Find the pair in the handle-to-index map
    auto map_entry_a = m_handle_to_index.find(handle_a);
    assert(map_entry_a != m_handle_to_index.end());
    
    // Get the index of the entity in the entity vector
    std::size_t index_a = map_entry_a->second;
    
    // Get the index of the last entity
    std::size_t index_b = m_vector.size() - 1;
    
    // Get the handle of the last entity
    Entity_Handle handle_b = m_vector[index_b].get_handle();
    
    // Find the pair in the handle-to-index map
    auto map_entry_b = m_handle_to_index.find(handle_b);
    assert(map_entry_b != m_handle_to_index.end());
    
    // Set the index
    map_entry_b->second = index_a;
    
    // Swap this entity with the back and remove
    std::iter_swap(m_vector.begin() + index_a, m_vector.begin() + index_b);
    m_vector.pop_back();
    assert(m_vector.size() == index_b);
    assert(m_vector.size() == 0 || 
            m_handle_to_index[m_vector[index_a].get_handle()] == index_a);
    
    // Remove the corresponding entry from the handle-to-index map
    m_handle_to_index.erase(map_entry_a);
    assert(m_handle_to_index.find(handle_a) == m_handle_to_index.end());
}

bool Entity_Collection::is_valid(Entity_Handle handle) {
    return handle.get_id() != -1
            && (m_handle_to_index.find(handle) != m_handle_to_index.end());
}

void Entity_Collection::for_each(std::function<void(Entity*)> for_body) {
    
}

} // namespace Runtime
} // namespace Gensys
} // namespace pegr
