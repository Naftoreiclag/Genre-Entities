#ifndef PEGR_GENSYS_HPP
#define PEGR_GENSYS_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "Script.hpp"

namespace pegr {
namespace Gensys {

enum struct Prim_T {
    STR,
    FUNC,
    F32, F64,
    I32, I64,
    ENUM_SIZE
};

struct Prim_V {
    Prim_T m_type;
    union {
        std::string m_str;
        Script::Regref m_func;
        float m_f32; double m_f64;
        int32_t m_i32; int64_t m_i64;
    };
};

typedef std::string Comp_Ref;
typedef std::string Symbol;

struct Comp_Def {
    // Named members with primitive types
    std::map<Symbol, Prim_T> m_members;
};

struct Archetype {
    // Named members with primitive values
    typedef std::map<Symbol, Prim_V> Default_Vals;
    
    // Key: component, Value: implementations for the component's members
    std::map<Comp_Ref, Default_Vals> m_implements;
};

struct Genre {
    // The interface
    std::map<Symbol, Prim_T> m_interface;
    
    enum Alias_T {
        RENAME,
        GETSET
    };
    
    struct Getset {
        Script::Regref m_getter;
        Script::Regref m_setter;
    };
    
    struct Alias_V {
        Alias_T m_type;
        union {
            Symbol m_symbol;
            Getset m_getset;
        };
    };
    
    // All patterns are applied
    struct Pattern {
        std::map<Symbol, Alias_V> m_aliases;
    };
    
    std::map<Comp_Ref, Pattern> m_patterns;
};

extern Script::Regref m_working_archetypes;
extern Script::Regref m_working_genres;
extern Script::Regref m_working_components;

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
void compile();

int li_add_archetype(lua_State* l);
int li_get_archetype(lua_State* l);
int li_add_genre(lua_State* l);
int li_get_genre(lua_State* l);
int li_add_component(lua_State* l);
int li_get_component(lua_State* l);

} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_HPP
