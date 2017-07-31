#include "pegr/gensys/Lua_Interf.hpp"

#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include <map>

#include "pegr/debug/Debug_Macros.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/gensys/Compiler.hpp"
#include "pegr/script/Script_Helper.hpp"
#include "pegr/gensys/Gensys.hpp"
#include "pegr/script/Lua_Interf_Util.hpp"
#include "pegr/util/Algs.hpp"

namespace pegr {
namespace Gensys {
namespace LI {

Script::Regref n_working_archetypes = LUA_REFNIL;
Script::Regref n_working_genres = LUA_REFNIL;
Script::Regref n_working_components = LUA_REFNIL;

void initialize_working_tables(lua_State* l) {
    assert_balance(0);
    lua_newtable(l);
    n_working_archetypes = Script::grab_reference();
    lua_newtable(l);
    n_working_genres = Script::grab_reference();
    lua_newtable(l);
    n_working_components = Script::grab_reference();
}

void cleanup_working_tables(lua_State* l) {
    assert_balance(0);
    Script::drop_reference(n_working_archetypes);
    n_working_archetypes = LUA_REFNIL;
    Script::drop_reference(n_working_components);
    n_working_components = LUA_REFNIL;
    Script::drop_reference(n_working_genres);
    n_working_genres = LUA_REFNIL;
}

// (These are defined in Lua_Interf_Runtime.cpp)
void initialize_userdata_metatables(lua_State* l);
void cleanup_userdata_metatables(lua_State* l);

const luaL_Reg n_setup_api_safe[] = {
    {"add_archetype", li_add_archetype},
    {"add_genre", li_add_genre},
    {"add_component", li_add_component},
    
    {"find_component", li_find_comp},
    {"find_archetype", li_find_archetype},
    {"new_entity", li_new_entity},
    {"delete_entity", li_delete_entity},
    
    // End of the list
    {nullptr, nullptr}
};

void initialize_expose_global_functions(lua_State* l) {
    assert_balance(0);
    Script::multi_expose_c_functions(n_setup_api_safe);
}

void initialize() {
    assert(Script::is_initialized());
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    initialize_working_tables(l);
    initialize_userdata_metatables(l);
    initialize_expose_global_functions(l);
}

void clear() {
    assert(Script::is_initialized());
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    cleanup_working_tables(l);
    initialize_working_tables(l);
}

void cleanup() {
    lua_State* l = Script::get_lua_state();
    cleanup_working_tables(l);
    cleanup_userdata_metatables(l);
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

Interm::Prim parse_primitive(int idx, Interm::Prim::Type required_t) {
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
            case Interm::Prim::Type::F32: 
            case Interm::Prim::Type::F64: 
            case Interm::Prim::Type::I32: 
            case Interm::Prim::Type::I64: {
                // Check that the type is either string or number
                int val_type = lua_type(l, -1);
                if (val_type != LUA_TNUMBER && val_type != LUA_TSTRING) {
                    std::stringstream sss;
                    sss << "Numeric primitive value cannot be type "
                        << lua_typename(l, val_type)
                        << ", (\""
                        << Script::Helper::to_string(-1, 
                                    Script::Helper::GENERIC_TO_STRING_DEFAULT)
                        << "\")";
                    throw std::runtime_error(sss.str());
                }
                
                // Try convert the value into a number
                lua_Number val;
                if (!Script::Helper::to_number_safe(-1, val)) {
                    std::stringstream sss;
                    sss << "Cannot convert value to number "
                        << " (\""
                        << Script::Helper::to_string(-1, 
                                    Script::Helper::GENERIC_TO_STRING_DEFAULT)
                        << "\")";
                    throw std::runtime_error(sss.str());
                }
                
                switch (ret_val.get_type()) {
                    case Interm::Prim::Type::F32: {
                        ret_val.set_f32(val);
                        break;
                    }
                    case Interm::Prim::Type::F64: {
                        ret_val.set_f64(val);
                        break;
                    }
                    case Interm::Prim::Type::I32: {
                        ret_val.set_i32(val);
                        break;
                    }
                    case Interm::Prim::Type::I64: {
                        ret_val.set_i64(val);
                        break;
                    }
                    // Should not be possible since the type must be one
                    // of the above cases
                    default: {
                        assert(false);
                        break;
                    }
                }
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
                int val_type = lua_type(l, -1);
                if (val_type != LUA_TFUNCTION) {
                    std::stringstream sss;
                    sss << "Function primitive value cannot be type "
                        << lua_typename(l, val_type)
                        << ", (\""
                        << Script::Helper::to_string(-1, 
                                    Script::Helper::GENERIC_TO_STRING_DEFAULT)
                        << "\")";
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

std::unique_ptr<Interm::Comp> parse_component_definition(int table_idx) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    table_idx = Script::absolute_idx(table_idx);
    
    std::unique_ptr<Interm::Comp> comp_def = 
            std::make_unique<Interm::Comp>();
    
    Script::Helper::for_pairs(table_idx, [&]()->bool {
        Interm::Symbol symbol = 
                assert_table_key_to_string(-2, 
                        "Invalid key in component table");
        Interm::Prim value;
        try {
            value = parse_primitive(-1);
        }
        catch (std::runtime_error e) {
            std::stringstream sss;
            sss << "Cannot parse primitive for member \""
                << symbol << "\": " << e.what();
            throw std::runtime_error(sss.str());
        }
        // Check that no symbol is duplicated (possible through integer keys)
        if (comp_def->m_members.find(symbol) != comp_def->m_members.end()) {
            std::stringstream sss;
            sss << "Symbol \"" << symbol
                << "\" occurs multiple times in component table";
            throw std::runtime_error(sss.str());
        }
        comp_def->m_members[symbol] = std::move(value);
        return true;
    }, false);
    
    return comp_def;
}

Interm::Arche::Implement parse_archetype_implementation(int table_idx) {
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
    
    implement.m_component = Compiler::get_staged_component(comp_id);
    
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
                    parse_primitive(-1, prim_def.get_type());
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

std::unique_ptr<Interm::Arche> parse_archetype(int table_idx) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    table_idx = Script::absolute_idx(table_idx);
    
    std::unique_ptr<Interm::Arche> arche
            = std::make_unique<Interm::Arche>();
    
    Script::Helper::for_pairs(table_idx, [&]()->bool {
        Interm::Symbol symbol = 
                assert_table_key_to_string(-2, 
                        "Invalid key in archetype table: ");
        Interm::Arche::Implement implement;
        try {
            implement = parse_archetype_implementation(-1);
            implement.m_error_msg_name = symbol;
        } catch (std::runtime_error e) {
            std::stringstream sss;
            sss << "Cannot parse archetype implementation \""
                << symbol << "\": "
                << e.what();
            throw std::runtime_error(sss.str());
        }
        // Check that no symbol is duplicated (possible through integer keys)
        if (arche->m_implements.find(symbol) != arche->m_implements.end()) {
            std::stringstream sss;
            sss << "Symbol \"" << symbol
                << "\" occurs multiple times in archetype table";
            throw std::runtime_error(sss.str());
        }
        arche->m_implements[symbol] = std::move(implement);
        
        return true;
    }, false);
    
    return arche;
}

/*
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
*/

std::map<Interm::Symbol, Interm::Comp*> 
        parse_genre_pattern_matching(int value_idx) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    value_idx = Script::absolute_idx(value_idx);

    std::map<Interm::Symbol, Interm::Comp*> retval;
    if (lua_istable(l, value_idx)) {
        Script::Helper::for_pairs(value_idx, [&]()->bool {
            Interm::Symbol comp_symbol = 
                    assert_table_key_to_string(-2, 
                            "Invalid key: ");
            std::string comp_id = 
                    assert_table_key_to_string(-1, 
                            "Invalid ID: ");
            Interm::Comp* comp = Compiler::get_staged_component(comp_id);
            
            if (!comp) {
                std::stringstream sss;
                sss << "No component with id [" << comp_id << ']';
                throw std::runtime_error(sss.str());
            }
            
            // Check that no symbol is duplicated
            if (retval.find(comp_symbol) != retval.end()) {
                std::stringstream sss;
                sss << "Symbol \"" << comp_symbol
                    << "\" occurs multiple times";
                throw std::runtime_error(sss.str());
            }
            
            retval[comp_symbol] = comp;
            return true;
        }, false);
    } else if (lua_isnil(l, value_idx)) {
        // Leave matching map empty
    } else {
        std::stringstream sss;
        sss << "Incorrect type, "
            << lua_typename(l, lua_type(l, value_idx));
        throw std::runtime_error(sss.str());
    }
    return retval;
}

std::map<Interm::Symbol, Interm::Genre::Pattern::Alias> 
        parse_genre_pattern_aliases(int value_idx, 
            const std::map<Interm::Symbol, Interm::Comp*>& local_name_to_comp,
            const std::map<Interm::Symbol, Interm::Prim>& genre_interface) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    value_idx = Script::absolute_idx(value_idx);


    std::map<Interm::Symbol, Interm::Genre::Pattern::Alias> retval;
    if (lua_istable(l, value_idx)) {
        Script::Helper::for_pairs(value_idx, [&]()->bool {
            // Get the member this alias is meant to fulfill
            Interm::Symbol dest_member_symb = 
                    assert_table_key_to_string(-2, 
                            "Invalid key: ");
            
            // Verify that the destination symbol exists in the interface
            auto dest_member_iter = genre_interface.find(dest_member_symb);
            if (dest_member_iter == genre_interface.end()) {
                std::stringstream sss;
                sss << "Symbol \"" << dest_member_symb
                    << "\" not in interface";
                throw std::runtime_error(sss.str());
            }
            const Interm::Prim& dest_prim = dest_member_iter->second;
            
            // Get the reference to the data source used for that member
            std::string alias_str = 
                    assert_table_key_to_string(-1, 
                            "Invalid alias: ");
            
            // The name used in the "matching" table
            std::string local_comp_name;
            
            // The named member in the source component
            std::string source_member_symb;
            
            // Attempt to separate the alias_str into the component and member
            {
                std::size_t dot_idx = alias_str.find('.');
                if (dot_idx == std::string::npos) {
                    std::stringstream sss;
                    sss << "Could not separate component and member in alias "
                            "(missing \".\"): "
                        << alias_str;
                    throw std::runtime_error(sss.str());
                }
                local_comp_name = alias_str.substr(0, dot_idx);
                source_member_symb = alias_str.substr(dot_idx + 1);
            }
            
            // Find the comp specified by the local name
            auto local_names_iter = local_name_to_comp.find(local_comp_name);
            if (local_names_iter == local_name_to_comp.end()) {
                std::stringstream sss;
                sss << "Component \"" << local_comp_name
                    << "\" is unspecified (missing from \"matching\" table)";
                throw std::runtime_error(sss.str());
            }
            
            // The comp to pull the data from
            Interm::Comp* source_comp = local_names_iter->second;
            
            assert(source_comp);
            
            // Verify that the source member exists in the source component
            const auto source_member_iter = 
                    source_comp->m_members.find(source_member_symb);
            if (source_member_iter == source_comp->m_members.end()) {
                std::stringstream sss;
                sss << "Could not find member \""
                    << source_member_symb
                    << "\" in component \"" << source_comp->m_error_msg_name
                    << '"';
                throw std::runtime_error(sss.str());
            }
            
            /* Verify that the source and destination member prims have the
             * same type.
             */
            const Interm::Prim& source_prim = source_member_iter->second;
            if (source_prim.get_type() != dest_prim.get_type()) {
                std::stringstream sss;
                sss << "Type mismatch, expected "
                    << Interm::prim_type_to_debug_str(dest_prim.get_type())
                    << ", got "
                    << Interm::prim_type_to_debug_str(source_prim.get_type());
                throw std::runtime_error(sss.str());
            }
            
            // Construct the alias
            Interm::Genre::Pattern::Alias alias;
            alias.m_comp = source_comp;
            alias.m_member = source_member_symb;
            
            // Check that no symbol is duplicated
            if (retval.find(dest_member_symb) != retval.end()) {
                std::stringstream sss;
                sss << "Symbol \"" << dest_member_symb
                    << "\" occurs multiple times";
                throw std::runtime_error(sss.str());
            }
            
            retval[dest_member_symb] = alias;
            return true;
        }, false);
    } else if (lua_isnil(l, value_idx)) {
        // Leave matching map empty
    } else {
        std::stringstream sss;
        sss << "Incorrect type, "
            << lua_typename(l, lua_type(l, value_idx));
        throw std::runtime_error(sss.str());
    }
    return retval;
}

std::map<Interm::Symbol, Interm::Prim>
        parse_genre_pattern_static(int value_idx,
            const std::map<Interm::Symbol, Interm::Prim>& genre_interface) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    
    value_idx = Script::absolute_idx(value_idx);
    std::map<Interm::Symbol, Interm::Prim> retval;
    if (lua_istable(l, value_idx)) {
        Script::Helper::for_pairs(value_idx, [&]()->bool {
            // Get the member this alias is meant to fulfill
            Interm::Symbol dest_member_symb = 
                    assert_table_key_to_string(-2, 
                            "Invalid key: ");
            
            // Verify that the destination symbol exists in the interface
            auto dest_member_iter = genre_interface.find(dest_member_symb);
            
            if (dest_member_iter == genre_interface.end()) {
                std::stringstream sss;
                sss << "Symbol \"" << dest_member_symb
                    << "\" not in interface";
                throw std::runtime_error(sss.str());
            }
            const Interm::Prim& dest_prim = dest_member_iter->second;
            
            // Parse the source
            Interm::Prim source_prim;
            try {
                source_prim = parse_primitive(-1);
            }
            catch (std::runtime_error e) {
                std::stringstream sss;
                sss << "Cannot parse primitive for static value \""
                    << dest_member_symb << "\": " << e.what();
                throw std::runtime_error(sss.str());
            }
            
            /* Verify that the source and destination member prims have the
             * same type.
             */
            if (source_prim.get_type() != dest_prim.get_type()) {
                std::stringstream sss;
                sss << "Type mismatch, expected "
                    << Interm::prim_type_to_debug_str(dest_prim.get_type())
                    << ", got "
                    << Interm::prim_type_to_debug_str(source_prim.get_type());
                throw std::runtime_error(sss.str());
            }
            
            // Check that no symbol is duplicated
            if (retval.find(dest_member_symb) != retval.end()) {
                std::stringstream sss;
                sss << "Symbol \"" << dest_member_symb
                    << "\" occurs multiple times";
                throw std::runtime_error(sss.str());
            }
            
            retval[dest_member_symb] = source_prim;
            return true;
        }, false);
    } else if (lua_isnil(l, value_idx)) {
        // Leave matching map empty
    } else {
        std::stringstream sss;
        sss << "Cannot be type, "
            << lua_typename(l, lua_type(l, value_idx));
        throw std::runtime_error(sss.str());
    }
    
    return retval;
}

Interm::Genre::Pattern parse_genre_pattern(int value_idx,
        const std::map<Interm::Symbol, Interm::Prim>& genre_interface) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    
    value_idx = Script::absolute_idx(value_idx);
    
