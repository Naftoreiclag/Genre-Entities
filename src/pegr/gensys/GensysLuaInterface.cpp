#include "pegr/gensys/GensysLuaInterface.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>

#include "pegr/logger/Logger.hpp"
#include "pegr/gensys/Gensys.hpp"
#include "pegr/script/ScriptHelper.hpp"

namespace pegr {
namespace Gensys {
namespace LI {

Script::Regref m_working_archetypes;
Script::Regref m_working_genres;
Script::Regref m_working_components;

void initialize() {
    assert(Script::is_initialized());
    lua_State* l = Script::get_lua_state();
    lua_newtable(l);
    m_working_archetypes = Script::grab_reference();
    lua_newtable(l);
    m_working_genres = Script::grab_reference();
    lua_newtable(l);
    m_working_components = Script::grab_reference();
}


Interm::Prim translate_primitive(int idx) {
    lua_State* l = Script::get_lua_state();
    int original_size; assert(original_size = lua_gettop(l)); // Balance sanity
    
    idx = Script::absolute_idx(idx);
    
    Interm::Prim ret_val;
    
    if (lua_type(l, idx) != LUA_TTABLE) {
        Logger::log()->warn("Invalid primitive constructor");
        assert(original_size == lua_gettop(l)); // Balance sanity
        return ret_val;
    }
    
    std::size_t strlen;
    const char* strdata;
    
    lua_rawgeti(l, idx, 1); // First member of the table should be type string
    strdata = lua_tolstring(l, -1, &strlen);
    if (!strdata) {
        Logger::log()->warn("Invalid type");
        lua_pop(l, 1); // Remove rawgeti
        assert(original_size == lua_gettop(l)); // Balance sanity
        return ret_val;
    }
    // TODO: light userdata instead of strings
    lua_pop(l, 1); // Remove type string
    std::string type_name(strdata, strlen);
    
    if (type_name == "f32") {
        ret_val.set_type(Interm::Prim::Type::F32);
    } else if (type_name == "f64") {
        ret_val.set_type(Interm::Prim::Type::F64);
    } else if (type_name == "i32") {
        ret_val.set_type(Interm::Prim::Type::I32);
    } else if (type_name == "i64") {
        ret_val.set_type(Interm::Prim::Type::I64);
    } else if (type_name == "str") {
        ret_val.set_type(Interm::Prim::Type::STR);
    } else if (type_name == "func") {
        ret_val.set_type(Interm::Prim::Type::FUNC);
    } else {
        Logger::log()->warn("Unknown type: %v", type_name);
        assert(original_size == lua_gettop(l)); // Balance sanity
        return ret_val;
    }
    
    lua_rawgeti(l, idx, 2); // Second member should be value
    switch (ret_val.get_type()) {
        case Interm::Prim::Type::F32: {
            ret_val.set_f32(lua_tonumber(l, -1));
            break;
        }
        case Interm::Prim::Type::F64: {
            ret_val.set_f64(lua_tonumber(l, -1));
            break;
        }
        case Interm::Prim::Type::I32: {
            ret_val.set_i32(lua_tonumber(l, -1));
            break;
        }
        case Interm::Prim::Type::I64: {
            ret_val.set_i64(lua_tonumber(l, -1));
            break;
        }
        case Interm::Prim::Type::STR: {
            strdata = lua_tolstring(l, -1, &strlen);
            if (!strdata) {
                Logger::log()->warn("Invalid string");
                lua_pop(l, 1); // Remove rawgeti
                assert(original_size == lua_gettop(l)); // Balance sanity
                return ret_val;
            }
            ret_val.set_string(std::string(strdata, strlen));
            break;
        }
        case Interm::Prim::Type::FUNC: {
            if (lua_type(l, -1) != LUA_TFUNCTION) {
                Logger::log()->warn("Invalid function");
                lua_pop(l, 1); // Remove rawgeti
                assert(original_size == lua_gettop(l)); // Balance sanity
                return ret_val;
            }
            lua_pushvalue(l, -1);
            ret_val.set_function(Script::make_shared(Script::grab_reference()));
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
 * @brief Make a new component definition from the table at the given index.
 * @param table_idx The index on the main Lua stack to translate
 * @return The component, or nullptr if failure
 */
Interm::Comp_Def* translate_component_definition(int table_idx) {
    lua_State* l = Script::get_lua_state();
    int original_size; assert(original_size = lua_gettop(l)); // Balance sanity
    
    std::size_t strlen;
    const char* strdata;
    
    std::map<Interm::Symbol, Interm::Prim> symbols;
    
    Script::Helper::for_pairs(-1, [&]()->bool {
        lua_pushvalue(l, -2); // Copy of key
        strdata = lua_tolstring(l, -1, &strlen);
        if (!strdata) {
            Logger::log()->warn("Invalid key in components cstr table");
            lua_pop(l, 2); // Copy of key and table value
            return true;
        }
        lua_pop(l, 1); // Copy of key
        
        Interm::Symbol symbol(strdata, strlen);
        Interm::Prim value = translate_primitive(-1);
        
        if (value.get_type() == Interm::Prim::Type::ERROR) {
            Logger::log()->warn("Incorrect symbol");
            lua_pop(l, 1); // Table value
            return true;
        }
        
        // Check that no symbol is duplicated (possible through integer keys)
        if (symbols.find(symbol) != symbols.end()) {
            Logger::log()->warn("Duplicate symbol");
            lua_pop(l, 1); // Table value
            return true;
        }
        
        symbols[symbol] = value;
        lua_pop(l, 1); // Table value
        return true;
    }, true);
    
    Interm::Comp_Def* ret_val = new Interm::Comp_Def();
    ret_val->m_members = std::move(symbols);
    assert(original_size == lua_gettop(l)); // Balance sanity
    return ret_val;
}

void translate_working() {
    Logger::log()->info("Parsing gensys data...");
    lua_State* l = Script::get_lua_state();
    int original_size; assert(original_size = lua_gettop(l)); // Balance sanity
    
    std::size_t strlen;
    const char* strdata;
    
    Script::push_reference(m_working_components);
    Script::Helper::for_pairs(-1, [&]()->bool {
        lua_pushvalue(l, -2); // Make a copy of the key
        strdata = lua_tolstring(l, -1, &strlen);
        if (!strdata) {
            Logger::log()->warn("Invalid key in working components table");
            lua_pop(l, 2);
            return true;
        }
        lua_pop(l, 1);
        
        std::string comp_id(strdata, strlen);
        Logger::log()->info("Parsing comp: %v", comp_id);
        
        Interm::Comp_Def* obj = translate_component_definition(-1);
        
        lua_pop(l, 1);
        return true;
    }, true);
    lua_pop(l, 1);
    
    assert(original_size == lua_gettop(l)); // Balance sanity
}

void cleanup() {
    Script::drop_reference(m_working_archetypes);
    Script::drop_reference(m_working_components);
    Script::drop_reference(m_working_genres);
}

int add_archetype(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::MUTABLE) {
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

int edit_archetype(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::MUTABLE) {
        luaL_error(l, "edit_archetype is only available during setup.");
    }
    const char* key = luaL_checklstring(l, 1, nullptr);
    Script::push_reference(m_working_archetypes);
    lua_pushvalue(l, 1);
    lua_gettable(l, 2);
    return 1;
}

int find_archetype(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::EXECUTABLE) {
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

int add_genre(lua_State* l) {
    const char* key = luaL_checklstring(l, 1, nullptr);
    luaL_checktype(l, 2, LUA_TTABLE);
    Script::push_reference(m_working_genres);
    lua_pushvalue(l, 1);
    lua_pushvalue(l, 2);
    lua_settable(l, 3);
    return 0;
}

int edit_genre(lua_State* l) {
    const char* key = luaL_checklstring(l, 1, nullptr);
    Script::push_reference(m_working_genres);
    lua_pushvalue(l, 1);
    lua_gettable(l, 2);
    return 1;
}

int add_component(lua_State* l) {
    const char* key = luaL_checklstring(l, 1, nullptr);
    luaL_checktype(l, 2, LUA_TTABLE);
    Script::push_reference(m_working_components);
    lua_pushvalue(l, 1);
    lua_pushvalue(l, 2);
    lua_settable(l, 3);
    return 0;
}

int edit_component(lua_State* l) {
    const char* key = luaL_checklstring(l, 1, nullptr);
    Script::push_reference(m_working_components);
    lua_pushvalue(l, 1);
    lua_gettable(l, 2);
    return 1;
}

int entity_new(lua_State* l) {
    
}

} // namespace LI
} // namespace Gensys
} // namespace pegr
