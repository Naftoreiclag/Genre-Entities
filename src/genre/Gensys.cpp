#include "genre/Gensys.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>

#include "genre/Logger.hpp"
#include "genre/GensysIntermediate.hpp"

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
    Logger::log()->info("Parsing gensys data...");
    assert(m_global_state == GlobalState::MUTABLE);
    lua_State* l = Script::get_lua_state();
    int original_size; assert(original_size = lua_gettop(l)); // Balance sanity
    /*
    
    std::size_t strlen;
    const char* strdata;
    
    Script::push_reference(m_working_components);
    lua_pushnil(l);
    while (lua_next(l, -2) != 0) {
        lua_pushvalue(l, -2); // Make a copy of the key
        
        strdata = lua_tolstring(l, -1, &strlen);
        if (!strdata) {
            Logger::log()->warn("Invalid key in working components table");
            lua_pop(l, 2); // Leave only the key for next iteration
            continue;
        }
        lua_pop(l, 1); // Remove the copy of the key
        std::string comp_id(strdata, strlen);
        Logger::log()->info("Process: %v", comp_id);
        
        m_components[comp_id] = parse_component_definition(-1);
        lua_pop(l, 1); // Leave only the key for next iteration
    }
    lua_pop(l, 1); // Remove the working table
    */
    m_global_state = GlobalState::EXECUTABLE;
    assert(original_size == lua_gettop(l)); // Balance sanity
}

int li_add_archetype(lua_State* l) {
    if (m_global_state != GlobalState::MUTABLE) {
        luaL_error(l, "add_archetype is only available during setup.");
    }
    // TODO: resolve namespace issues
    const char* key = luaL_checklstring(l, 1, nullptr);
    luaL_checktype(l, 2, LUA_TTABLE);
    Script::push_reference(m_working_archetypes);
    lua_pushvalue(l, 1);
    lua_pushvalue(l, 2);
    lua_settable(l, 3);
    return 0;
}

int li_edit_archetype(lua_State* l) {
    if (m_global_state != GlobalState::MUTABLE) {
        luaL_error(l, "edit_archetype is only available during setup.");
    }
    const char* key = luaL_checklstring(l, 1, nullptr);
    Script::push_reference(m_working_archetypes);
    lua_pushvalue(l, 1);
    lua_gettable(l, 2);
    return 1;
}

int li_find_archetype(lua_State* l) {
    if (m_global_state != GlobalState::EXECUTABLE) {
        luaL_error(l, "find_archetype is only available during execution.");
    }
    std::size_t strlen;
    const char* strdata = luaL_checklstring(l, 1, &strlen);
    std::string key(strdata, strlen);
    
    /*
    auto iter = m_archetypes.find(key);
    if (iter == m_archetypes.end()) {
        // ???
    }
    */
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

int li_edit_genre(lua_State* l) {
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

int li_edit_component(lua_State* l) {
    const char* key = luaL_checklstring(l, 1, nullptr);
    Script::push_reference(m_working_components);
    lua_pushvalue(l, 1);
    lua_gettable(l, 2);
    return 1;
}

int li_entity_new(lua_State* l) {
    
}

} // namespace Gensys
} // namespace pegr
