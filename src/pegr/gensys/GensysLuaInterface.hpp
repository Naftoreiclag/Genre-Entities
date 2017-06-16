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
 * @brief Initializes the gensys. Sets up workspaces for adding new components,
 * genres, archetypes, etc. Initially in editing (working) mode.
 */
void initialize();

/**
 * @brief Transitions to executable mode, turning all of the lua tables into
 * their post-process types
 */
void compile();

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
