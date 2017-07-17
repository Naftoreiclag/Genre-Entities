#ifndef PEGR_GENSYS_RUNTIMETYPES_HPP
#define PEGR_GENSYS_RUNTIMETYPES_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <map>

#include "pegr/script/Script.hpp"
#include "pegr/gensys/PodChunk.hpp"

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
 * @class Component
 * @brief Maps archetypes to where the component is within the chunk
 */
struct Component {
    std::map<Symbol, Prim> m_member_offsets;
};

/**
 * @brief These values are added to the "raw" index provided in the map
 * Component::m_member_offsets
 */
struct Aggregate_Offset {
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
    std::map<Symbol, Component*> m_components;

    /* Given a component, provides the offsets into the various aggregate
     * arrays that store the first member of that type. For instance, when
     * looking for an int32, first locate the component-relative offset from
     * the Component object and then add that offset to the m_pod_idx offset
     * provided in this map, using the Component's memory address as a key.
     */
    std::map<Component*, Aggregate_Offset> m_comp_offsets;

    /* Default chunk which is memcpy'd into the entity's chunk. These chunks
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
    std::map<Arche*, std::vector<Aggregate_Offset> > m_archetype_lookup;
};

/**
 * @class Entity_Ptr
 * @brief The entity's "real data" doesn't exist in a single self-contained
 * struct (it can't), and so what would be called an "Entity" is really a
 * collection of relevant pointers. Therefore, these are constant-size pointers
 * to be copied and passed around indefinitely. Memory management of the
 * contained pointers is not the responsibility of this class.
 */
class Entity_Ptr {
public:
    // Default constructor, makes a null pointer
    Entity_Ptr();

    // Constructs a pointer given all the Entity "members"
    Entity_Ptr(const Arche* arche, Pod::Chunk_Ptr chunk, std::string* strings);

    // Copy constructor
    Entity_Ptr(const Entity_Ptr& rhs);

    // Move constructor
    Entity_Ptr(Entity_Ptr&& rhs);

    // Copy assignment
    Entity_Ptr& operator =(const Entity_Ptr& rhs);

    // Move assignment
    Entity_Ptr& operator =(Entity_Ptr&& rhs);

    // Deconstructor
    ~Entity_Ptr();

    /**
     * @return True iff this pointer points to no entity
     */
    bool is_nullptr() const;

    /**
     * @brief Resets this pointer to the nullptr state (no associated entity)
     */
    void make_nullptr();

    /**
     * @brief Turns this pointer into a smart, reference-counting pointer that
     * calls the proper grab and drop functions on the held entity. All future
     * copies of this pointer will also have the smart property.
     */
    void make_smart();

    /**
     * @brief Makes a copy of this pointer which does not reference-count. This
     * is the only way to make a weak copy of the pointer given a smart version
     * of the pointer.
     * @return The weak pointer for the same entity
     */
    Entity_Ptr make_weak_copy() const;

    /**
     * @return m_archetype, pointer to the archetype which created the entity.
     */
    const Arche* get_archetype() const;
    /**
     * @return m_chunk, the data chunk which holds instance data
     */
    Pod::Chunk_Ptr get_chunk() const;

    /**
     * @return m_strings, the array of strings for replacement instance data
     */
    std::string* get_strings() const;

    bool has_been_spawned() const;
    bool is_dead() const;
    bool can_be_spawned() const;
    int32_t get_lua_reference_count() const;

private:
    // The archetype used by the entity (maybe add to the chunk?)
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
     * The instance data comprises the remainder of the memory block. Only
     * constant-size data is stored here.
     */
    Pod::Chunk_Ptr m_chunk;

    // Array of strings that replace the archetype defaults.
    // TODO: a bitfield for checking if the strings are actually different
    // from the defaults (save space)
    std::string* m_strings;

    // If true, then this pointer will properly grab/drop the held entity. This
    // smartness property carries on to all copies of this pointer.
    bool m_is_smart;
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
void kill_entity(const Entity_Ptr& ent);

/**
 * @brief Undoes the effect of new_entity(). Completely deletes the entity's
 * memory. Accessing the entity through an Entity_Ptr at this point is
 * undefined behavior, just like accessing any other C++ object through a
 * pointer after deletion.
 * @param ent The entity to delete. Must have been created through new_entity()
 */
void delete_entity(const Entity_Ptr& ent);

/**
 * @brief Increments the reference count for the entity.
 * @param ent
 */
void grab_entity(const Entity_Ptr& ent);

/**
 * @brief Decrements the reference count for the entity, deleting it if it
 * reaches zero.
 * @param ent
 */
void drop_entity(const Entity_Ptr& ent);

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

#endif // PEGR_GENSYS_RUNTIMETYPES_HPP
