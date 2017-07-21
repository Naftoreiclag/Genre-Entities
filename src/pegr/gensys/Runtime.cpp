#include "pegr/gensys/Runtime.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <cstring>

#include "pegr/logger/Logger.hpp"
#include "pegr/gensys/Util.hpp"

namespace pegr {
namespace Gensys {
namespace Runtime {

std::map<std::string, std::unique_ptr<Runtime::Comp> > m_runtime_comps;
std::map<std::string, std::unique_ptr<Runtime::Arche> > m_runtime_arches;
std::map<std::string, std::unique_ptr<Runtime::Genre> > m_runtime_genres;

const uint64_t ENT_IDX_FLAGS = 0;
const uint64_t ENT_IDX_INST = ENT_IDX_FLAGS + 8;

const uint64_t ENT_FLAG_SPAWNED =           1 << 0;
const uint64_t ENT_FLAG_KILLED =            1 << 1;
const uint64_t ENT_FLAG_LUA_OWNED =         1 << 2;
const uint64_t ENT_FLAGS_DEFAULT =          0;

uint64_t n_next_handle = 0;
std::vector<Entity> n_entities;
std::unordered_map<uint64_t, std::size_t> n_handle_to_index;

Entity_Handle::Entity_Handle(uint64_t id)
: m_data(id) {}

Entity_Handle::Entity_Handle()
: m_data(-1) {}

uint64_t Entity_Handle::get_id() const {
    return m_data;
}
bool Entity_Handle::does_exist() const {
    return m_data != -1
            && n_handle_to_index.find(*this) != n_handle_to_index.end();
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

Entity* Entity_Handle::get_entity() const {
    if (m_data == -1) return nullptr;
    auto iter = n_handle_to_index.find(*this);
    if (iter == n_handle_to_index.end()) {
        return nullptr;
    }
    std::size_t idx = iter->second;
    assert(idx >= 0 && idx < n_entities.size());
    return &(n_entities[idx]);
}

Entity::Entity(const Arche* arche)
: m_arche(arche)
, m_handle(reserve_new_handle()) {
    
    m_chunk.reset(
            Pod::new_pod_chunk(8 + m_arche->m_default_chunk.get().get_size()));
    
    Pod::copy_pod_chunk(
            m_arche->m_default_chunk.get(), 0, 
            m_chunk.get(), 8,
            m_arche->m_default_chunk.get().get_size());
    
    m_chunk.get().set_value<uint64_t>(0, ENT_FLAGS_DEFAULT);
    m_strings = m_arche->m_default_strings;
}

Entity::Entity()
: m_arche(nullptr) {}

Entity_Handle Entity::new_entity(const Arche* arche) {
    // Record what the entity's index in the vector will be
    std::size_t index = n_entities.size();
    
    // Emplace-back a new entity
    n_entities.emplace_back(arche);
    
    // If there is a failure for some reason
    if (n_entities.size() != index + 1) {
        Logger::log()->warn("Unable to create new entity!");
        
        // Return a null handle
        return Entity_Handle();
    }
    
    // Get the entity we just created by reference
    Entity& ent = n_entities.back();
    
    // Map the entity handle to that index in the vector
    Entity_Handle hand = ent.get_handle();
    n_handle_to_index[hand] = index;
    
    // Return handle to newly created entity
    return hand;
}

void Entity::delete_entity(Entity_Handle handle_a) {
    /* Delete the entity given by the handle (entity "A") by swapping it with
     * the last entity in the vector (entity "B"), then popping off the last
     * entity (A). Must also update B's index (set B's index to A's old index).
     * 
     * The special case where A and B are the same entity is handled implicitly.
     */
    
    assert(n_entities.size() > 0);
    
    // Find the pair in the handle-to-index map
    auto map_entry_a = n_handle_to_index.find(handle_a);
    assert(map_entry_a != n_handle_to_index.end());
    
    // Get the index of the entity in the entity vector
    std::size_t index_a = map_entry_a->second;
    
    // Get the index of the last entity
    std::size_t index_b = n_entities.size() - 1;
    
    // Get the handle of the last entity
    Entity_Handle handle_b = n_entities[index_b].get_handle();
    
    // Find the pair in the handle-to-index map
    auto map_entry_b = n_handle_to_index.find(handle_b);
    assert(map_entry_b != n_handle_to_index.end());
    
    // Set the index
    map_entry_b->second = index_b;
    
    // Swap this entity with the back and remove
    std::iter_swap(n_entities.begin() + index_a, n_entities.begin() + index_b);
    n_entities.pop_back();
    assert(n_entities.size() == index_b);
    
    // Remove the corresponding entry from the handle-to-index map
    n_handle_to_index.erase(map_entry_a);
    assert(n_handle_to_index.find(handle_a) == n_handle_to_index.end());
}

const Arche* Entity::get_arche() const {
    return m_arche;
}
Pod::Chunk_Ptr Entity::get_chunk() const {
    return m_chunk.get();
}
Entity_Handle Entity::get_handle() const {
    return m_handle;
}

uint64_t Entity::get_flags() const {
    return m_chunk.get().get_value<uint64_t>(ENT_IDX_FLAGS);
}

bool Entity::has_been_spawned() const {
    return get_flags() & ENT_FLAG_SPAWNED == ENT_FLAG_SPAWNED;
}
bool Entity::is_alive() const {
    uint64_t flags = get_flags();
    // Spawned and not killed
    return (flags & ENT_FLAG_SPAWNED == ENT_FLAG_SPAWNED)
            && (flags & ENT_FLAG_KILLED != ENT_FLAG_KILLED);
}
bool Entity::has_been_killed() const {
    return get_flags() & ENT_FLAG_KILLED == ENT_FLAG_KILLED;
}
bool Entity::can_be_spawned() const {
    return !has_been_spawned();
}
bool Entity::is_lua_owned() const {
    return get_flags() & ENT_FLAG_LUA_OWNED == ENT_FLAG_LUA_OWNED;
}

void Entity::set_flags(uint64_t flags, bool set) {
    uint64_t& my_flags = m_chunk.get().get_value<uint64_t>(ENT_IDX_FLAGS);
    if (set) {
        my_flags |= flags;
    } else {
        my_flags &= ~flags;
    }
}
    
void Entity::set_flag_spawned(bool flag) {
    set_flags(ENT_FLAG_SPAWNED, flag);
}
void Entity::set_flag_lua_owned(bool flag) {
    set_flags(ENT_FLAG_LUA_OWNED, flag);
}
void Entity::set_flag_killed(bool flag) {
    set_flags(ENT_FLAG_KILLED, flag);
}
void initialize() {
}
void cleanup() {
    n_entities.clear();
    // Very important: otherwise entity handles may wrongly report existence.
    n_handle_to_index.clear();
    
    n_next_handle = 0;
    m_runtime_comps.clear();
    m_runtime_arches.clear();
    m_runtime_genres.clear();
}

Entity_Handle reserve_new_handle() {
    return Entity_Handle(n_next_handle++);
}

Runtime::Comp* find_component(std::string id_str) {
    return Util::find_something(m_runtime_comps, id_str, 
            "Could not find component: %v");
}
Runtime::Arche* find_archetype(std::string id_str) {
    return Util::find_something(m_runtime_arches, id_str, 
            "Could not find archetype: %v");
}
Runtime::Genre* find_genre(std::string id_str) {
    return Util::find_something(m_runtime_genres, id_str, 
            "Could not find genre: %v");
}

} // namespace Runtime
} // namespace Gensys
} // namespace pegr
