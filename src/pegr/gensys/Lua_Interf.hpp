#ifndef PEGR_GENSYS_LUAINTERF_HPP
#define PEGR_GENSYS_LUAINTERF_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "pegr/script/Script.hpp"
#include "pegr/gensys/Interm_Types.hpp"
#include "pegr/gensys/Runtime.hpp"

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

struct Cview {
    Runtime::Entity_Handle m_ent;
    Runtime::Arche::Aggindex m_cached_aggidx;
    Runtime::Comp* m_comp;
};

struct Genview {
    Runtime::Entity_Handle m_ent;
    Runtime::Genre* m_genre;
};

Runtime::Comp** arg_require_comp(lua_State* l, int idx);
Runtime::Arche** arg_require_arche(lua_State* l, int idx);
Runtime::Entity_Handle* arg_require_entity(lua_State* l, int idx);
Cview* arg_require_cview(lua_State* l, int idx);
Genview* arg_require_genview(lua_State* l, int idx);

void push_comp_pointer(lua_State* l, Runtime::Comp* ptr);
void push_arche_pointer(lua_State* l, Runtime::Arche* ptr);
void push_genre_pointer(lua_State* l, Runtime::Genre* ptr);
void push_entity_handle(lua_State* l, Runtime::Entity_Handle ent);
void push_cview(lua_State* l, Cview ent);
void push_genview(lua_State* l, Genview ent);

std::string to_string_comp(Runtime::Comp* comp);
std::string to_string_arche(Runtime::Arche* arche);
std::string to_string_genre(Runtime::Genre* genre);
std::string to_string_entity(Runtime::Entity_Handle ent);
std::string to_string_cview(Cview cview);
std::string to_string_genview(Genview genview);

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
 * @brief Calls the deconstructor on the Genview.
 * 1: Genview (guaranteed)
 */
int li_genview_mt_gc(lua_State* l);

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
 * @brief Creates a new entity, initially memory-managed by Lua. Note that
 * such entities (memory managed by Lua) can only have one entity handle
 * userdata.
 * 1: Archetype
 */
int li_new_entity(lua_State* l);

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
