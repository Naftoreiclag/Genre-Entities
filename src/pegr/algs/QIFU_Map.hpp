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

#ifndef PEGR_ALGS_QIFUMAP_HPP
#define PEGR_ALGS_QIFUMAP_HPP

#include <cassert>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "pegr/except/Except.hpp"

namespace pegr {
namespace Algs {

/**
 * @class QIFU_Map
 * @brief Quick iteration, finite usage map.
 * 
 * Handle_T must be a number type:
 *      Integers must be assignable to it.
 *      It must be incrementable.
 */
template <typename Handle_T, typename Value_T>
class QIFU_Map {
public:
    static const Handle_T EMPTY_HANDLE = -1;
    
    /**
     * @brief Retrieves an element via handle, or nullptr if it was not found.
     * @param handle The handle of the element, as returned by add or emplace.
     * @return Pointer to the element or nullptr
     */
    Value_T* find(Handle_T handle) {
        // Check special case
        if (handle == EMPTY_HANDLE) return nullptr;
        
        // Try find the entity in the "official" set
        Value_T* retval = find_inside(handle,
                m_handle_to_index, m_vector);
                
        // Possible these entities exist, but are just deferred
        if (m_deferred_mode && !retval) {
            // Try find it
            retval = find_inside(handle, 
                    m_queued_handle_to_index, m_queued_vector);
        }
        
        return retval;
    }
    
    /**
     * @brief Add an element to this collection.
     * @param elem The element to add
     * @return A handle (NOT an index) for the collection.
     */
    template<typename Value_U>
    Handle_T add(Value_U&& elem) {
        if (m_deferred_mode) {
            return add_into(std::forward<Value_U>(elem), 
                    m_queued_handle_to_index, m_queued_vector);
        } else {
            return add_into(std::forward<Value_U>(elem), 
                    m_handle_to_index, m_vector);
        }
    }
    
    /**
     * @brief Remove an element by its handle. Does nothing if the handle does
     * not point to an element.
     * 
     * @param handle The handle of the element to remove
     * @return True if an element was actually removed (handle was valid)
     */
    void remove(Handle_T handle) {
        if (m_deferred_mode) {
            if (!remove_from(handle, 
                    m_queued_handle_to_index, m_queued_vector)) {
                m_queued_removals.insert(handle);
            }
        } else {
            remove_from(handle, m_handle_to_index, m_vector);
        }
        assert(!find(handle));
    }
    
    void clear() {
        assert(!m_deferred_mode);
        
        m_next_handle = 0;
        m_vector.clear();
        m_handle_to_index.clear();
    }
    
    /**
     * @brief Execute a function for every element, in some unspecified order. 
     * Concurrent modification is allowed (adding/removing new elements) by 
     * deferring removal and addition until after all elements that were a part 
     * of this collection before for_each was called have been iterated over. If 
     * any elements are added during an iteration, those elements are then 
     * iterated over seamlessly. This process repeats until no new elements are 
     * added during an iteration.
     * @param for_body The function to run on every entity, as described above.
     */
    void for_each(std::function<void(Value_T*, Handle_T)> for_body) {
        assert(!m_deferred_mode && "Cannot run for_each recursively.");
        
        enable_deferred();
        for (Hanval_Pair& pair : m_vector) {
            try {
                for_body(&(pair.m_value), pair.m_handle);
            } catch (Except::Runtime& e) {
                disable_deferred();
                throw;
            }
        }
        disable_deferred();
    }
    void for_each(std::function<void(Value_T*)> for_body) {
        assert(!m_deferred_mode && "Cannot run for_each recursively.");
        
        enable_deferred();
        for (Hanval_Pair& pair : m_vector) {
            try {
                for_body(&(pair.m_value));
            } catch (Except::Runtime& e) {
                disable_deferred();
                throw;
            }
        }
        disable_deferred();
    }
    
private:

