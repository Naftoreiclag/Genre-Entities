#include "Gensys.hpp"

#include <cassert>
#include <cstddef>
#include <iostream>

namespace pegr {
namespace Gensys {

Script::Regref m_working_archetypes;
Script::Regref m_working_genres;
Script::Regref m_working_components;

GlobalState m_global_state = GlobalState::UNINITIALIZED;

GlobalState get_global_state() {
    return m_global_state;
}

void initialize() {
    assert(Script::is_initialized());
    assert(m_global_state == GlobalState::UNINITIALIZED);
    lua_State* l = Script::get_lua_state();
    lua_newtable(l);
    m_working_archetypes = Script::grab_reference();
    lua_newtable(l);
    m_working_genres = Script::grab_reference();
    lua_newtable(l);
    m_working_components = Script::grab_reference();
    m_global_state = GlobalState::MUTABLE;
}

void compile() {
    assert(m_global_state == GlobalState::MUTABLE);
    
    
    
    m_global_state = GlobalState::MUTABLE;
}

int li_add_archetype(lua_State* l) {
    const char* key = luaL_checklstring(l, 1, nullptr);
    luaL_checktype(l, 2, LUA_TTABLE);
    Script::push_reference(m_working_archetypes);
    lua_pushvalue(l, 1);
    lua_pushvalue(l, 2);
    lua_settable(l, 3);
    return 0;
}

int li_get_archetype(lua_State* l) {
    const char* key = luaL_checklstring(l, 1, nullptr);
    Script::push_reference(m_working_archetypes);
    lua_pushvalue(l, 1);
    lua_gettable(l, 2);
    return 1;
}

int li_add_genre(lua_State* l) {
    const char* key = luaL_checklstring(l, 1, nullptr);
    luaL_checktype(l, 2, LUA_TTABLE);
    Script::push_reference(m_working_genres);
    lua_pushvalue(l, 1);
    lua_pushvalue(l, 2);
    lua_settable(l, 3);
    return 0;
}

int li_get_genre(lua_State* l) {
    const char* key = luaL_checklstring(l, 1, nullptr);
    Script::push_reference(m_working_genres);
    lua_pushvalue(l, 1);
    lua_gettable(l, 2);
    return 1;
}

int li_add_component(lua_State* l) {
    const char* key = luaL_checklstring(l, 1, nullptr);
    luaL_checktype(l, 2, LUA_TTABLE);
    Script::push_reference(m_working_components);
    lua_pushvalue(l, 1);
    lua_pushvalue(l, 2);
    lua_settable(l, 3);
    return 0;
}

int li_get_component(lua_State* l) {
    const char* key = luaL_checklstring(l, 1, nullptr);
    Script::push_reference(m_working_components);
    lua_pushvalue(l, 1);
    lua_gettable(l, 2);
    return 1;
}

} // namespace Gensys
} // namespace pegr
