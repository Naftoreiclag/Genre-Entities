#include "pegr/gensys/GensysLuaInterface.hpp"

#include <stdexcept>
#include <sstream>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
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
        std::stringstream sss;
        sss << "Invalid primitive constructor: "
            << Script::Helper::to_string(idx, 
                    Script::Helper::GENERIC_TO_STRING_DEFAULT);
        throw std::runtime_error(sss.str());
    }
    
    std::size_t strlen;
    const char* strdata;
    
    lua_rawgeti(l, idx, 1); // First member of the table should be type string
    Script::Pop_Guard pop_guard(1);
    strdata = lua_tolstring(l, -1, &strlen);
    if (!strdata) {
        std::stringstream sss;
        sss << "Invalid type: "
            << Script::Helper::to_string(-1, 
                    Script::Helper::GENERIC_TO_STRING_DEFAULT);
        throw std::runtime_error(sss.str());
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
    else {
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
                    std::stringstream sss;
                    sss << "Cannot parse string value for primitive: "
                        << e.what();
                    throw std::runtime_error(sss.str());
                }
                break;
            }
            case Interm::Prim::Type::FUNC: {
                if (lua_type(l, -1) != LUA_TFUNCTION) {
                    std::stringstream sss;
                    sss << "Invalid function: "
                        << Script::Helper::to_string(-1, 
                                    Script::Helper::GENERIC_TO_STRING_DEFAULT);
                    throw std::runtime_error(sss.str());
                }
                lua_pushvalue(l, -1);
                ret_val.set_function(
                        Script::make_shared(Script::grab_reference()));
                break;
            }
            default: {
                assert(false && "Unhandled primitive type");
            }
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
                        "Invalid key in component table");
        Interm::Prim value;
        try {
            value = translate_primitive(-1);
        }
        catch (std::runtime_error e) {
            std::stringstream sss;
            sss << "Cannot parse primitive for member \""
                << symbol << "\": " << e.what();
            throw std::runtime_error(sss.str());
        }
        // Check that no symbol is duplicated (possible through integer keys)
        if (comp_def.m_members.find(symbol) != comp_def.m_members.end()) {
            std::stringstream sss;
            sss << "Symbol \"" << symbol
                << "\" occurs multiple times in component table";
            throw std::runtime_error(sss.str());
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
        throw std::runtime_error("Key __is cannot be nil!");
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
        sss << "Cannot find a component with id [" << comp_id << ']';
        throw std::runtime_error(sss.str());
    }
    
    Script::Helper::for_pairs(table_idx, [&]()->bool {
        Interm::Symbol symbol = 
                assert_table_key_to_string(-2, 
                        "Invalid key in archetype table: ");
        if (symbol == "__is") {
            return true;
        }
        
        // Check that no symbol is duplicated (possible through integer keys)
        if (implement.m_values.find(symbol) != implement.m_values.end()) {
            std::stringstream sss;
            sss << "Symbol \"" << symbol
                << "\" occurs multiple times in archetype table";
            throw std::runtime_error(sss.str());
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
            sss << "Error while parsing primitive in implementation: "
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
                        "Invalid key in archetype table: ");
        Interm::Arche::Implement implement;
        try {
            implement = translate_archetype_implementation(-1);
            implement.m_error_msg_name = symbol;
        } catch (std::runtime_error e) {
            std::stringstream sss;
            sss << "Cannot parse archetype implementation \""
                << symbol << "\": "
                << e.what();
            throw std::runtime_error(sss.str());
        }
        // Check that no symbol is duplicated (possible through integer keys)
        if (arche.m_implements.find(symbol) != arche.m_implements.end()) {
            std::stringstream sss;
            sss << "Symbol \"" << symbol
                << "\" occurs multiple times in archetype table";
            throw std::runtime_error(sss.str());
        }
        arche.m_implements[symbol] = std::move(implement);
        
        return true;
    }, false);
    
    return new Interm::Arche(std::move(arche));
}