    if (lua_istable(l, value_idx)) {
        Interm::Genre::Pattern pattern;
        
        {
            lua_getfield(l, value_idx, "matching");
            Script::Pop_Guard pop_guard(1);
            try {
                pattern.m_matching = 
                        std::move(parse_genre_pattern_matching(-1));
            } catch (std::runtime_error e) {
                std::stringstream sss;
                sss << "Error in \"matching\" field: "
                    << e.what();
                throw std::runtime_error(sss.str());
            }
        }
        
        {
            lua_getfield(l, value_idx, "aliases");
            Script::Pop_Guard pop_guard(1);
            try {
                pattern.m_aliases = 
                        std::move(parse_genre_pattern_aliases(-1,
                                pattern.m_matching,
                                genre_interface));
            } catch (std::runtime_error e) {
                std::stringstream sss;
                sss << "Error in \"aliases\" field: "
                    << e.what();
                throw std::runtime_error(sss.str());
            }
        }
        
        {
            lua_getfield(l, value_idx, "static");
            Script::Pop_Guard pop_guard(1);
            try {
                pattern.m_static_redefine = 
                        std::move(parse_genre_pattern_static(-1,
                                genre_interface));
            } catch (std::runtime_error e) {
                std::stringstream sss;
                sss << "Error in \"static\" field: "
                    << e.what();
                throw std::runtime_error(sss.str());
            }
        }
        
        // More sanity checking:
        
        // Verify that there is no overlap between static and alias lists
        {
            std::vector<Interm::Symbol> intersect = Util::map_key_intersect(
                    pattern.m_aliases, pattern.m_static_redefine);
            if (intersect.size() > 0) {
                std::stringstream sss;
                sss << "There are "
                    << intersect.size()
                    << " interface members are present in both the "
                        "\"aliases\" and \"static\" tables: ";
                const char* delim = "(";
                for (const Interm::Symbol& symb : intersect) {
                    sss << delim << symb;
                    delim = ", ";
                }
                sss << ')';
                throw std::runtime_error(sss.str());
            }
        }
        
        return pattern;
    } else {
        std::stringstream sss;
        sss << "Cannot be type "
            << lua_typename(l, lua_type(l, value_idx));
        throw std::runtime_error(sss.str());
    }
}

std::map<Interm::Symbol, Interm::Prim> parse_genre_interface(int table_idx) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    table_idx = Script::absolute_idx(table_idx);
    std::map<Interm::Symbol, Interm::Prim> retval;
    if (lua_istable(l, table_idx)) {
        Script::Helper::for_pairs(-1, [&]()->bool {
            Interm::Symbol symbol = 
                    assert_table_key_to_string(-2, 
                            "Invalid key: ");
            Interm::Prim value;
            try {
                value = parse_primitive(-1);
            }
            catch (std::runtime_error e) {
                std::stringstream sss;
                sss << "Cannot parse primitive for member \""
                    << symbol << "\": " << e.what();
                throw std::runtime_error(sss.str());
            }
            // Check that no symbol is duplicated
            if (retval.find(symbol) != retval.end()) {
                std::stringstream sss;
                sss << "Key \"" << symbol
                    << "\" occurs multiple times";
                throw std::runtime_error(sss.str());
            }
            retval[symbol] = std::move(value);
            return true;
        }, false);
    } else if (lua_isnil(l, table_idx)) {
        // Nothing
    } else {
        std::stringstream sss;
        sss << "Cannot be type, "
            << lua_typename(l, lua_type(l, table_idx));
        throw std::runtime_error(sss.str());
    }
    return retval;
}

std::vector<Interm::Genre::Pattern> parse_genre_pattern_list(int list_idx,
        const std::map<Interm::Symbol, Interm::Prim>& genre_interface) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    list_idx = Script::absolute_idx(list_idx);
    std::vector<Interm::Genre::Pattern> retval;
    
