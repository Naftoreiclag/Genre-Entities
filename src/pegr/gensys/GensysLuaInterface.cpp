#include "pegr/gensys/GensysLuaInterface.hpp"

#include <stdexcept>
#include <sstream>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>

#include "pegr/debug/DebugMacros.hpp"
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
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    lua_newtable(l);
    m_working_archetypes = Script::grab_reference();
    lua_newtable(l);
    m_working_genres = Script::grab_reference();
    lua_newtable(l);
    m_working_components = Script::grab_reference();
}

/**
 * @brief Converts a modder's table keys into strings, or die trying
 * [BALANCED]
 * @param idx The index of the key
 * @param err_msg String to prefix error message with
 * @return The string
 */
std::string assert_table_key_to_string(int idx, const char* err_msg) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    lua_pushvalue(l, idx); // Copy of key
    Script::Pop_Guard pop_guard(1);
    std::size_t strlen;
    const char* strdata = lua_tolstring(l, -1, &strlen);
    if (!strdata) {
        std::string str_debug = 
                Script::Helper::to_string(-1, 
                        Script::Helper::GENERIC_TO_STRING_DEFAULT);
        std::stringstream sss;
        sss << err_msg << str_debug;
        throw std::runtime_error(sss.str());
    }
    return std::string(strdata, strlen);
}

Interm::Prim translate_primitive(int idx, Interm::Prim::Type required_t) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    idx = Script::absolute_idx(idx);
    
    if (lua_type(l, idx) != LUA_TTABLE) {
        std::string str_debug = 
                Script::Helper::to_string(idx, 
                        Script::Helper::GENERIC_TO_STRING_DEFAULT);
        std::stringstream ss;
        ss << "Invalid primitive constructor: " << str_debug;
        throw std::runtime_error(ss.str());
    }
    
    std::size_t strlen;
    const char* strdata;
    
    lua_rawgeti(l, idx, 1); // First member of the table should be type string
    Script::Pop_Guard pop_guard(1);
    strdata = lua_tolstring(l, -1, &strlen);
    if (!strdata) {
        std::string str_debug = 
                Script::Helper::to_string(-1, 
                        Script::Helper::GENERIC_TO_STRING_DEFAULT);
        std::stringstream ss;
        ss << "Invalid type: " << str_debug;
        throw std::runtime_error(ss.str());
    }
    // TODO: light userdata instead of strings
    pop_guard.pop(1);
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
        std::stringstream sss;
        sss << "Unknown type: " << type_name;
        throw std::runtime_error(sss.str());
    }
    
    if (required_t != Interm::Prim::Type::UNKNOWN
            && ret_val.get_type() != required_t) {
        std::stringstream sss;
        sss << "Type mismatch! Required: " << prim_type_to_debug_str(required_t)
            << " Found:" << prim_type_to_debug_str(ret_val.get_type());
        throw std::runtime_error(sss.str());
    }
    
    lua_rawgeti(l, idx, 2); // Second member should be value
    pop_guard.on_push(1);
    if (lua_isnil(l, -1)) {
        ret_val.set_empty();
    }
    
    
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
                std::stringstream ss;
                ss << "Cannot parse string value for primitive: " << e.what();
                throw std::runtime_error(ss.str());
            }
            break;
        }
        case Interm::Prim::Type::FUNC: {
            if (lua_type(l, -1) != LUA_TFUNCTION) {
                std::string str_debug = 
                        Script::Helper::to_string(-1, 
                                Script::Helper::GENERIC_TO_STRING_DEFAULT);
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
    
    return ret_val;
}

Interm::Comp_Def* translate_component_definition(int table_idx) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    table_idx = Script::absolute_idx(table_idx);
    
    Interm::Comp_Def comp_def;
    
    Script::Helper::for_pairs(table_idx, [&]()->bool {
        Interm::Symbol symbol = 
                assert_table_key_to_string(-2, 
                        "Invalid key in components cstr table");
        Interm::Prim value;
        try {
            value = translate_primitive(-1);
        }
        catch (std::runtime_error e) {
            std::stringstream sss;
            sss << "Cannot construct primitive: " << e.what();
            throw std::runtime_error(sss.str());
        }
        // Check that no symbol is duplicated (possible through integer keys)
        if (comp_def.m_members.find(symbol) != comp_def.m_members.end()) {
            throw std::runtime_error("Duplicate symbol");
        }
        comp_def.m_members[symbol] = std::move(value);
        return true;
    }, false);
    
    return new Interm::Comp_Def(std::move(comp_def));
}

