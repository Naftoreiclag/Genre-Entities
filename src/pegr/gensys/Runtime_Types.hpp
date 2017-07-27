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

/**
 * @class Prim
 * @brief Not really a primitive, but rather a way of locating the data within
 * the entity. The type determines both how to read/write the data, and which
 * unionized index to use when searching in the entity aggregate arrays and POD
 * chunk.
 */
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
    union Refer {
        /**
         * @brief This is an index into the global gensys Lua table that holds
         * Lua-exclusve data.
         * This member is a part of a union with other members prefixed by "u".
         * Used for [FUNC]
         */
        Script::Arridx m_table_idx;
        
        /**
         * @brief This is a location within the entity chunk. Measured in bytes.
         * This member is a part of a union with other members prefixed by "u".
         * Used for pod data types, [F32, F64, I32, I64]
         */
        std::size_t m_byte_offset;
        
        /**
         * @brief This is an index into various arrays that store C++ types.
         * This member is a part of a union with other members prefixed by "u".
         * Used for everything else [STR]
         */
        std::size_t m_index;
    };
    
    Refer m_refer;
};

/**
 * @class Comp
 * @brief Maps archetypes to where the component is within the chunk
 */
struct Comp {
    /**
     * @brief Maps a symbol to the primitive. When read/writing from an entity
     * instance, we must first look into this map to find where within a
     * component the data lives, and then add this offset to the corresponding
     * "component offset" as given by the entity's archetype.
     */
    std::map<Symbol, Prim> m_member_offsets;
};

/**
 * @class Arche
 */
struct Arche {

    /**
     * @brief These values are added to the "raw" index provided in the map
     * Component::m_member_offsets
     */
    struct Aggindex {
        /**
         * @brief Index for the first byte of the entity pod chunk.
         */
        std::size_t m_pod_idx;
        
        /**
         * @brief Index for the first string in the entity aggregate string 
         * array.
         */
        std::size_t m_string_idx;
    };
    
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
    
    /* Cached Lua value to provide when accessed in a Lua script. The compiler
     * does not populate this field automatically. A Lua userdata value is
     * created and handed to the Arche upon the first access.
     */
    Script::Unique_Regref m_lua_userdata;
};

/**
 * @class Genre
 */
struct Genre {
    /* These two maps produce a matrix of indices and byte offsets used to
     * locate data inside of an archetypical entity.
     * 
     * The analogy is that the symbol identifies the column, while the
     * archetype identifies the row. Every matching archetype has a row, and
     * every valid symbol has a column. The intersection of the row and column
     * give:
     * - The prim type (given by the column)
     * - Where to find the data (given by the )
     */
    
    struct Column {
        Prim::Type m_type;
        std::size_t m_index;
    };
    
    typedef std::vector<Prim::Refer> Row;
     
    /* First, look into this array to convert symbol into an index into the
     * vector in the map m_archetype_lookup:
     */
    std::map<Symbol, Column> m_member_indices;

    /* Then, lookup the archetype in this map, use the index provided by
     * m_member_indices and then 
     */
    std::map<Arche*, Row> m_archetype_lookup;
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
 * 
 * Note that since we will be passing these ids to Lua (and therefore be 
 * converting them into 64-bit floats), the bottom 52 bits must also guaranteed
 * be unique. (2^53 is the smallest positive value that, when stored in a 
 * 64-bit IEEE 754 double, cannot be incremented by one).
 */
class Entity_Handle {
public:
    Entity_Handle();
    explicit Entity_Handle(uint64_t id);
    
    uint64_t get_id() const;
    
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
     * 
     * Note that it might be beneficial to use get_volatile_entity_ptr(), but
     * doing so can be unsafe. The arrow operator is guaranteed to always return
     * a valid single-use pointer to the entity.
     */
    Entity* operator ->() const;
    
    explicit operator bool() const;
    operator uint64_t() const;
    