    if (lua_istable(l, list_idx)) {
        Script::Helper::for_number_pairs_sorted(-1, [&]()->bool {
            lua_Number idx = lua_tonumber(l, -2);
            try {
                Interm::Genre::Pattern pattern = 
                        std::move(parse_genre_pattern(-1, genre_interface));
                pattern.m_error_msg_idx = idx;
                retval.emplace_back(std::move(pattern));
            }
            catch (std::runtime_error e) {
                std::stringstream sss;
                sss << "Cannot parse pattern #" << idx << ": " << e.what();
                throw std::runtime_error(sss.str());
            }
            return true;
        }, false);
    } else if (lua_isnil(l, list_idx)) {
        // Nothing
    } else {
        std::stringstream sss;
        sss << "Cannot be type, "
            << lua_typename(l, lua_type(l, list_idx));
        throw std::runtime_error(sss.str());
    }
    
    return retval;
}

std::unique_ptr<Interm::Genre> parse_genre(int table_idx) {
    assert_balance(0);
    lua_State* l = Script::get_lua_state();
    table_idx = Script::absolute_idx(table_idx);
    
    std::unique_ptr<Interm::Genre> genre = std::make_unique<Interm::Genre>();
    lua_getfield(l, table_idx, "interface"); // Field guaranteed to exist
    Script::Pop_Guard pop_guard(1);
    try {
        genre->m_interface = std::move(parse_genre_interface(-1));
    } catch (std::runtime_error e) {
        std::stringstream sss;
        sss << "Cannot parse interface: "
            << e.what();
        throw std::runtime_error(e);
    }
    pop_guard.pop(1);
    
    lua_getfield(l, table_idx, "patterns");
    pop_guard.on_push(1);
    try {
        genre->m_patterns = 
                std::move(parse_genre_pattern_list(-1, genre->m_interface));
    } catch (std::runtime_error e) {
        std::stringstream sss;
        sss << "Cannot parse pattern list: "
            << e.what();
        throw std::runtime_error(e);
    }
    pop_guard.pop(1);
    
    return genre;
}

