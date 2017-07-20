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

const int64_t ENT_IDX_FLAGS = 0;
const int64_t ENT_IDX_INST = ENT_IDX_FLAGS + 8;

const int64_t ENT_FLAG_SPAWNED =        1 << 0;
const int64_t ENT_FLAG_KILLED =           1 << 1;
const int64_t ENT_FLAGS_DEFAULT =       0;

int64_t n_next_handle = 0;
std::vector<Entity> n_entities;
std::unordered_map<int64_t, std::size_t> n_handle_to_index;

Entity_Handle::Entity_Handle(int64_t id)
: m_data(id) {}

Entity_Handle::Entity_Handle()
: m_data(-1) {}

int64_t Entity_Handle::get_id() const {
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
Entity_Handle::operator int64_t() const {
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
    
    m_chunk.get().set_value<int64_t>(0, ENT_FLAGS_DEFAULT);
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

void Entity::delete_entity(Entity_Handle hand) {
    // The handle should always exist in the map
    assert(n_handle_to_index.find(hand) != n_handle_to_index.end());
    
    // Find the pair in the handle-to-index map
    auto idx_iter = n_handle_to_index.find(hand);
    
    // Get the index of the entity in the entity vector
    std::size_t index = idx_iter->second;
    
    // Swap this entity with the back and remove
    std::iter_swap(n_entities.begin() + index, n_entities.end() - 1);
    n_entities.pop_back();
    
    // Remove the corresponding entry from the handle-to-index map
    n_handle_to_index.erase(idx_iter);
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

int64_t Entity::get_flags() const {
    return m_chunk.get().get_value<int64_t>(ENT_IDX_FLAGS);
}

bool Entity::has_been_spawned() const {
    return get_flags() & ENT_FLAG_SPAWNED == ENT_FLAG_SPAWNED;
}
bool Entity::is_alive() const {
    int64_t flags = get_flags();
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

void initialize() {
}
void cleanup() {
    n_entities.clear();
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
