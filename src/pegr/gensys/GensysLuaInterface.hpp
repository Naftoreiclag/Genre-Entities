#ifndef PEGR_GENSYS_HPP
#define PEGR_GENSYS_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "pegr/script/Script.hpp"

namespace pegr {
namespace Gensys {
namespace LI {

/**
 * @brief Initializes. Sets up workspaces for adding new components,
 * genres, archetypes, etc.
 */
void initialize();

/**
 * @brief Translates all submitted Lua tables into their intermediate forms and
 * submits to the gensys pipeline.
 */
void translate_working();

void cleanup();

int add_archetype(lua_State* l);
int edit_archetype(lua_State* l);
int find_archetype(lua_State* l);
int add_genre(lua_State* l);
int edit_genre(lua_State* l);
int add_component(lua_State* l);
int edit_component(lua_State* l);

int entity_new(lua_State* l);

} // namespace LI
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_HPP
