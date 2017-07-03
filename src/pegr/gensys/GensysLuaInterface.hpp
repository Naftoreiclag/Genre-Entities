#ifndef PEGR_GENSYS_HPP
#define PEGR_GENSYS_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "pegr/script/Script.hpp"
#include "pegr/gensys/GensysIntermediate.hpp"

namespace pegr {
namespace Gensys {
namespace LI {


/**
 * @brief Initializes. Sets up workspaces for adding new components,
 * genres, archetypes, etc.
 */
void initialize();

/**
 * @brief Undoes initialize()
 */
void cleanup();

//// SETUP ////

/**
 * @brief Produces a typed primitive value from a Lua value.
 * Can throw runtime errors. Guaranteed to return a non-error Prim.
 * [BALANCED]
 * @param table_idx the index of the input Lua value on the main stack
 * @param required_t the returned primitive must be of this type, else error.
 * If this is Type::UNKNOWN, then the returned primitive can be of any type
 * @return a primitive value produced from the Lua value
 */
Interm::Prim parse_primitive(int table_idx, 
        Interm::Prim::Type required_t = Interm::Prim::Type::UNKNOWN);

/**
 * @brief Make a new component definition from the table at the given index.
 * Can throw runtime errors. Guaranteed to return a valid Comp_Def pointer.
 * User is responsible for calling delete.
 * [BALANCED]
 * @param table_idx The index on the main Lua stack to translate
 * @return The component
 */
Interm::Comp_Def* parse_component_definition(int table_idx);

/**
 * @brief Make an archetype implementation from the table at the given index.
 * Can throw runtime errors. Guaranteed to return a valid implementation.
 * [BALANCED]
 * @param table_idx The index on the main Lua stack to translate
 * @return The archetype implementation
 */
Interm::Arche::Implement parse_archetype_implementation(int table_idx);

/**
 * @brief Make a new archetype from the table at the given index.
 * Can throw runtime errors. Guaranteed to return a valid Arche pointer.
 * User is responsible for calling delete.
 * [BALANCED]
 * @param table_idx The index on the main Lua stack to translate
 * @return The archetype
 */
Interm::Arche* parse_archetype(int table_idx);

/**
 * @brief Make a genre pattern from the given Lua value.
 * Can throw runtime errors. Guaranteed to return a valid pattern.
 * [BALANCED]
 * @param table_idx The index on the main Lua stack to translate
 * @return The pattern
 */
Interm::Genre::Pattern parse_genre_pattern(int idx);

/**
 * @brief Make a new genre from the table at the given index.
 * Can throw runtime errors. Guaranteed to return a valid Genre pointer.
 * User is responsible for calling delete.
 * [BALANCED]
 * @param table_idx The index on the main Lua stack to translate
 * @return The genre
 */
Interm::Genre* parse_genre(int table_idx);

/**
 * @brief Translates all submitted Lua tables into their intermediate forms and
 * submits to the gensys pipeline.
 */
void stage_all();

int add_component(lua_State* l);
int edit_component(lua_State* l);
int add_archetype(lua_State* l);
int edit_archetype(lua_State* l);
int add_genre(lua_State* l);

//// RUNTIME ////

// MTI = metatable id
extern const char* MTI_COMPONENT;
extern const char* MTI_ARCHETYPE;
extern const char* MTI_GENRE;
extern const char* MTI_ENTITY;

int archetype_mt_tostring(lua_State* l);

// ...
int find_archetype(lua_State* l);

int new_entity(lua_State* l);

} // namespace LI
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_HPP