void assert_pattern_source_has_symbol(
        const Interm::Genre::Pattern& pattern, const Interm::Symbol& symbol) {
    switch (pattern.m_type) {
        case Interm::Genre::Pattern::Type::FROM_COMP: {
            assert(pattern.m_from_component);
            if (pattern.m_from_component->m_members.find(symbol)
                    == pattern.m_from_component->m_members.end()) {
                std::stringstream sss;
                sss << "Alias pattern references member \""
                    << symbol
                    << "\" which does not exist in component \""
                    << pattern.m_from_component->m_error_msg_name
                    << '"';
                throw std::runtime_error(sss.str());
            }
            break;
        }
        case Interm::Genre::Pattern::Type::FROM_GENRE: {
            assert(pattern.m_from_genre);
            if (pattern.m_from_genre->m_interface.find(symbol)
                    == pattern.m_from_genre->m_interface.end()) {
                std::stringstream sss;
                sss << "Alias pattern references member \""
                    << symbol
                    << "\" which does not exist in genre \""
                    << pattern.m_from_genre->m_error_msg_name
                    << '"';
                throw std::runtime_error(sss.str());
            }
            break;
        }
        default: {
            assert(false);
            break;
        }
    }
}

Interm::Genre::Pattern translate_genre_pattern(int value_idx) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    
    int value_type = lua_type(l, value_idx);
    switch (value_type) {
        
        // Pattern is a table
        case LUA_TTABLE: {
            Interm::Genre::Pattern pattern;
            lua_getfield(l, value_idx, "__from");
            Script::Pop_Guard pop_guard(1);
            if (lua_isnil(l, -1)) {
                throw std::runtime_error(
                        "\"__from\" field missing from pattern table");
            }
            
            std::string id;
            try {
                id = Script::Helper::to_string(-1);
            } catch (std::runtime_error e) {
                std::stringstream sss;
                sss << "Could not retrieve string from \"__from\" field: "
                    << e.what();
                throw std::runtime_error(sss.str());
            }
            pop_guard.pop(1); // Remove __from string
            
            // Determine what we are drawing data from
            switch (Gensys::get_type(id)) {
                // Object does not exist
                case Gensys::ObjectType::NOT_FOUND: {
                    std::stringstream sss;
                    sss << "No component or genre with id ["
                        << id << ']';
                    throw std::runtime_error(sss.str());
                }
                // Object is an archetype (not allowed)
                case Gensys::ObjectType::ARCHETYPE: {
                    std::stringstream sss;
                    sss << "Genres cannot depend on the existence of "
                            "particular archetypes, such as ["
                        << id << ']';
                    throw std::runtime_error(sss.str());
                }
                // Object is a component
                case Gensys::ObjectType::COMP_DEF: {
                    pattern.m_type = 
                            Interm::Genre::Pattern::Type::FROM_COMP;
                    pattern.m_from_component = 
                            Gensys::get_staged_component(id);
                    assert(pattern.m_from_component);
                    break;
                }
                // Object is a genre
                case Gensys::ObjectType::GENRE: {
                    pattern.m_type = 
                            Interm::Genre::Pattern::Type::FROM_GENRE;
                    pattern.m_from_genre =
                            Gensys::get_staged_genre(id);
                    assert(pattern.m_from_genre);
                    break;
                }
                // All cases should have been handled already
                default: {
                    assert(false);
                    break;
                }
            }
            
            // Store and check all provided aliases
            Script::Helper::for_pairs(value_idx, [&]()->bool {
                Interm::Symbol alias_symbol = 
                        assert_table_key_to_string(-2, 
                                "Invalid key in pattern table: ");
                
                // Ignore the from key
                if (alias_symbol == "__from") {
                    return true;
                }
                
                // Check that no symbol is duplicated
                if (pattern.m_aliases.find(alias_symbol) 
                        != pattern.m_aliases.end()) {
                    std::stringstream sss;
                    sss << "Symbol \"" << alias_symbol
                        << "\" occurs multiple times in pattern table";
                    throw std::runtime_error(sss.str());
                }
                
                std::string source_symbol;
                try {
                    source_symbol = Script::Helper::to_string(-1);
                } catch (std::runtime_error e) {
                    std::stringstream sss;
                    sss << "Could not retrieve string from alias \""
                        << alias_symbol
                        << "\": "
                        << e.what();
                    throw std::runtime_error(sss.str());
                }
                
                // Ensure that the source is available in that resource
                assert_pattern_source_has_symbol(pattern, source_symbol);
                
                pattern.m_aliases[alias_symbol] = std::move(source_symbol);
                
                return true;
            }, false);
            
            
            return pattern;
        }
        
        // Pattern is a function
        case LUA_TFUNCTION: {
            Interm::Genre::Pattern pattern;
            pattern.m_type = Interm::Genre::Pattern::Type::FUNC;
            lua_pushvalue(l, value_idx);
            
            // No need to to a function type check, that was done already
            
            pattern.m_function = Script::make_shared(Script::grab_reference());
            return pattern;
        }
        
        // Pattern is a ???
        default: {
            std::stringstream sss;
            sss << "Pattern cannot be type " << lua_typename(l, value_type);
            throw std::runtime_error(sss.str());
        }
    }
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
                        "Invalid key in interface table: ");
        Interm::Prim value;
        try {
            value = translate_primitive(-1);
        }
        catch (std::runtime_error e) {
            std::stringstream sss;
            sss << "Cannot parse interface's default primitive for member \""
                << symbol << "\": " << e.what();
            throw std::runtime_error(sss.str());
        }
        // Check that no symbol is duplicated (possible through integer keys)
        if (genre.m_interface.find(symbol) != genre.m_interface.end()) {
            std::stringstream sss;
            sss << "Symbol \"" << symbol
                << "\" occurs multiple times in interface table";
            throw std::runtime_error(sss.str());
        }
        genre.m_interface[symbol] = std::move(value);
        return true;
    }, false);
    pop_guard.pop(1);
    
    lua_getfield(l, table_idx, "patterns"); // Field guaranteed to exist
    pop_guard.on_push(1);
    
    Script::Helper::for_number_pairs_sorted(-1, [&]()->bool {
        lua_Number idx = lua_tonumber(l, -2);
        try {
            Interm::Genre::Pattern pattern = translate_genre_pattern(-1);
            pattern.m_error_msg_idx = idx;
            genre.m_patterns.push_back(pattern);
        }
        catch (std::runtime_error e) {
            std::stringstream sss;
            sss << "Cannot parse pattern #" << idx << ": " << e.what();
            throw std::runtime_error(sss.str());
        }
        return true;
    }, false);
    pop_guard.pop(1);
    
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
        // This should never happen
        if (!strdata) {
            Logger::log()->warn("Invalid key in working components table");
            return true;
        }
        pop_guard2.pop(1);
        std::string id(strdata, strlen);
        try {
            Interm::Comp_Def* obj = translate_component_definition(-1);
            obj->m_error_msg_name = id;
            Gensys::stage_component(id, obj);
            Logger::log()->info("Successfully parsed compnent [%v]", id);
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("Failed to parse component [%v]: %v", 
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
            obj->m_error_msg_name = id;
            Gensys::stage_archetype(id, obj);
            Logger::log()->info("Successfully parsed archetype [%v]", id);
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("Failed to parse archetype [%v]: %v", 
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
            obj->m_error_msg_name = id;
            Gensys::stage_genre(id, obj);
            Logger::log()->info("Successfully parsed genre [%v]", id);
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("Failed to parse genre [%v]: %v", 
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
        luaL_error(l, "add_archetype is only available during setup");
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
        luaL_error(l, "edit_archetype is only available during setup");
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
        luaL_error(l, "find_archetype is only available during execution");
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

/**
 * @brief Ensures that all invariants are satisfied for a genre table
 * The table is at position -1 on the stack
 * @param l
 */
void fix_genre_table(lua_State* l) {
    assert_balance(0);
    for (const char* key : {"interface", "patterns"}) {
        lua_getfield(l, -1, key);
        if (!lua_istable(l, -1)) {
            lua_pop(l, 1);
            lua_newtable(l);
            lua_setfield(l, -2, key);
        }
        lua_pop(l, 1);
    }
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
    
    fix_genre_table(l);
    
    lua_settable(l, 3);
    return 0;
}

int entity_new(lua_State* l) {
    
}

} // namespace LI
} // namespace Gensys
} // namespace pegr
