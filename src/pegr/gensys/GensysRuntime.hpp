#ifndef PEGR_GENSYS_RUNTIME_GENSYSRUNTIME_HPP
#define PEGR_GENSYS_RUNTIME_GENSYSRUNTIME_HPP

#include <cstdint>
#include <string>
#include <map>

#include "pegr/Script/Script.hpp"

namespace pegr {
namespace Gensys {
namespace Runtime {

typedef std::string Symbol;

struct Prim {
    enum struct Type {
        // Uses size_t, goes into string array
        STR,
        
        // Uses size_t, goes into byte chunk
        F32, F64,
        I32, I64,
        
        // Uses Arridx, goes into the Gensys table
        FUNC,
        
        // Enum size
        ENUM_SIZE
    };
    
    // Indicates which member to use and where it points to
    Type m_type;
    
    // What this actually points to depends on the type and whether this is a
    // member of an archetype or a member of a genre
    union {
        Script::Arridx m_table_idx;
        std::size_t m_byte_offset;
    };
};

/**
 * @class Arche
 * @brief 
 * Memory management of the contained pointers is not the responsibility of 
 * this class.
 */
struct Arche {
    /* Default chunk which is memcpy'd into the entity's chunk. These chunks
     * only contain POD types.
     */
    std::size_t m_default_chunk_size;
    int64_t* m_default_chunk;
    
    // It may not be necessary to store how many strings there are, but the 
    // extra few bytes won't make a difference...
    std::size_t m_num_strings;
    const char* m_aggregate_default_strings;
    std::size_t* m_default_string_offsets;
    
    // Named members of this archetype
    std::map<Symbol, Prim> m_members;
};

/**
 * @class Component
 * @brief Maps archetypes to where the component is within the chunk
 */
struct Component {
    std::map<Symbol, Prim> m_member_offset;
    std::map<Arche*, size_t> m_data_offsets;
};

/**
 * @class Entity_Ptr
 * @brief The entity's "real data" doesn't exist in a single self-contained
 * struct (it can't), and so what would be called an "Entity" is really a
 * collection of relevant pointers. Therefore, these are constant-size pointers
 * to be copied and passed around indefinitely. Memory management of the
 * contained pointers is not the responsibility of this class.
 */
struct Entity_Ptr {
    const Arche* m_archetype;
    
    /* The POD chunk containing flags, lua reference counts, and instance data
     * This chunk is aligned for 64-bit values. This guarantees that it is also 
     * aligned for 32-bit values. This array is used to store values of small 
     * fixed-sized types (float, int, vec3, etc...)
     * 
     * The first 32 bit integer (0) stores flags. All of these flags are
     * initially set to zero (unset):
     * 00   Set if this has ever been spawned.
     * 01   Set if this entity has been despawned ("dead").
     *      If this is set, then this also means that the instance data part of
     *      the chunk is no longer accessible, possibly because it has been
     *      freed.)
     * 
     * The second 32 bit integer (1) stores how many active Lua references there
     * are for this object. The entity data can only be completely cleaned up
     * when this value reaches zero. This means that the entity may be cleaned
     * up during a Lua GC cycle.
     * 
     * The instance data comprises the remainder of the memory block.
     */
    int64_t* m_chunk;
    
    // Array of strings that replace the archetype defaults.
    // TODO: a bitfield for checking if the strings are actually different
    // from the defaults (save space)
    std::string* m_strings;
    
    bool has_been_spawned() const;
    bool is_dead() const;
    bool can_be_spawned() const;
    int32_t get_lua_reference_count() const;
};

/**
 * @brief Creates a new unspawned entity
 * @param arche The archetype to use for its creation
 * @return Pointer for that entity
 */
Entity_Ptr new_entity(Arche* arche);

/**
 * @brief Sets the death flag and invalidates the entity's instance data.
 * Note that this does not free the entity from whatever may be in charge of
 * its lifetime, such as a world instance. Does nothing if the entity is
 * already dead.
 * @param ent The entity to kill
 */
void kill_entity(Entity_Ptr ent);

/**
 * @brief Undoes the effect of new_entity(). Completely deletes the entity's
 * memory. Accessing the entity through an Entity_Ptr at this point is
 * undefined behavior, just like accessing any other C++ object through a 
 * pointer after deletion.
 * @param ent The entity to delete. Must have been created through new_entity()
 */
void delete_entity(Entity_Ptr ent);

/**
 * @brief Increments the reference count for the entity.
 * @param ent
 */
void grab_entity(Entity_Ptr ent);

/**
 * @brief Decrements the reference count for the entity, deleting it if it
 * reaches zero.
 * @param ent
 */
void drop_entity(Entity_Ptr ent);

struct Genre {
    std::map<Symbol, Prim> m_member_lookup;
    std::map<Arche*, std::size_t> m_archetype_lookup;
};

/**
 * @class Aview
 * @brief Enables mutation of an entity through its archetype. This view is the
 * most permissive possible, granting access to all of the entity's data
 * directly.
 */
struct Aview {
    Entity_Ptr m_entity;
};

/**
 * @class Gview
 * @brief Interfaces with an underlying entity via a genre. This view may or
 * may not grant access to the entity's data; it really depends on the nature
 * of the genre.
 */
struct Gview {
    Genre* m_genre;
    Entity_Ptr m_entity;
};

/**
 * @class Cview
 * @brief Allows access to only one of the components
 */
struct Cview {
    // TODO
};

} // namespace Runtime
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_RUNTIME_GENSYSRUNTIME_HPP
