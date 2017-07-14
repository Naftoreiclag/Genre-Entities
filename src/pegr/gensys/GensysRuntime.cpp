#include "pegr/gensys/GensysRuntime.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>

namespace pegr {
namespace Gensys {
namespace Runtime {

const int32_t EFLAG_SPAWNED = 0x0001;
const int32_t EFLAG_DEAD = 0x0002;
const std::size_t INST_OFF_FLAGS = 0;
const std::size_t INST_OFF_REF_COUNT = 4;
const std::size_t INST_OFF_SIZE = 8;

/**
 * @brief Private function for finding entity flags address
 * @param ent The entity
 * @return The memory address containing the 32-bitfield for an entity.
 */
int32_t* get_entity_flags_addr(Pod::Chunk_Ptr chunk) {
    return static_cast<int32_t*>(
            chunk.get_aligned<int32_t>(INST_OFF_FLAGS));
}

/**
 * @brief Private function for finding entity reference count address
 * @param ent The entity
 * @return The memory address containing the reference count for an entity.
 */
int32_t* get_entity_rcount_addr(Pod::Chunk_Ptr chunk) {
    return static_cast<int32_t*>(
            chunk.get_aligned<int32_t>(INST_OFF_REF_COUNT));
}

Entity_Ptr::Entity_Ptr()
: m_is_smart(false)
, m_archetype(nullptr)
, m_strings(nullptr) {}

Entity_Ptr::Entity_Ptr(
        const Arche* arche, Pod::Chunk_Ptr chunk, std::string* strings)
: m_is_smart(false)
, m_archetype(arche)
, m_chunk(chunk)
, m_strings(strings) {}
    
// Copy constructor
Entity_Ptr::Entity_Ptr(const Entity_Ptr& rhs)
: m_is_smart(rhs.m_is_smart)
, m_archetype(rhs.m_archetype)
, m_chunk(rhs.m_chunk)
, m_strings(rhs.m_strings) {
    if (m_is_smart) {
        grab_entity(*this);
    }
}

// Move constructor
Entity_Ptr::Entity_Ptr(Entity_Ptr&& rhs)
: m_is_smart(rhs.m_is_smart)
, m_archetype(rhs.m_archetype)
, m_chunk(rhs.m_chunk)
, m_strings(rhs.m_strings) {
    
    // (Do not call make_nullptr() on rhs because that could cause the entity
    // to be dropped and deleted, after which it cannot be incremented again.)
    rhs.m_is_smart = false;
    rhs.m_archetype = nullptr;
    rhs.m_chunk.make_nullptr();
    rhs.m_strings = nullptr;
    
    /* [MOVING OWNERSHIP]
     * If we are moving from a smart pointer, then do nothing because the net
     * result of incrementing the reference count and immediately decrementing
     * it is wasted time.
     * 
     * If we are not moving from a smart pointer, then do nothing trivially.
     * 
     * Therefore do not change the reference count when move constructing.
     */
}

// Copy assignment
Entity_Ptr& Entity_Ptr::operator =(const Entity_Ptr& rhs) {
    if (m_chunk == rhs.m_chunk) {
        return *this;
    }
    make_nullptr();
    m_is_smart = rhs.m_is_smart;
    m_archetype = rhs.m_archetype;
    m_chunk = rhs.m_chunk;
    m_strings = rhs.m_strings;
    
    // Also grab since this a copy
    if (m_is_smart) {
        grab_entity(*this);
    }
    return *this;
}

// Move assignment
Entity_Ptr& Entity_Ptr::operator =(Entity_Ptr&& rhs) {
    make_nullptr();
    m_is_smart = rhs.m_is_smart;
    m_archetype = rhs.m_archetype;
    m_chunk = rhs.m_chunk;
    m_strings = rhs.m_strings;
    
    // (Do not call make_nullptr() on rhs because that could cause the entity
    // to be dropped and deleted, after which it cannot be incremented again.)
    rhs.m_is_smart = false;
    rhs.m_archetype = nullptr;
    rhs.m_chunk.make_nullptr();
    rhs.m_strings = nullptr;

    // (See [MOVING_OWNERSHIP] above)

    return *this;
}

// Deconstructor
Entity_Ptr::~Entity_Ptr() {
    make_nullptr();
}

bool Entity_Ptr::is_nullptr() const {
    return m_archetype == nullptr;
}

void Entity_Ptr::make_nullptr() {
    if (m_is_smart) {
        drop_entity(*this);
    }
    m_is_smart = false;
    m_archetype = nullptr;
    
    // Not necessary, but who cares
    m_chunk.make_nullptr();
    m_strings = nullptr;
}

const Arche* Entity_Ptr::get_archetype() const {
    return m_archetype;
}

Pod::Chunk_Ptr Entity_Ptr::get_chunk() const {
    return m_chunk;
}

std::string* Entity_Ptr::get_strings() const {
    return m_strings;
}

bool Entity_Ptr::has_been_spawned() const {
    return (*get_entity_flags_addr(m_chunk) & EFLAG_SPAWNED) == EFLAG_SPAWNED;
}

bool Entity_Ptr::is_dead() const {
    return (*get_entity_flags_addr(m_chunk) & EFLAG_DEAD) == EFLAG_DEAD;
}

bool Entity_Ptr::can_be_spawned() const {
    return !has_been_spawned() && !is_dead();
}

int32_t Entity_Ptr::get_lua_reference_count() const {
    return *get_entity_rcount_addr(m_chunk);
}

Entity_Ptr new_entity(Arche* arche) {
    
    assert(arche->m_default_chunk.get_size() % 8 == 0);
    
    Pod::Chunk_Ptr chunk = Pod::new_pod_chunk(
            INST_OFF_SIZE + arche->m_default_chunk.get_size());
    
    Pod::copy_pod_chunk(
            arche->m_default_chunk, 0, 
            chunk, INST_OFF_SIZE, 
            arche->m_default_chunk.get_size());
            
    *get_entity_flags_addr(chunk) = 0;
    *get_entity_rcount_addr(chunk) = 0;
    
    
    std::string* strings = new std::string[arche->m_num_strings];
    for (std::size_t idx = 0; idx < arche->m_num_strings; ++idx) {
        const char* str_data = arche->m_aggregate_default_strings
                + arche->m_default_string_offsets[idx];
        std::size_t str_len = arche->m_default_string_offsets[idx + 1]
                - arche->m_default_string_offsets[idx];
        strings[idx] = std::string(str_data, str_len);
    }
    
    return Entity_Ptr(arche, chunk, strings);
}

void kill_entity(const Entity_Ptr& ent) {
    if (ent.is_dead()) {
        return;
    }
    *get_entity_flags_addr(ent.get_chunk()) |= EFLAG_DEAD;
    delete[] ent.get_strings();
}

void delete_entity(const Entity_Ptr& ent) {
    kill_entity(ent);
    Pod::delete_pod_chunk(ent.get_chunk());
}

void grab_entity(const Entity_Ptr& ent) {
    int32_t& ref_count = *get_entity_rcount_addr(ent.get_chunk());
    ++ref_count;
}

void drop_entity(const Entity_Ptr& ent) {
    int32_t& ref_count = *get_entity_rcount_addr(ent.get_chunk());
    --ref_count;
    if (ref_count == 0) {
        delete_entity(ent);
    }
}

} // namespace Runtime
} // namespace Gensys
} // namespace pegr
