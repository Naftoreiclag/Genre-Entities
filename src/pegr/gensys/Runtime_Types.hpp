#ifndef PEGR_GENSYS_RUNTIMETYPES_HPP
#define PEGR_GENSYS_RUNTIMETYPES_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <map>

#include "pegr/script/Script.hpp"
#include "pegr/gensys/Pod_Chunk.hpp"

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
        std::size_t m_index;
    };
};

/**
 * @class Comp
 * @brief Maps archetypes to where the component is within the chunk
 */
struct Comp {
    std::map<Symbol, Prim> m_member_offsets;
};

/**
 * @brief These values are added to the "raw" index provided in the map
 * Component::m_member_offsets
 */
struct Aggindex {
    std::size_t m_pod_idx;
    std::size_t m_string_idx;
};

/**
 * @class Arche
 * @brief
 * Memory management of the contained pointers is not the responsibility of
 * this class.
 */
struct Arche {
    /* Archetypes are composed of components. This maps the internal name to
     * the actual component.
     */
    std::map<Symbol, Comp*> m_components;

    /* Given a component, provides the offsets into the various aggregate
     * arrays that store the first member of that type. For instance, when
     * looking for an int32, first locate the component-relative offset from
     * the Component object and then add that offset to the m_pod_idx offset
     * provided in this map, using the Component's memory address as a key.
     */
    std::map<Comp*, Aggindex> m_comp_offsets;

    /* Default chunk which is fast-copied into the entity's chunk. These chunks
     * only contain POD types.
     */
    Pod::Unique_Chunk_Ptr m_default_chunk;

    /* Default collection of default strings
     */
    std::vector<std::string> m_default_strings;
};

struct Genre {
    /* First, look into this array to convert symbol into an index into the
     * vector in the map m_archetype_lookup
     */
    std::map<Symbol, Prim> m_member_indices;

    /* Then, lookup the archetype in this map
     */
    std::map<Arche*, std::vector<Aggindex> > m_archetype_lookup;
};

class Entity;

/**
 * @class Entity_Handle
 * @brief Since Lua may have holds on Entity instances, rather than pass around
 * pointers, we use 64-bit handles. Note: we used raw pointers to Comp, Arche, 
 * etc. earlier because those objects are guaranteed to exist during the
 * lifetime of a gensys ecosystem. This is not true for Entity, and so we
 * cannot use raw pointers because those can 1) become invalidated and 2)
 * suddenly point to another Entity.
 * Acts like plain ol data.
 */
class Entity_Handle {
public:
    Entity_Handle();
    explicit Entity_Handle(int64_t id);
    
    int64_t get_id() const;
    
    /**
     * @brief Returns false if any of the following conditions are true:
     *  -   This handle has no entity associated with it at all, such as
     *      immediately after using the default constructor
     *  -   The entity that this handle once referred to was deleted (and
     *      therefore there is absolutely no data that this handle can
     *      return.)
     * This function returns false iff operator-> returns nullptr
     * Note that existence implies that the entity has been despawned, but
     * being despawned does not imply non-existence.
     * 
     * Existence indicates whether or not any data is retrievable from the 
     * entity instance.
     * 
     * @return Whether an entity "exists"
     */
    bool does_exist() const;
    
    /**
     * @brief Rather than expose the get_entity() method, we use an arrow
     * operator. This also allows Entity members to be accessed directly.
     * Users should check does_exist() before using.
     */
    Entity* operator ->() const;
    
    explicit operator bool() const;
    operator int64_t() const;
    
private:
    int64_t m_data;
    
    /**
     * @brief Returns the memory address of the enclosed entity. Note that there
     * is no guarantee that the return of this value will be consistent over
     * the lifetime of the handle, and therefore it cannot be cached and must
     * be re-fetched at the beginning of each method call.
     * @return "volatile" memory address or nullptr if the handle points to
     * nothing or the entity has been deleted.
     */
    Entity* get_entity() const;
};


extern const int64_t ENT_FLAG_SPAWNED;
extern const int64_t ENT_FLAG_KILLED;
extern const int64_t ENT_FLAGS_DEFAULT;

