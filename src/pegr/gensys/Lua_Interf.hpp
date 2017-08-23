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

#ifndef PEGR_GENSYS_LUAINTERF_HPP
#define PEGR_GENSYS_LUAINTERF_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "pegr/gensys/Interm_Types.hpp"
#include "pegr/gensys/Runtime.hpp"
#include "pegr/script/Script.hpp"

namespace pegr {
namespace Gensys {
namespace LI {

/**
 * @brief Initializes. Sets up workspaces for adding new components,
 * genres, archetypes, etc.
 * 
 * This should really only be called once during the program's lifetime.
 */
void initialize();

void clear();

/**
 * @brief Undoes initialize()
 * 
 * This should really only be called once during the program's lifetime.
 */
void cleanup();

//// SETUP ////

/**
 * @brief Produces a typed primitive value from a Lua value.
 * Can throw runtime errors. Guaranteed to return a non-error Prim.
 * [BALANCED]
 * @param table_idx the index of the input Lua value on the main stack
 * @param required_t the returned primitive must be of this type, else error.
 * If this is Type::UNKNOWN, then the returned primitive can be of any type.
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
std::unique_ptr<Interm::Comp> parse_component_definition(int table_idx);

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
std::unique_ptr<Interm::Arche> parse_archetype(int table_idx);

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
std::unique_ptr<Interm::Genre> parse_genre(int table_idx);

/**
 * @brief Translates all submitted Lua tables into their intermediate forms and
 * submits to the gensys pipeline.
 */
void stage_all();

int li_add_component(lua_State* l);
int li_edit_component(lua_State* l);
int li_add_archetype(lua_State* l);
int li_edit_archetype(lua_State* l);
int li_add_genre(lua_State* l);

//// RUNTIME ////

lua_Number entity_handle_to_lua_number(int64_t data);

/**
 * @brief Checks that the argument at narg is a userdata with the provided
 * metatable. Throws an argument type error with standard formatting if this
 * is not possible.
 * @param l The Lua state
 * @param narg The argument (positive)
 * @param metatable The metable the userdata ptr must have
 * @param dbg_tname The name of the type to use in the error message
 */
void* arg_require_userdata(lua_State* l, int narg, Script::Regref metatable,
            const char* dbg_tname);

/* The arg_require_X functions check that the given Lua argument is the correct
 * type, casting if it is or throwing a standard Lua arg type error otherwise.
 */
Runtime::Comp** arg_require_comp(lua_State* l, int narg);
Runtime::Arche** arg_require_arche(lua_State* l, int narg);
Runtime::Entity_Handle* arg_require_entity(lua_State* l, int narg);
Runtime::Cview* arg_require_cview(lua_State* l, int narg);
Runtime::Genview* arg_require_genview(lua_State* l, int narg);

/* The push_X functions push a new userdata ptr for the provided raw ptr,
 * applying all proper metatables.
 */
void push_gensys_obj(lua_State* l, Runtime::Comp* ptr);
void push_gensys_obj(lua_State* l, Runtime::Arche* ptr);
void push_gensys_obj(lua_State* l, Runtime::Genre* ptr);
void push_gensys_obj(lua_State* l, Runtime::Entity_Handle ent);
void push_gensys_obj(lua_State* l, Runtime::Cview ent);
void push_gensys_obj(lua_State* l, Runtime::Genview ent);

/**
 * @brief Attempts to get a component view for the provided entity. If this is
 * impossible, return nil.
 * 1: Comp (guaranteed)
 * 2: Entity
 */
int li_comp_mt_call(lua_State* l);

/**
 * @brief Basic tostring for Component
 * 1: Comp (guaranteed)
 */
int li_comp_mt_tostring(lua_State* l);

/**
 * @brief Attempts to get an archetype view for the provided entity. If this is
 * impossible, return nil.
 * 1: Arche (guaranteed)
 */
int li_arche_mt_call(lua_State* l);

/**
 * @brief Basic tostring for Archetype
 * 1: Arche (guaranteed)
 */
int li_arche_mt_tostring(lua_State* l);

/**
 * @brief Calls the deconstructor on the Entity_Handle. Also frees the entity
 * if it's memory is managed solely by Lua.
 * 1: Entity (guaranteed)
 */
int li_entity_mt_gc(lua_State* l);

/**
 * @brief Returns the component view for the requested component or something
 * else if it matches one of the special members that begin with "__"
 * 1: Entity (guaranteed)
 * 2: String, member
 */
int li_entity_mt_index(lua_State* l);

/**
 * @brief Basic tostring for Entity
 * 1: Entity (guaranteed)
 */
int li_entity_mt_tostring(lua_State* l);

/**
 * @brief Attempts to get a genre view for the provided entity. If this is
 * impossible, return nil.
 * 1: Genre (guaranteed)
 */
int li_genre_mt_call(lua_State* l);

/**
 * @brief Basic tostring for Genre
 * 1: Genre (guaranteed)
 */
int li_genre_mt_tostring(lua_State* l);

/**
 * @brief Equality between cviews implies both point to the same entity and both
 * are using the same component as a view
 * 1: Cview (guaranteed)
 * 2: Cview (guaranteed)
 */
int li_cview_mt_eq(lua_State* l);

/**
 * @brief Calls the deconstructor on the Cview.
 * 1: Cview (guaranteed)
 */
int li_cview_mt_gc(lua_State* l);

/**
 * @brief Returns by value the member held by the underlying entity provided
 * the symbol.
 * 1: Cview (guaranteed)
 * 2: String, symbol
 */
int li_cview_mt_index(lua_State* l);

/**
 * @brief Assigns to the member held by the underlying entity provided by the
 * symbol.
 * 1: Cview (guaranteed)
 * 2: String, symbol
 * 3: Value
 */
int li_cview_mt_newindex(lua_State* l);

/**
 * @brief Basic tostring for Cview
 * 1: Cview (guaranteed)
 */
int li_cview_mt_tostring(lua_State* l);

/**
 * @brief Equality between genviews implies both point to the same entity and 
 * both are using the same genre pattern as a view
 * 1: Genview (guaranteed)
 * 2: Genview (guaranteed)
 */
int li_genview_mt_eq(lua_State* l);

/**
 * @brief Calls the deconstructor on the Genview.
 * 1: Genview (guaranteed)
 */
int li_genview_mt_gc(lua_State* l);

/**
 * @brief Returns by value the member held by the underlying entity provided
 * the symbol.
 * 1: Genview (guaranteed)
 * 2: String, symbol
 */
int li_genview_mt_index(lua_State* l);

/**
 * @brief Assigns to the member held by the underlying entity provided by the
 * symbol.
 * 1: Genview (guaranteed)
 * 2: String, symbol
 * 3: Value
 */
int li_genview_mt_newindex(lua_State* l);

/**
 * @brief Basic tostring for Genview
 * 1: Genview (guaranteed)
 */
int li_genview_mt_tostring(lua_State* l);

/**
 * @brief Find the component given by the resource ID.
 * 1: String, resource id
 */
int li_find_comp(lua_State* l);

/**
 * @brief Find the archetype given by the resource ID.
 * 1: String, resource id
 */
int li_find_archetype(lua_State* l);

/**
 * @brief Find the genre given by the resource ID.
 * 1: String, resource id
 */
int li_find_genre(lua_State* l);

/**
 * @brief Creates a new entity, initially memory-managed by Lua. Note that
 * such entities (memory managed by Lua) can only have one entity handle
 * userdata.
 * 1: Archetype
 */
int li_new_entity(lua_State* l);

/**
 * @brief Spawns an entity, throwing an error if this is not possible
 * 1: Entity
 */
int li_spawn_entity(lua_State* l);

/**
 * @brief Kills an entity, throwing an error if this is not possible
 * 1: Entity
 */
int li_kill_entity(lua_State* l);

/**
 * @brief Manually deletes an entity. Note that this is not required, and Lua
 * can still gc the entity manually for you. Does not invalidate the provided
 * Entity_Handle, but it does ensure that the handle reports correctly that
 * its entity has been deleted.
 * 1: Entity 
 */
int li_delete_entity(lua_State* l);

} // namespace LI
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_LUAINTERF_HPP
