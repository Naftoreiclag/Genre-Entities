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

#include "pegr/except/Except.hpp"

namespace pegr {
namespace Gensys {
namespace Runtime {
    
Entity* Entity_Collection::get_entity(Entity_Handle handle) {
    // Check special case
    if (handle == -1) return nullptr;
    
    // Try find the entity in the "official" set
    Entity* retval = get_entity_inside(handle,
            m_handle_to_index, m_vector);
            
    // Possible these entities exist, but are just deferred
    if (m_deferred_mode && !retval) {
        // Try find it
        retval = get_entity_inside(handle, 
                m_queued_handle_to_index, m_queued_vector);
    }
    
    return retval;
}

void Entity_Collection::clear() {
    m_next_handle = 0;
    m_vector.clear();
    // Very important: otherwise entity handles may wrongly report existence.
    m_handle_to_index.clear();
    
    m_deferred_mode = false;
    m_queued_vector.clear();
    m_queued_handle_to_index.clear();
    m_queued_removals.clear();
}

Entity_Handle Entity_Collection::emplace(Arche* arche) {
    if (m_deferred_mode) {
        return emplace_into(arche, m_queued_handle_to_index, m_queued_vector);
    } else {
        return emplace_into(arche, m_handle_to_index, m_vector);
    }
}

void Entity_Collection::remove(Entity_Handle handle_a) {
    /* In deferred mode, we can't remove an entity just yet, since we need to
     * preserve the ordering of the "actual" vector
     */
    if (m_deferred_mode) {
        m_queued_removals.insert(handle_a);
        return;
    }
    
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
    if (handle.get_id() == -1) {
        return false;
    }
    if (m_handle_to_index.find(handle) == m_handle_to_index.end()) {
        return false;
    }
    if (m_deferred_mode 
            && m_queued_removals.find(handle) != m_queued_removals.end()) {
        return false;
    }
    return true;
}

void Entity_Collection::for_each(std::function<void(Entity*)> for_body) {
    assert(!m_deferred_mode && "Cannot run for_each recursively.");
    
    enable_deferred();
    for (Entity& ent : m_vector) {
        try {
            for_body(&ent);
        } catch (Except::Runtime& e) {
            disable_deferred();
            throw;
        }
    }
    disable_deferred();
}
void Entity_Collection::enable_deferred() {
    assert(!m_deferred_mode);
    m_deferred_mode = true;
}
void Entity_Collection::disable_deferred() {
    assert(m_deferred_mode);
    
    // Must disable deferred mode right now in order to use the usual methods
    m_deferred_mode = false;
    
    for (std::uint64_t hand : m_queued_removals) {
        remove(Entity_Handle(hand));
    }
    
    std::size_t bottom = m_vector.size();
    if (bottom == 0) {
        assert(m_handle_to_index.size() == 0);
        
        m_vector = std::move(m_queued_vector);
        m_handle_to_index = std::move(m_queued_handle_to_index);
        
        m_queued_vector.clear();
        m_queued_handle_to_index.clear();
    } else {
        std::move(m_queued_vector.begin(), m_queued_vector.end(), 
                std::back_inserter(m_vector));
        m_queued_vector.clear();
        
        for (auto& queued_pair : m_queued_handle_to_index) {
            queued_pair.second += bottom;
        }
        
        m_handle_to_index.insert(m_queued_handle_to_index.begin(), 
                m_queued_handle_to_index.end());
        
        m_queued_handle_to_index.clear();
    }
}

Entity_Handle Entity_Collection::emplace_into(Arche* arche, 
            std::unordered_map<std::uint64_t, std::size_t>& hti,
            std::vector<Entity>& vec) {
    // Record what the entity's index in the vector will be
    std::size_t index = vec.size();
    
    Entity_Handle hand(m_next_handle);
    
    // Emplace-back a new entity
    vec.emplace_back(arche, hand);
    
    // Increase handle now, otherwise there could be issues with exceptions
    ++m_next_handle;
    
    // Get the entity we just created by reference
    Entity& ent = vec.back();
    
    // Map the entity handle to that index in the vector
    hti[hand] = index;
    
    // Return handle to newly created entity
    return hand;
}
Entity* Entity_Collection::get_entity_inside(Entity_Handle handle, 
        std::unordered_map<std::uint64_t, std::size_t>& hti,
        std::vector<Entity>& vec) {
    auto iter = hti.find(handle);
    if (iter == hti.end()) {
        return nullptr;
    }
    std::size_t idx = iter->second;
    assert(idx >= 0 && idx < vec.size());
    return &(vec[idx]);
}

} // namespace Runtime
} // namespace Gensys
} // namespace pegr