Interm::Arche::Implement translate_archetype_implementation(int table_idx) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    table_idx = Script::absolute_idx(table_idx);
    
    Interm::Arche::Implement implement;
    
    lua_getfield(l, table_idx, "__is");
    Script::Pop_Guard pop_guard(1);
    
    if (lua_isnil(l, -1)) {
        throw std::runtime_error("__is cannot be nil!");
    }
    
    std::string comp_id;
    try {
        comp_id = Script::Helper::to_string(-1);
    } catch (std::runtime_error e) {
        std::stringstream sss;
        sss << "Cannot convert __is value to string: " << e.what();
        throw std::runtime_error(sss.str());
    }
    
    pop_guard.pop(1); // Pop __is string
    
    implement.m_component = Gensys::get_staged_component(comp_id);
    
    if (!implement.m_component) {
        std::stringstream sss;
        sss << "Cannot find a component with id: " << comp_id;
        throw std::runtime_error(sss.str());
    }
    
    Script::Helper::for_pairs(table_idx, [&]()->bool {
        Interm::Symbol symbol = 
                assert_table_key_to_string(-2, 
                        "Invalid key in component cstr table");
        if (symbol == "__is") {
            return true;
        }
        
        auto iter = implement.m_component->m_members.find(symbol);
        if (iter == implement.m_component->m_members.end()) {
            std::stringstream sss;
            sss << "Tried to assign to member \"" << symbol
                << "\" which does not exist in component \""
                << implement.m_component->m_error_msg_name
                << '"';
            throw std::runtime_error(sss.str());
        }
        
        const Interm::Prim& prim_def = iter->second;
        
        try {
            implement.m_values[symbol] = 
                    translate_primitive(-1, prim_def.get_type());
        }
        catch (std::runtime_error e) {
            std::stringstream sss;
            sss << "Error while parsing primitive in archetype cstr table: "
                << e.what();
            throw std::runtime_error(sss.str());
        }
        return true;
    }, false);
    return implement;
}

Interm::Arche* translate_archetype(int table_idx) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    table_idx = Script::absolute_idx(table_idx);
    
    Interm::Arche arche;
    Script::Helper::for_pairs(table_idx, [&]()->bool {
        Interm::Symbol symbol = 
                assert_table_key_to_string(-2, 
                        "Invalid key in archetype cstr table");
        Interm::Arche::Implement implement;
        try {
            implement = translate_archetype_implementation(-1);
        } catch (std::runtime_error e) {
            std::stringstream sss;
            sss << "Cannot parse archetype implementation cstr table: "
                << e.what();
            throw std::runtime_error(sss.str());
        }
        // Check that no symbol is duplicated (possible through integer keys)
        if (arche.m_implements.find(symbol) != arche.m_implements.end()) {
            throw std::runtime_error("Duplicate symbol");
        }
        arche.m_implements[symbol] = std::move(implement);
        
        return true;
    }, false);
    
    return new Interm::Arche(std::move(arche));
}

Interm::Genre* translate_genre(int table_idx) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    table_idx = Script::absolute_idx(table_idx);
    
    Interm::Genre genre;
    lua_getfield(l, table_idx, "interface"); // Field guaranteed to exist
    Script::Pop_Guard pop_guard(1);
    Script::Helper::for_pairs(-1, [&]()->bool {
        Interm::Symbol symbol = 
                assert_table_key_to_string(-2, 
                        "Invalid key in genre interface table");
        Interm::Prim value;
        try {
            value = translate_primitive(-1);
        }
        catch (std::runtime_error e) {
            std::stringstream sss;
            sss << "Cannot parse default interface primitive: " << e.what();
            throw std::runtime_error(sss.str());
        }
        // Check that no symbol is duplicated (possible through integer keys)
        if (genre.m_interface.find(symbol) != genre.m_interface.end()) {
            throw std::runtime_error("Duplicate symbol");
        }
        genre.m_interface[symbol] = std::move(value);
    }, false);
    pop_guard.pop(1);
    
    /*
    lua_getfield(l, table_idx, "patterns"); // Field guaranteed to exist
    pop_guard.on_push(1);
    Script::Helper::for_pairs(-1 [&]()->bool {
        
    }, false);
    pop_guard.pop(1);
    */
    
    return new Interm::Genre(std::move(genre));
}

