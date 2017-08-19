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

#ifndef PEGR_GENSYS_RUNTIMETYPES_HPP
#define PEGR_GENSYS_RUNTIMETYPES_HPP

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "pegr/gensys/Pod_Chunk.hpp"
#include "pegr/script/Script.hpp"

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
        
        // Used only for nullptr Member_Ptr
        NULLPTR,

        // Enum size
        ENUM_SIZE
    };

    // Indicates which member to use and where it points to
    Type m_type;

    // What this actually points to depends on the type and whether this is a
    // member of an archetype or a member of a genre
    union Refer {
        /**
         * @brief This is a location within the entity chunk. Measured in bytes.
         * This member is a part of a union.
         * Used for pod data types, [F32, F64, I32, I64]
         */
        std::size_t m_byte_offset;
        
        /**
         * @brief This is an index into various arrays that store C++ types.
         * This member is a part of a union.
         * Used for everything else [STR, FUNC]
         */
        std::size_t m_index;
    };
    
    Refer m_refer;
};

const char* prim_to_dbg_string(Prim::Type ty);

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
    
    /* Cached Lua value to provide when accessed in a Lua script. The compiler
     * does not populate this field automatically. A Lua userdata value is
     * created and handed to the Comp upon the first access.
     */
    Script::Unique_Regref m_lua_userdata;
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
        
        /**
         * @brief Index for the first string in the archetype aggregate func
         * array.
         */
        std::size_t m_func_idx;
    };
    
    /* Merely an array of all of the components that this Archetype uses. To
     * quickly check if an Archetype has every component in some set. (Genre
     * matching, namely.)
     */
    std::vector<Comp*> m_sorted_component_array;
    
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
    
    /* Vector of static lua references
     */
    std::vector<Script::Regref> m_static_funcs;
    
    /* Cached Lua value to provide when accessed in a Lua script. The compiler
     * does not populate this field automatically. A Lua userdata value is
     * created and handed to the Arche upon the first access.
     */
    Script::Unique_Regref m_lua_userdata;
};

/**
 * @class Member_Ptr
 * @brief Rather than return a bare void ptr, we return a Member_Ptr to make
 * runtime read/write checks easier.
 */
class Member_Ptr {
public:
    Member_Ptr(Prim::Type typ, void* ptr);
    Member_Ptr(); // nullptr
    
    // (Can't use overloading as some types are actually equivalent)
    
    void set_value_i32(std::int32_t val) const;
    void set_value_i64(std::int64_t val) const;
    void set_value_f32(float val) const;
    void set_value_f64(double val) const;
    void set_value_str(const std::string& val) const;
    void set_value_func(Script::Regref val) const;
    
    void get_value_i32(std::int32_t& val) const;
    void get_value_i64(std::int64_t& val) const;
    void get_value_f32(float& val) const;
    void get_value_f64(double& val) const;
    void get_value_str(std::string& val) const;
    void get_value_func(Script::Regref& val) const;
    
    void set_value_any_number(double val) const;
    void get_value_any_number(double& val) const;
    
private:
    Prim::Type m_type;
    void* m_ptr;
};

/**
 * @class Member_Key
 * @brief The penultimate step to "unlock" the void pointer for the data
 * representing an entity's member. See Entity::get_member()
 */
struct Member_Key {
    Member_Key(Arche::Aggindex aggidx, Prim prim);
    
    Arche::Aggindex m_aggidx;
    Prim m_prim;
};

/**
 * @class Genre
 */
struct Genre {
    struct Pattern {
        // Used when trying to see if an archetype matches the pattern
        std::vector<Comp*> m_sorted_required_comps_specific;
        
        struct Alias {
            // Used to find the position in the archetype
            Comp* m_comp;
            
            // Rather than copy the symbol names and then constantly lookup
            // in m_comp's member_offsets map, just cache the value here.
            Prim m_prim_copy;
        };
        std::map<Symbol, Alias> m_aliases;
        
        // TODO: static values
    };
    
    /* This is a sorted list of all of the components that is used in every
     * pattern. Essentially, if a component is lacking at least one of these
     * components, then it cannot match any of the patterns. This also means
     * that, to save space and time, the patterns' list of required components
     * contain only those components specific to them (i.e. not in this list)
     */
    std::vector<Comp*> m_sorted_required_intersection;
    