/**
 * @class Entity
 * 
 * Typical entity lifecycle:
 * +---+--------+---------+-------+--------+----------------+
 * | i | exists | spawned | alive | killed | can be spawned |
 * +---+--------+---------+-------+--------+----------------+
 * | 0 |   -    |    -    |   -   |   -    |       -        |
 * | 1 |   X    |    -    |   -   |   -    |       X        |
 * | 2 |   X    |    X    |   X   |   -    |       -        |
 * | 3 |   X    |    X    |   -   |   X    |       -        |
 * | 4 |   -    |    -    |   -   |   -    |       -        |
 * +---+--------+---------+-------+--------+----------------+
 * 
 * 0. No call to new_entity, no handle can report that this entity exists.
 * 1. Call to new_entity, entity exists and can be spawned
 * 2. Entity spawned, entity has been spawned and is alive, can not be spawned.
 * 3. Entity despawned, entity has been spawned and is dead, still unspawnable.
 * 4. Call to delete_entity, entity does not exist anymore
 * 
 * This table also shows all possible states that all entities can have.
 * 
 * killed implies spawned
 * alive implies spawned
 * killed implies not alive
 * alive implies not killed
 * can be spawned implies not spawned
 * spawned implies not can be spawned
 * 
 */
class Entity {
public:
    static Entity_Handle new_entity(const Arche* arche);
    static void delete_entity(Entity_Handle handle);
    
    Entity(const Entity& rhs) = delete;
    Entity(Entity&& rhs) = default;
    Entity& operator =(const Entity& rhs) = delete;
    Entity& operator =(Entity&& rhs) = default;
    
    /**
     * @return m_archetype, pointer to the archetype which created the entity.
     */
    const Arche* get_arche() const;
    
    /**
     * @return m_chunk, the data chunk which holds instance data
     */
    Pod::Chunk_Ptr get_chunk() const;
    
    /**
     * @return m_handle The handle for this entity
     */
    Entity_Handle get_handle() const;

    /**
     * @return m_strings, the array of strings for replacement instance data
     */
    const std::vector<std::string>& get_strings() const;

    /**
     * @return The set of flags that all entity instances must have, particular
     * to this instance
     */
    int64_t get_flags() const;
    
    /**
     * @return If this entity has ever been spawned. This value can be true
     * even if the entity is dead.
     */
    bool has_been_spawned() const;
    
    /**
     * @return If the entity has been newly constructed but not yet spawned.
     * True iff has_been_spawned() returns false
     */
    bool can_be_spawned() const;
    
    /**
     * @return If the entity has been spawned and has not been despawned.
     */
    bool is_alive() const;
    
    /**
     * @return If the entity has been spawned but more recently has been
     * despawned.
     */
    bool has_been_killed() const;

    /**
     * @brief Constructor. You likely do not want to use this. Use the static
     * factory methods instead.
     */ 
    explicit Entity(const Arche* arche);
    
    /**
     * @brief Default constructor for moving to. Use the static factory methods
     * to create new instances. Used for making temporaries only.
     */
    Entity();
    
private:
    // The archetype used by the entity (maybe add to the chunk?)
    const Arche* m_arche;

    /* The POD chunk containing flags, lua reference counts, and instance data
     * This chunk is aligned for 64-bit values. This guarantees that it is also
     * aligned for 32-bit values. This array is used to store values of small
     * fixed-sized types (float, int, vec3, etc...)
     *
     * The first 64 bit integer (0) stores flags. All of these flags are
     * initially set to zero (unset):
     * 00   Set if this has ever been spawned.
     * 01   Set if this entity has been despawned ("dead").
     *      If this is set, then this also means that the instance data part of
     *      the chunk is no longer accessible, possibly because it has been
     *      freed.)
     *
     * The instance data comprises the remainder of the memory block. Only
     * constant-size data is stored here.
     */
    Pod::Unique_Chunk_Ptr m_chunk;

    // Array of strings that replace the archetype defaults.
    // TODO: a bitfield for checking if the strings are actually different
    // from the defaults (save space)
    std::vector<std::string> m_strings;
    
    Entity_Handle m_handle;
};

} // namespace Runtime
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_RUNTIMETYPES_HPP
