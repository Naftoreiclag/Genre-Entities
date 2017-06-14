#include "Gensys.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>

#include "Logger.hpp"

namespace pegr {
namespace Gensys {
    
std::map<std::string, Archetype*> m_archetypes;
std::map<std::string, Comp_Def*> m_components;

Script::Regref m_working_archetypes;
Script::Regref m_working_genres;
Script::Regref m_working_components;

Script::Regref m_entity_mt;

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
    
    lua_newtable(l);
    m_entity_mt = Script::grab_reference();
}

/**
 * @brief Produces a typed primitive value from a lua table
 * @param idx the index of the input table on the stack
 * @return a primitive value produced from the lua table
 */
Prim_V parse_primitive(int idx) {
    assert(m_global_state == GlobalState::MUTABLE);
    lua_State* l = Script::get_lua_state();
    int original_size; assert(original_size = lua_gettop(l)); // Balance sanity
    
    Prim_V ret_val;
    
    if (lua_type(l, idx) != LUA_TTABLE) {
        Logger::log(Logger::WARN)
                << "Invalid primitive constructor" << std::endl;
        assert(original_size == lua_gettop(l)); // Balance sanity
        return ret_val;
    }
    
    std::size_t strlen;
    const char* strdata;
    
    lua_rawgeti(l, idx, 1); // First member of the table should be type
    strdata = lua_tolstring(l, -1, &strlen);
    if (!strdata) {
        Logger::log(Logger::WARN) << "Invalid type" << std::endl;
        lua_pop(l, 1); // Remove rawgeti
        assert(original_size == lua_gettop(l)); // Balance sanity
        return ret_val;
    }
    // TODO: light userdata instead of strings
    lua_pop(l, 1); // Remove type string
    std::string type_name(strdata, strlen);
    
    if (type_name == "f32") {
        ret_val.m_type = Prim_T::F32;
    } else if (type_name == "f64") {
        ret_val.m_type = Prim_T::F64;
    } else if (type_name == "i32") {
        ret_val.m_type = Prim_T::I32;
    } else if (type_name == "i64") {
        ret_val.m_type = Prim_T::I64;
    } else if (type_name == "str") {
        ret_val.m_type = Prim_T::STR;
    } else if (type_name == "func") {
        ret_val.m_type = Prim_T::FUNC;
    } else {
        Logger::log(Logger::WARN) << "Unknown type: " << type_name << std::endl;
        assert(original_size == lua_gettop(l)); // Balance sanity
        return ret_val;
    }
    
    lua_rawgeti(l, idx, 2); // Second member should be value
    switch (ret_val.m_type) {
        case Prim_T::F32: {
            ret_val.m_f32 = lua_tonumber(l, -1);
            break;
        }
        case Prim_T::F64: {
            ret_val.m_f64 = lua_tonumber(l, -1);
            break;
        }
        case Prim_T::I32: {
            ret_val.m_i32 = lua_tonumber(l, -1);
            break;
        }
        case Prim_T::I64: {
            ret_val.m_i64 = lua_tonumber(l, -1);
            break;
        }
        case Prim_T::STR: {
            strdata = lua_tolstring(l, -1, &strlen);
            if (!strdata) {
                Logger::log(Logger::WARN) << "Invalid string" << std::endl;
                lua_pop(l, 1); // Remove rawgeti
                assert(original_size == lua_gettop(l)); // Balance sanity
                return ret_val;
            }
            ret_val.m_str = std::string(strdata, strlen);
            break;
        }
        case Prim_T::FUNC: {
            if (lua_type(l, -1) != LUA_TFUNCTION) {
                Logger::log(Logger::WARN) << "Invalid function" << std::endl;
                lua_pop(l, 1); // Remove rawgeti
                assert(original_size == lua_gettop(l)); // Balance sanity
                return ret_val;
            }
            lua_pushvalue(l, -1);
            ret_val.m_func = Script::grab_reference();
            break;
        }
        default: {
            assert(false && "Unhandled primitive type");
        }
    }
    lua_pop(l, 1); // Remove rawgeti
    
    assert(original_size == lua_gettop(l)); // Balance sanity
    return ret_val;
}

/**
 * @brief Make a new component definition from the table at the given index
 */
Comp_Def* parse_component_definition(int table_idx) {
    assert(m_global_state == GlobalState::MUTABLE);
    lua_State* l = Script::get_lua_state();
    int original_size; assert(original_size = lua_gettop(l)); // Balance sanity
    
    std::size_t strlen;
    const char* strdata;
    
    std::map<Symbol, Prim_V> symbols;
    
    lua_pushnil(l);
    while (lua_next(l, table_idx) != 0) {
        lua_pushvalue(l, -2); // Make a copy of the key
        strdata = lua_tolstring(l, -1, &strlen);
        if (!strdata) {
            Logger::log(Logger::WARN)
                    << "Invalid key in components cstr table" << std::endl;
            lua_pop(l, 2); // Leave only the key for next iteration
            continue;
        }
        lua_pop(l, 1); // Remove the copy of the key
        Symbol symbol(strdata, strlen);
        Prim_V value = parse_primitive(-1);
        
        // Check that no symbol is duplicated (possible through integer keys)
        if (symbols.find(symbol) != symbols.end()) {
            Logger::log(Logger::WARN)
                << "Duplicate symbol" << std::endl;
            lua_pop(l, 1);
            continue;
        }
        
        symbols[symbol] = value;
    }
    
    Comp_Def* ret_val = new Comp_Def();
    ret_val->m_members = std::move(symbols);
    assert(original_size == lua_gettop(l)); // Balance sanity
    return ret_val;
}

void parse_all() {
    Logger::Out logger = Logger::log(Logger::VERBOSE);
    
    logger << "Parsing gensys data..." << std::endl;
    assert(m_global_state == GlobalState::MUTABLE);
    lua_State* l = Script::get_lua_state();
    int original_size; assert(original_size = lua_gettop(l)); // Balance sanity
    
    std::size_t strlen;
    const char* strdata;
    
    Script::push_reference(m_working_components);
    lua_pushnil(l);
    while (lua_next(l, -2) != 0) {
        lua_pushvalue(l, -2); // Make a copy of the key
        
        strdata = lua_tolstring(l, -1, &strlen);
        if (!strdata) {
            Logger::log(Logger::WARN)
                    << "Invalid key in working components table" << std::endl;
            lua_pop(l, 2); // Leave only the key for next iteration
            continue;
        }
        lua_pop(l, 1); // Remove the copy of the key
        std::string comp_id(strdata, strlen);
        logger << "Process: " << comp_id << std::endl;
        
        m_components[comp_id] = parse_component_definition(-1);
        lua_pop(l, 1); // Leave only the key for next iteration
    }
    lua_pop(l, 1); // Remove the working table
    
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
    
    auto iter = m_archetypes.find(key);
    if (iter == m_archetypes.end()) {
        // ???
    }
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