    /* The first matching pattern is used.
     */
    std::vector<Pattern> m_patterns;
    
    /* Cached Lua value to provide when accessed in a Lua script. The compiler
     * does not populate this field automatically. A Lua userdata value is
     * created and handed to the Genre upon the first access.
     */
    Script::Unique_Regref m_lua_userdata;
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
 * converting them into 64-bit floats), the bottom 52 bits are also guaranteed
 * to be unique. (2^53 is the smallest positive value that, when stored in a 
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
 *    entity itself is indistinguishable from state A, but somewhere out there 
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
    
    /* Entities can be moved (within the vector), but not copied.
     */
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
     * @return m_generic_table The extra Lua data associated with this
     * entity. If no table exists, generate one.
     */
    Script::Regref get_table();
    
    /**
     * @return m_generic_weak_table The extra Lua data associated with this
     * entity. The values are weak. If no table exists, generate one.
     * Commonly used for caching.
     */
    Script::Regref get_weak_table();
    
    /**
     * @brief Drops the reference to the internal Lua table
     */
    void free_table();
    
    /**
     * @brief Drops the reference to the internal Lua table
     */
    void free_weak_table();

    /**
     * @return m_strings, the array of strings for replacement instance data
     */
    std::string get_string(std::size_t idx) const;
    
    /**
     * @return static function in archetype
     */
    Script::Regref get_func(std::size_t idx) const;

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
    
    /**
     * @brief Changes the state of multiple flags at once. Sets all of the flags
     * specified in "flags" to the state specified in "set". Note that this does
     * not just simply set the flags to "flags". Instead, only those flags
     * bitmasked by "flags" are set to the state "set".
     * @param flags
     * @param set
     */
    void set_flags(uint64_t flags, bool set);
    
    /**
     * @brief Flags this entity as having been spawned or not
     */
    void set_flag_spawned(bool flag);
    
    /**
     * @brief Flags this entity as being lua-owned or not
     */
    void set_flag_lua_owned(bool flag);
    
    /**
     * @brief Flags this entity as having been killed or not
     */
    void set_flag_killed(bool flag);
    
    void set_string(std::size_t idx, std::string str);
    
    /**
     * @brief Get a void pointer referenced by this data. Note that this does
     * NO SANITY CHECKING WHATSOEVER. This method assumes that you have already
     * checked that the key is appropriate for this entity.
     * @param aggidx
     * @param prim
     */
    Member_Ptr get_member(const Member_Key& key);

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

    /* The POD chunk containing flags, Lua reference counts, and instance data
     * This chunk is aligned for 64-bit values. This guarantees that it is also
     * aligned for 32-bit values. This array is used to store values of small
     * fixed-sized types (float, int, vec3, etc...)
     *
     * The first 64 bit integer (0) stores flags. All of these flags are
     * initially set to zero (unset):
     * SPAWNED set if this has ever been spawned.
     * KILLED set if this entity has been despawned ("dead").
     *      If this is set, then this also means that the instance data part of
     *      the chunk is no longer accessible, possibly because it has been
     *      freed.)
     * LUA_OWNED set if this entity should be deleted when its handle is gc'd
     *      by Lua
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
    Script::Unique_Regref m_generic_table;
    
    /**
     * @brief Same as m_generic_table, except the values are weak.
     */
    Script::Unique_Regref m_generic_weak_table;
    
};

/**
 * @class Cview
 * @brief Component-based view on an entity. Essentially a cached aggregate
 * index on an entity. (No need to repeatedly lookup in the Arche's aggidx
 * table)
 */
struct Cview {
    Entity_Handle m_ent;
    Arche::Aggindex m_cached_aggidx;
    Comp* m_comp;

public:
    Member_Ptr get_member_ptr(const Symbol& member_symb) const;
    bool operator ==(const Cview& rhs) const;
};

/**
 * @class Genview
 * @brief Genre-based view on an entity.
 */
struct Genview {
    Entity_Handle m_ent;
    Genre::Pattern* m_pattern;
    
public:
    Member_Ptr get_member_ptr(const Symbol& member_symb) const;
    bool operator ==(const Genview& rhs) const;
};

} // namespace Runtime
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_RUNTIMETYPES_HPP
