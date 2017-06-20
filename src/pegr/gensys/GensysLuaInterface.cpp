#include "pegr/gensys/GensysLuaInterface.hpp"

#include <stdexcept>
#include <sstream>
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
    
    if (lua_type(l, idx) != LUA_TTABLE) {
        std::string str_debug = 
                Script::Helper::to_string(idx, 
                        Script::Helper::GENERIC_TO_STRING_DEFAULT);
        assert(original_size == lua_gettop(l)); // Balance sanity
        std::stringstream ss;
        ss << "Invalid primitive constructor: " << str_debug;
        throw std::runtime_error(ss.str());
    }
    
    std::size_t strlen;
    const char* strdata;
    
    lua_rawgeti(l, idx, 1); // First member of the table should be type string
    strdata = lua_tolstring(l, -1, &strlen);
    if (!strdata) {
        std::string str_debug = 
                Script::Helper::to_string(-1, 
                        Script::Helper::GENERIC_TO_STRING_DEFAULT);
        lua_pop(l, 1); // Remove rawgeti
        assert(original_size == lua_gettop(l)); // Balance sanity
        std::stringstream ss;
        ss << "Invalid type: " << str_debug;
        throw std::runtime_error(ss.str());
    }
    // TODO: light userdata instead of strings
    lua_pop(l, 1); // Remove type string
    std::string type_name(strdata, strlen);
    
    Interm::Prim ret_val;
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
        assert(original_size == lua_gettop(l)); // Balance sanity
        std::stringstream ss;
        ss << "Unknown type: " << type_name;
        throw std::runtime_error(ss.str());
    }
    
    lua_rawgeti(l, idx, 2); // Second member should be value
    switch (ret_val.get_type()) {
        // TODO: check validity of number type
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
            try {
                ret_val.set_string(Script::Helper::to_string(-1));
            }
            catch (std::runtime_error e) {
                lua_pop(l, 1); // Remove rawgeti
                assert(original_size == lua_gettop(l)); // Balance sanity
                std::stringstream ss;
                ss << "Error while parsing primitive string: " << e.what();
                throw std::runtime_error(ss.str());
            }
            break;
        }
        case Interm::Prim::Type::FUNC: {
            if (lua_type(l, -1) != LUA_TFUNCTION) {
                std::string str_debug = 
                        Script::Helper::to_string(-1, 
                                Script::Helper::GENERIC_TO_STRING_DEFAULT);
                lua_pop(l, 1); // Remove rawgeti
                assert(original_size == lua_gettop(l)); // Balance sanity
                std::stringstream ss;
                ss << "Invalid function: " << str_debug;
                throw std::runtime_error(ss.str());
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

Interm::Comp_Def* translate_component_definition(int table_idx) {
    lua_State* l = Script::get_lua_state();
    int original_size; assert(original_size = lua_gettop(l)); // Balance sanity
    
    std::size_t strlen;
    const char* strdata;
    
    Interm::Comp_Def comp_def;
    
    Script::Helper::for_pairs(-1, [&]()->bool {
        {
            lua_pushvalue(l, -2); // Copy of key
            Script::Pop_Guard pop_guard(1);
            strdata = lua_tolstring(l, -1, &strlen);
            if (!strdata) {
                std::string str_debug = 
                        Script::Helper::to_string(-1, 
                                Script::Helper::GENERIC_TO_STRING_DEFAULT);
                std::stringstream ss;
                ss << "Invalid key in components cstr table" << str_debug;
                throw std::runtime_error(ss.str());
            }
        }
        Interm::Symbol symbol(strdata, strlen);
        Interm::Prim value;
        try {
            value = translate_primitive(-1);
        }
        catch (std::runtime_error e) {
            std::stringstream ss;
            ss << "Error while constructing primitive: " << e.what();
            throw std::runtime_error(ss.str());
        }
        // Check that no symbol is duplicated (possible through integer keys)
        if (comp_def.m_members.find(symbol) != comp_def.m_members.end()) {
            throw std::runtime_error("Duplicate symbol");
        }
        comp_def.m_members[symbol] = value;
        return true;
    }, false);
    
    assert(original_size == lua_gettop(l)); // Balance sanity
    return new Interm::Comp_Def(std::move(comp_def));
}

Interm::Arche* translate_archetype(int table_idx) {
    lua_State* l = Script::get_lua_state();
    int original_size; assert(original_size = lua_gettop(l)); // Balance sanity
    
    std::size_t strlen;
    const char* strdata;
    
    Interm::Arche arche;
    Script::Helper::for_pairs(-1, [&]()->bool {
        return true;
    }, false);
    
    assert(original_size == lua_gettop(l)); // Balance sanity
    return new Interm::Arche(std::move(arche));
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
        std::string id(strdata, strlen);
        Logger::log()->info("Parsing comp: %v", id);
        Interm::Comp_Def* obj = translate_component_definition(-1);
        Gensys::stage_component(id.c_str(), obj);
        return true;
    }, false);
    lua_pop(l, 1);
    
    Script::push_reference(m_working_archetypes);
    Script::Helper::for_pairs(-1, [&]()->bool {
        lua_pushvalue(l, -2); // Make a copy of the key
        strdata = lua_tolstring(l, -1, &strlen);
        if (!strdata) {
            Logger::log()->warn("Invalid key in working archetype table");
            lua_pop(l, 2);
            return true;
        }
        lua_pop(l, 1);
        std::string id(strdata, strlen);
        Logger::log()->info("Parsing archetype: %v", id);
        Interm::Arche* obj = translate_archetype(-1);
        Gensys::stage_archetype(id.c_str(), obj);
        return true;
    }, false);
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
