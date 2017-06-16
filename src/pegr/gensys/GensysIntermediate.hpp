#ifndef PEGR_GENSYS_INTERM_HPP
#define PEGR_GENSYS_INTERM_HPP

#include <cstdint>
#include <map>

#include "pegr/script/Script.hpp"

namespace pegr {
namespace Gensys {
namespace Interm {

enum struct Prim_T {
    STR,
    FUNC,
    F32, F64,
    I32, I64,
    ERROR,
    ENUM_SIZE
};

struct Prim_V {
    Prim_T m_type = Prim_T::ERROR;

    std::string m_str;
    Script::Regref m_func;
    float m_f32; double m_f64;
    int32_t m_i32; int64_t m_i64;
};

typedef std::string Symbol;

struct Comp_Def {
    // Named members with primitive values
    std::map<Symbol, Prim_V> m_members;
};

struct Archetype {
    // Named members with primitive values
    typedef std::map<Symbol, Prim_V> Default_Vals;
    
    // Key: component, Value: implementations for the component's members
    std::map<Comp_Def*, Default_Vals> m_implements;
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
    
    std::map<Comp_Def*, Pattern> m_patterns;
};

} // namespace Interm
} // namespace Gensys
} // namespace pegr


#endif // PEGR_GENSYS_INTERM_HPP
