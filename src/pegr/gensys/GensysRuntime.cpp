#include "pegr/gensys/GensysRuntime.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>

namespace pegr {
namespace Gensys {
namespace Runtime {

const int32_t EFLAG_SPAWNED = 0x0001;
const int32_t EFLAG_DEAD = 0x0002;
const std::size_t E_FLAG_CHAROFF = 0;
const std::size_t E_RCOUNT_CHAROFF = 4;

/**
 * @brief Private function for finding entity flags address
 * @param ent The entity
 * @return The memory address containing the 32-bitfield for an entity.
 */
int32_t* get_entity_flags_addr(const Entity_Ptr& ent) {
    return 
        static_cast<int32_t*>(static_cast<char*>(m_chunk) + E_FLAG_CHAROFF);
}

/**
 * @brief Private function for finding entity reference count address
 * @param ent The entity
 * @return The memory address containing the reference count for an entity.
 */
int32_t* get_entity_rcount_addr(const Entity_Ptr& ent) {
    return 
        static_cast<int32_t*>(static_cast<char*>(m_chunk) + E_RCOUNT_CHAROFF);
}

bool Entity_Ptr::has_been_spawned() const {
    return (*get_entity_flags_addr(*this) & EBM_SPAWNED) == EBM_SPAWNED;
}

bool Entity_Ptr::is_dead() const {
    return (*get_entity_flags_addr(*this) & EBM_DEAD) == EBM_DEAD;
}

bool Entity_Ptr::can_be_spawned() const {
    return !has_been_spawned() && !is_dead();
}

int32_t Entity_Ptr::get_lua_reference_count() const {
    return *get_entity_rcount_addr(*this);
}

Entity_Ptr new_entity(Arche* arche) {
    Entity_Ptr ent;
    
    assert(arche->m_default_chunk_size % 8 == 0);
    
    ent.m_archetype = arche;
    ent.m_chunk = new int64_t[1 + (arche->m_default_chunk_size / 8)];
    std::memcpy(
            ent.m_chunk + 1, arche->m_default_chunk, 
            arche->m_default_chunk_size);
    
    // No flags set, reference count is zero
    ent.m_chunk[0] = 0;
    ent.m_strings = new std::string[arche->m_num_strings];
    for (std::size_t idx = 0; idx < arche->m_num_strings; ++idx) {
        const char* str_data = arche->m_aggregate_default_strings
                + arche->m_default_string_offsets[idx];
        std::size_t str_len = arche->m_default_string_offsets[idx + 1]
                - arche->m_default_string_offsets[idx];
        ent.m_strings[idx] = std::string(str_data, str_len);
    }
    return ent;
}

void kill_entity(Entity_Ptr ent) {
    if (ent.is_dead()) {
        return;
    }
    ent.m_chunk[0] |= EBM_DEAD;
    delete[] ent.m_strings;
}

void delete_entity(Entity_Ptr ent) {
    kill_entity(ent);
    delete[] ent.m_chunk;
}

void grab_entity(Entity_Ptr ent) {
    int32_t& ref_count = *get_entity_rcount_addr(ent);
    ++ref_count;
}

void drop_entity(Entity_Ptr ent) {
    int32_t& ref_count = *get_entity_rcount_addr(ent);
    --ref_count;
    if (ref_count == 0) {
        delete_entity(ent);
    }
}

Aview::Aview(Entity_Ptr ent)
: m_entity(ent) {}

} // namespace Runtime
} // namespace Gensys
} // namespace pegr
