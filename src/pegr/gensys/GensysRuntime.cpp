#include "pegr/gensys/GensysRuntime.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>

namespace pegr {
namespace Gensys {
namespace Runtime {

const int32_t EFLAG_SPAWNED = 0x0001;
const int32_t EFLAG_DEAD = 0x0002;
const std::size_t E_CHAROFF_FLAGS = 0;
const std::size_t E_CHAROFF_RCOUNT = 4;

/**
 * @brief Private function for finding entity flags address
 * @param ent The entity
 * @return The memory address containing the 32-bitfield for an entity.
 */
int32_t* get_entity_flags_addr(const Entity_Ptr& ent) {
    return static_cast<int32_t*>(
            static_cast<void*>(
            static_cast<char*>(
            static_cast<void*>(ent.get_chunk())) + E_CHAROFF_FLAGS));
}

/**
 * @brief Private function for finding entity reference count address
 * @param ent The entity
 * @return The memory address containing the reference count for an entity.
 */
int32_t* get_entity_rcount_addr(const Entity_Ptr& ent) {
    return static_cast<int32_t*>(
            static_cast<void*>(
            static_cast<char*>(
            static_cast<void*>(ent.get_chunk())) + E_CHAROFF_RCOUNT));
}

Entity_Ptr::Entity_Ptr()
: m_is_smart(false)
, m_archetype(nullptr)
, m_chunk(nullptr)
, m_strings(nullptr) {}

Entity_Ptr::Entity_Ptr(
        const Arche* arche, int64_t* chunk, std::string* strings)
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
    rhs.m_chunk = nullptr;
    rhs.m_strings = nullptr;
    
    /* If we are moving from a smart pointer, then do nothing because the net
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
    if (m_is_smart) {
        grab_entity(*this);
    }
    return *this;
}

// Move assignment
Entity_Ptr& Entity_Ptr::operator =(Entity_Ptr&& rhs) {
    rhs.m_is_smart = false;
    rhs.m_archetype = nullptr;
    rhs.m_chunk = nullptr;
    rhs.m_strings = nullptr;
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
    m_chunk = nullptr;
    m_strings = nullptr;
}

const Arche* Entity_Ptr::get_archetype() const {
    return m_archetype;
}

int64_t* Entity_Ptr::get_chunk() const {
    return m_chunk;
}

std::string* Entity_Ptr::get_strings() const {
    return m_strings;
}

bool Entity_Ptr::has_been_spawned() const {
    return (*get_entity_flags_addr(*this) & EFLAG_SPAWNED) == EFLAG_SPAWNED;
}

bool Entity_Ptr::is_dead() const {
    return (*get_entity_flags_addr(*this) & EFLAG_DEAD) == EFLAG_DEAD;
}

bool Entity_Ptr::can_be_spawned() const {
    return !has_been_spawned() && !is_dead();
}

int32_t Entity_Ptr::get_lua_reference_count() const {
    return *get_entity_rcount_addr(*this);
}

Entity_Ptr new_entity(Arche* arche) {
    
    assert(arche->m_default_chunk_size % 8 == 0);
    
    int64_t* chunk = new int64_t[1 + (arche->m_default_chunk_size / 8)];
    std::memcpy(
            &(chunk[1]), arche->m_default_chunk, 
            arche->m_default_chunk_size);
    
    // No flags set, reference count is zero
    chunk[0] = 0;
    
    
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
    ent.get_chunk()[0] |= EFLAG_DEAD;
    delete[] ent.get_strings();
}

void delete_entity(const Entity_Ptr& ent) {
    kill_entity(ent);
    delete[] ent.get_chunk();
}

void grab_entity(const Entity_Ptr& ent) {
    int32_t& ref_count = *get_entity_rcount_addr(ent);
    ++ref_count;
}

void drop_entity(const Entity_Ptr& ent) {
    int32_t& ref_count = *get_entity_rcount_addr(ent);
    --ref_count;
    if (ref_count == 0) {
        delete_entity(ent);
    }
}

} // namespace Runtime
} // namespace Gensys
} // namespace pegr