void stage_all() {
    assert_balance(0);
    Logger::log()->info("Parsing gensys data...");
    lua_State* l = Script::get_lua_state();
    
    std::size_t strlen;
    const char* strdata;
    
    Script::push_reference(n_working_components);
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
            auto obj = parse_component_definition(-1);
            obj->m_error_msg_name = id;
            Compiler::stage_component(id, std::move(obj));
            Logger::log()->info("Successfully parsed compnent [%v]", id);
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("Failed to parse component [%v]: %v", 
                    id, e.what());
        }
        return true;
    }, false);
    pop_guard.pop(1);
    
    Script::push_reference(n_working_archetypes);
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
            auto obj = parse_archetype(-1);
            obj->m_error_msg_name = id;
            Compiler::stage_archetype(id, std::move(obj));
            Logger::log()->info("Successfully parsed archetype [%v]", id);
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("Failed to parse archetype [%v]: %v", 
                    id, e.what());
        }
        return true;
    }, false);
    pop_guard.pop(1);
    
    Script::push_reference(n_working_genres);
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
            auto obj = parse_genre(-1);
            obj->m_error_msg_name = id;
            Compiler::stage_genre(id, std::move(obj));
            Logger::log()->info("Successfully parsed genre [%v]", id);
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("Failed to parse genre [%v]: %v", 
                    id, e.what());
        }
        return true;
    }, false);
    pop_guard.pop(1);
    
    clear();
}

int li_add_component(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::MUTABLE) {
        luaL_error(l, "add_component is only available during setup");
    }
    Script::Util::generic_li_add_to_res_table(l, n_working_components);
    return 0;
}

int li_edit_component(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::MUTABLE) {
        luaL_error(l, "edit_component is only available during setup");
    }
    Script::Util::generic_li_edit_from_res_table(l, n_working_components);
    return 1;
}

int li_add_archetype(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::MUTABLE) {
        luaL_error(l, "add_archetype is only available during setup");
    }
    Script::Util::generic_li_add_to_res_table(l, n_working_archetypes);
    return 0;
}

int li_edit_archetype(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::MUTABLE) {
        luaL_error(l, "edit_archetype is only available during setup");
    }
    Script::Util::generic_li_edit_from_res_table(l, n_working_archetypes);
    return 1;
}

int li_add_genre(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::MUTABLE) {
        luaL_error(l, "add_genre is only available during setup.");
    }
    Script::Util::generic_li_add_to_res_table(l, n_working_genres);
    return 0;
}

} // namespace LI
} // namespace Gensys
} // namespace pegr