    /**
     * @brief WARNING! THIS POINTER STAYS VALID ONLY UNDER VERY SPECIFIC 
     * CONDITIONS! Segfaults ahoy!
     * 
     * This is only used when there are many calls to -> that are guaranteed
     * to have the same return value over time. This should happen so long as
     * no other entities are created or destroyed during that time.
     * 
     * On destruction:
     *      It's possible that the soon-to-be-deleted entity swaps places with 
     *      this one, causing the memory address for the entity to change.
     * On creation:
     *      Following vector iterator invalidation rules, if a resize occurs,
     *      then all pointers are invalidated. A resize may occur when adding
     *      an Entity (i.e. during creation)
     * 
     * In general, use this pointer only within the scope of single function,
     * don't store this value in any long-term container, and do not use this
     * value if there is any chance of entity creation or deletion during its
     * usage. For example, don't use this if you are also calling an addon's
     * function, which could create an entity.
     */
    Entity* get_volatile_entity_ptr() const;
    
private:
    uint64_t m_entity_id;
    
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

extern const uint64_t ENT_HEADER_FLAGS;
extern const uint64_t ENT_HEADER_SIZE;

extern const uint64_t ENT_FLAG_SPAWNED;
extern const uint64_t ENT_FLAG_KILLED;
extern const uint64_t ENT_FLAG_LUA_OWNED;
extern const uint64_t ENT_FLAGS_DEFAULT;

extern const Script::Arridx ENT_LTABLE_CVIEW_CACHE;

/**
 * @class Entity
 * 
 * Typical entity lifecycle:
 * +----+--------+---------+-------+--------+-----------+-------+------+
 * | #  | exists | spawned | alive | killed | spawnable | Lua-o | next |
 * +----+--------+---------+-------+--------+-----------+-------+------+
 * | A  |   -    |    -    |   -   |   -    |    -      |   -   | BC   |
 * | B  |   X    |    -    |   -   |   -    |    X      |   X   | CF   |
 * | C  |   X    |    -    |   -   |   -    |    X      |   -   | DF   |
 * | D  |   X    |    X    |   X   |   -    |    -      |   -   | E    |
 * | E  |   X    |    X    |   -   |   X    |    -      |   -   | F    |
 * | F  |   -    |    -    |   -   |   -    |    -      |   -   | -    |
 * +----+--------+---------+-------+--------+-----------+-------+------+
 * 
 * Lifetime flow: (see column "next")
 *      +--------------+
 *      |              v
 * A -> C -> D -> E -> F
 * v    ^              ^
 * B ---+--------------+
 * 
 * A. No call to new_entity, there are no handles to the entity and not even
 *    an ID reserved for it.
 * B. Entity was created via new_entity within a Lua script. If a handle to
 *    this entity is cleaned up during a Lua GC cycle, then this also deletes
 *    the entity (go to F). Entities in this state or later have a unique ID
 *    that can never be used by another Entity during a single gensys lifetime, 
 *    even if the entity is deleted.
 * C. Either a) the entity was created via new_entity within C++, b) or entity 
 *    in state B had its ownership passed from Lua to C++. The entity can be
 *    deleted in this state, since it hasn't been spawned yet (go to F).
 * D. Entity spawned, entity has been spawned and is alive, can not be spawned
 *    for a second time. The only way to delete the entity (arrive at F) is to
 *    first despawn it (go to E).
 * E. Entity despawned, entity has been spawned and is dead, still unspawnable.
 *    Entity is ready to be deleted (go to E).
 * F. Call to delete_entity, entity does not exist anymore. (In some sense, the
 *    entity itself is indestinguishable from state A, but somewhere out there 
 *    could still be handles with that entity's ID, and all of those handles
 *    must report that the entity is non-existent.)
 * 
 * This table also shows all possible states that all entities can have.
 * 
 * killed implies spawned
 * alive implies spawned
 * killed implies not alive
 * alive implies not killed
 * can be spawned implies not spawned
 * spawned implies not can be spawned
 * spawned implies not Lua-owned
 * 
 */
class Entity {
public:
    static Entity_Handle new_entity(Arche* arche);
    static void delete_entity(Entity_Handle handle);
    
    Entity(const Entity& rhs) = delete;
    Entity(Entity&& rhs) = default;
    Entity& operator =(const Entity& rhs) = delete;
    Entity& operator =(Entity&& rhs) = default;
    
    /**
     * @return m_archetype, pointer to the archetype which created the entity.
     */
    Arche* get_arche() const;
    
    /**
     * @return m_chunk, the data chunk which holds instance data
     */
    Pod::Chunk_Ptr get_chunk() const;
    
    /**
     * @return m_handle The handle for this entity
     */
    Entity_Handle get_handle() const;
    
    /**
     * @return m_generic_lua_table The extra lua data associated with this
     * entity. If no table exists, generate one.
     */
    Script::Regref get_lua_table();
    
    /**
     * @brief Drops the reference to the internal Lua table
     */
    void free_lua_table();

    /**
     * @return m_strings, the array of strings for replacement instance data
     */
    std::string get_string(std::size_t idx) const;

    /**
     * @return The set of flags that all entity instances must have, particular
     * to this instance
     */
    uint64_t get_flags() const;
    
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
     * @return If deletion of the entity is the responsibility of the Lua GC
     */
    bool is_lua_owned() const;
    
    void set_flags(uint64_t flags, bool set);
    void set_flag_spawned(bool flag);
    void set_flag_lua_owned(bool flag);
    void set_flag_killed(bool flag);
    
    void set_string(std::size_t idx, std::string str);

    /**
     * @brief Constructor. You likely do not want to use this. Use the static
     * factory methods instead.
     */ 
    explicit Entity(Arche* arche);
    
    /**
     * @brief Default constructor for moving to. Use the static factory methods
     * to create new instances. Used for making temporaries only.
     */
    Entity();
    
private:
    // The archetype used by the entity (maybe add to the chunk?)
    Arche* m_arche;

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
    
    /**
     * @brief Sometimes it is useful to also have Lua data associated with the
     * entity. This points to a Lua table with any such data, or nil if there
     * is no data.
     */
    Script::Unique_Regref m_generic_lua_table;
    
};

} // namespace Runtime
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_RUNTIMETYPES_HPP
