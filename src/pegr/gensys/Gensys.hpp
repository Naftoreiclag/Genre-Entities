#ifndef PEGR_GENSYS_HPP
#define PEGR_GENSYS_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "pegr/script/Script.hpp"

namespace pegr {
namespace Gensys {

enum struct GlobalState {
    UNINITIALIZED,
    MUTABLE,
    EXECUTABLE,
    ENUM_SIZE
};

GlobalState get_global_state();

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

int li_add_archetype(lua_State* l);
int li_edit_archetype(lua_State* l);
int li_find_archetype(lua_State* l);
int li_add_genre(lua_State* l);
int li_edit_genre(lua_State* l);
int li_add_component(lua_State* l);
int li_edit_component(lua_State* l);

int li_entity_new(lua_State* l);

} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_HPP