void stage_all() {
    assert_balance(0);
    Logger::log()->info("Parsing gensys data...");
    lua_State* l = Script::get_lua_state();
    
    std::size_t strlen;
    const char* strdata;
    
    Script::push_reference(m_working_components);
    Script::Pop_Guard pop_guard(1);
    Script::Helper::for_pairs(-1, [&]()->bool {
        lua_pushvalue(l, -2); // Make a copy of the key
        Script::Pop_Guard pop_guard2(1);
        strdata = lua_tolstring(l, -1, &strlen);
        if (!strdata) {
            Logger::log()->warn("Invalid key in working components table");
            return true;
        }
        pop_guard2.pop(1);
        std::string id(strdata, strlen);
        try {
            Interm::Comp_Def* obj = translate_component_definition(-1);
            Gensys::stage_component(id, obj);
            Logger::log()->info("Successfully parsed compnent: %v", id);
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("Failed to parse component %v: %v", 
                    id, e.what());
        }
        return true;
    }, false);
    pop_guard.pop(1);
    
    Script::push_reference(m_working_archetypes);
    pop_guard.on_push(1);
    Script::Helper::for_pairs(-1, [&]()->bool {
        lua_pushvalue(l, -2); // Make a copy of the key
        Script::Pop_Guard pop_guard2(1);
        strdata = lua_tolstring(l, -1, &strlen);
        if (!strdata) {
            Logger::log()->warn("Invalid key in working archetype table");
            return true;
        }
        pop_guard2.pop(1);
        std::string id(strdata, strlen);
        try {
            Interm::Arche* obj = translate_archetype(-1);
            Gensys::stage_archetype(id, obj);
            Logger::log()->info("Successfully parsed archetype: %v", id);
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("Failed to parse archetype %v: %v", 
                    id, e.what());
        }
        return true;
    }, false);
    pop_guard.pop(1);
    
    Script::push_reference(m_working_genres);
    pop_guard.on_push(1);
    Script::Helper::for_pairs(-1, [&]()->bool {
        lua_pushvalue(l, -2); // Make a copy of the key
        Script::Pop_Guard pop_guard2(1);
        strdata = lua_tolstring(l, -1, &strlen);
        if (!strdata) {
            Logger::log()->warn("Invalid key in working genres table");
            return true;
        }
        pop_guard2.pop(1);
        std::string id(strdata, strlen);
        try {
            Interm::Genre* obj = translate_genre(-1);
            Gensys::stage_genre(id, obj);
            Logger::log()->info("Successfully parsed genre: %v", id);
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("Failed to parse genre %v: %v", 
                    id, e.what());
        }
        return true;
    }, false);
    pop_guard.pop(1);
}

void cleanup() {
    Script::drop_reference(m_working_archetypes);
    Script::drop_reference(m_working_components);
    Script::drop_reference(m_working_genres);
}

int add_component(lua_State* l) {
    luaL_checktype(l, 2, LUA_TTABLE);
    std::size_t strlen;
    const char* strdata = luaL_checklstring(l, 1, &strlen);
    std::string key(strdata, strlen);
    // TODO: resolve namespace issues in key
    Script::push_reference(m_working_components);
    lua_pushstring(l, key.c_str());
    Script::Helper::simple_deep_copy(2);
    lua_settable(l, 3);
    return 0;
}

int edit_component(lua_State* l) {
    std::size_t strlen;
    const char* strdata = luaL_checklstring(l, 1, &strlen);
    std::string key(strdata, strlen);
    // TODO: resolve namespace issues in key
    Script::push_reference(m_working_components);
    lua_pushvalue(l, 1);
    lua_gettable(l, 2);
    return 1;
}

int add_archetype(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::MUTABLE) {
        luaL_error(l, "add_archetype is only available during setup.");
    }
    luaL_checktype(l, 2, LUA_TTABLE);
    std::size_t strlen;
    const char* strdata = luaL_checklstring(l, 1, &strlen);
    std::string key(strdata, strlen);
    // TODO: resolve namespace issues in key
    Script::push_reference(m_working_archetypes);
    lua_pushstring(l, key.c_str());
    Script::Helper::simple_deep_copy(2);
    lua_settable(l, 3);
    return 0;
}

int edit_archetype(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::MUTABLE) {
        luaL_error(l, "edit_archetype is only available during setup.");
    }
    std::size_t strlen;
    const char* strdata = luaL_checklstring(l, 1, &strlen);
    std::string key(strdata, strlen);
    // TODO: resolve namespace issues in key
    Script::push_reference(m_working_archetypes);
    lua_pushstring(l, key.c_str());
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
    // TODO: resolve namespace issues in key
    
    /*
    auto iter = m_archetypes.find(key);
    if (iter == m_archetypes.end()) {
        // ???
    }
    */
}

void fix_genre_table(lua_State* l) {
    
}

int add_genre(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::MUTABLE) {
        luaL_error(l, "add_genre is only available during setup.");
    }
    luaL_checktype(l, 2, LUA_TTABLE);
    std::size_t strlen;
    const char* strdata = luaL_checklstring(l, 1, &strlen);
    std::string key(strdata, strlen);
    // TODO: resolve namespace issues in key
    Script::push_reference(m_working_genres);
    lua_pushstring(l, key.c_str());
    Script::Helper::simple_deep_copy(2);
    
    lua_settable(l, 3);
    return 0;
}

int entity_new(lua_State* l) {
    
}

} // namespace LI
} // namespace Gensys
} // namespace pegr
