#ifndef PEGR_GENSYS_INTERM_HPP
#define PEGR_GENSYS_INTERM_HPP

#include <cstdint>
#include <map>

#include "pegr/script/Script.hpp"

namespace pegr {
namespace Gensys {
namespace Interm {

class Prim {
public:
    enum struct Type {
        STR,
        FUNC,
        F32, F64,
        I32, I64,
        ERROR,
        ENUM_SIZE
    };
    
    Prim();
    Prim(const Prim& other_p);
    Prim(Prim&& other_p);
    Prim& operator =(const Prim& other_p);
    Prim& operator =(Prim&& other_p);
    ~Prim();
    
    void set_type(Type type);
    Type get_type() const;
    
    bool is_error() const;
    
    const std::string& get_string() const;
    const Script::Regref_Guard& get_function() const;
    float get_f32() const;
    double get_f64() const;
    int32_t get_i32() const;
    int64_t get_i64() const;
    
    void set_string(std::string str);
    void set_function(Script::Regref func);
    void set_f32(float f32);
    void set_f64(double f64);
    void set_i32(int32_t i32);
    void set_i64(int64_t i64);

private:
    Type m_type = Type::ERROR;
    union {
        std::string m_str;
        Script::Regref_Guard m_func;
        float m_f32; double m_f64;
        int32_t m_i32; int64_t m_i64;
    };
    
    void deconstruct_current();
};

typedef std::string Symbol;

struct Comp_Def {
    // Named members with primitive values
    std::map<Symbol, Prim> m_members;
};

struct Arche {
    // Named members with primitive values
    typedef std::map<Symbol, Prim> Default_Vals;
    
    // Key: component, Value: implementations for the component's members
    std::map<Comp_Def*, Default_Vals> m_implements;
};

struct Genre {
    // The interface
    std::map<Symbol, Prim::Type> m_interface;
    
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
