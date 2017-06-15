#include "genre/GensysIntermediate.hpp"

#include <cassert>

#include "genre/Logger.hpp"

namespace pegr {
namespace Gensys {
namespace Interm {

/**
 * @brief Produces a typed primitive value from a lua table
 * @param idx the index of the input table on the stack
 * @return a primitive value produced from the lua table
 */
Prim_V parse_primitive(int idx) {
    lua_State* l = Script::get_lua_state();
    int original_size; assert(original_size = lua_gettop(l)); // Balance sanity
    
    Prim_V ret_val;
    
    if (lua_type(l, idx) != LUA_TTABLE) {
        Logger::log()->warn("Invalid primitive constructor");
        assert(original_size == lua_gettop(l)); // Balance sanity
        return ret_val;
    }
    
    std::size_t strlen;
    const char* strdata;
    
    lua_rawgeti(l, idx, 1); // First member of the table should be type
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
        Logger::log()->warn("Unknown type: %v", type_name);
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
                Logger::log()->warn("Invalid string");
                lua_pop(l, 1); // Remove rawgeti
                assert(original_size == lua_gettop(l)); // Balance sanity
                return ret_val;
            }
            ret_val.m_str = std::string(strdata, strlen);
            break;
        }
        case Prim_T::FUNC: {
            if (lua_type(l, -1) != LUA_TFUNCTION) {
                Logger::log()->warn("Invalid function");
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
            Logger::log()->warn("Invalid key in components cstr table");
            lua_pop(l, 2); // Leave only the key for next iteration
            continue;
        }
        lua_pop(l, 1); // Remove the copy of the key
        Symbol symbol(strdata, strlen);
        Prim_V value = parse_primitive(-1);
        
        // Check that no symbol is duplicated (possible through integer keys)
        if (symbols.find(symbol) != symbols.end()) {
            Logger::log()->warn("Duplicate symbol");
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
    
} // namespace Interm
} // namespace Gensys
} // namespace pegr