    struct Hanval_Pair {
        template<typename Value_U>
        Hanval_Pair(Handle_T handle, Value_U&& value)
        : m_handle(handle)
        , m_value(value) {}
        
        Handle_T m_handle;
        Value_T m_value;
    };

    Handle_T m_next_handle = 0;
    std::unordered_map<Handle_T, std::size_t> m_handle_to_index;
    std::vector<Hanval_Pair> m_vector;
    
    /* Deferred mode is used during a call to for_each()
     * When active, removals and additions are queued. From the user's
     * perspective, however, these removals and additions really do take place.
     * 
     * Internally, no modifications to the length of m_vector are allowed
     * when deferred mode is active.
     */
    bool m_deferred_mode = false;
    std::unordered_set<Handle_T> m_queued_removals;
    std::unordered_map<Handle_T, std::size_t> m_queued_handle_to_index;
    std::vector<Hanval_Pair> m_queued_vector;
    
    void enable_deferred() {
        assert(!m_deferred_mode);
        m_deferred_mode = true;
    }
    void disable_deferred() {
        assert(m_deferred_mode);
        
        // Must disable deferred mode right now to use the usual methods
        m_deferred_mode = false;
        
        for (Handle_T hand : m_queued_removals) {
            remove(hand);
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
    
    template<typename Value_U>
    Handle_T add_into(Value_U&& val, 
            std::unordered_map<Handle_T, std::size_t>& hti,
            std::vector<Hanval_Pair>& vec) {
        std::size_t index = vec.size();
        Handle_T hand(m_next_handle);
        vec.push_back(Hanval_Pair(hand, std::forward<Value_U>(val)));
        ++m_next_handle;
        hti[hand] = index;
        return hand;
    }
    
    /**
     * @brief Removes from a map, vector pair
     * 
     * Removes the element given by the handle (element "A") by swapping it 
     * with the last element in the vector (element "B"), then popping off 
     * the last element (A). Must also update B's index (set B's index to 
     * A's old index).
     * 
     * The special case where A and B are the same element is handled 
     * implicitly.
     * @param handle_a
     * @param hti
     * @param vec
     * @return if something was actually done
     */
    bool remove_from(Handle_T handle_a, 
            std::unordered_map<Handle_T, std::size_t>& hti,
            std::vector<Hanval_Pair>& vec) {
        assert(vec.size() > 0);
        
        // Find the pair in the handle-to-index map
        auto map_entry_a = hti.find(handle_a);
        if (map_entry_a == hti.end()) {
            return false;
        }
        
        // Get the index of the entity in the entity vector
        std::size_t index_a = map_entry_a->second;
        
        // Get the index of the last entity
        std::size_t index_b = vec.size() - 1;
        
        // Get the handle of the last entity
        Handle_T handle_b = vec[index_b].m_handle;
        
        // Find the pair in the handle-to-index map
        auto map_entry_b = hti.find(handle_b);
        assert(map_entry_b != hti.end());
        
        // Set the index
        map_entry_b->second = index_a;
        
        // Swap this entity with the back and remove
        std::iter_swap(vec.begin() + index_a, vec.begin() + index_b);
        vec.pop_back();
        assert(vec.size() == index_b);
        assert(vec.size() == 0 || 
                hti[vec[index_a].m_handle] == index_a);
        
        // Remove the corresponding entry from the handle-to-index map
        hti.erase(map_entry_a);
        
        assert(!find(handle_a));
        
        return true;
    }
    
    Value_T* find_inside(Handle_T handle, 
            std::unordered_map<Handle_T, std::size_t>& hti,
            std::vector<Hanval_Pair>& vec) {
        auto iter = hti.find(handle);
        if (iter == hti.end()) {
            return nullptr;
        }
        std::size_t idx = iter->second;
        assert(idx >= 0 && idx < vec.size());
        return &(vec[idx].m_value);
    }
};
    
} // namespace Algs
} // namespace pegr

#endif // PEGR_ALGS_QIFUMAP_HPP
