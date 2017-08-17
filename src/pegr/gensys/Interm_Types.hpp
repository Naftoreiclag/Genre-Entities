/*
 *  Copyright 2017 James Fong
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef PEGR_GENSYS_INTERMTYPES_HPP
#define PEGR_GENSYS_INTERMTYPES_HPP

#include <cstdint>
#include <map>
#include <vector>

#include "pegr/gensys/Pod_Chunk.hpp"
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
        UNKNOWN,
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
    const Script::Shared_Regref& get_function() const;
    float get_f32() const;
    double get_f64() const;
    int32_t get_i32() const;
    int64_t get_i64() const;

    void set_string(std::string str);
    void set_function(Script::Shared_Regref func);
    void set_f32(float f32);
    void set_f64(double f64);
    void set_i32(int32_t i32);
    void set_i64(int64_t i64);

    bool is_empty() const;
    void set_empty();

private:
    Type m_type = Type::UNKNOWN;
    bool m_empty;
    union {
        float m_f32; double m_f64;
        int32_t m_i32; int64_t m_i64;
    };

    std::string m_str;
    
    // We use a shared instead of unique because Prims need to be copiable..?
    Script::Shared_Regref m_func;

    void copy_from(const Prim& other_p);
    void move_from(Prim&& other_p);

    void deconstruct_current();
};

/**
 * @brief Converts the primitive type enum into a string for debug messages
 * @param t the type
 * @return the string
 */
const char* prim_type_to_debug_str(Prim::Type t);

typedef std::string Symbol;

struct Comp {
    // Used only in error messages
    std::string m_error_msg_name;

    // Named members with primitive values
    std::map<Symbol, Prim> m_members;
};

struct Arche {
    // Used only in error messages
    std::string m_error_msg_name;

    struct Implement {
        std::string m_error_msg_name;

        const Comp* m_component;
        std::map<Symbol, Prim> m_values;
    };

    // Key: symbol, Value: implementations for the component's members
    std::map<Symbol, Implement> m_implements;
};

struct Genre {
    // Used only in error messages
    std::string m_error_msg_name;

    std::map<Symbol, Prim> m_interface;
    struct Pattern {
        // Used only in error messages
        double m_error_msg_idx;
        
        struct Alias {
            Comp* m_comp;
            Symbol m_member;
        };
        
        std::map<Symbol, Comp*> m_matching;
        std::map<Symbol, Alias> m_aliases;
        std::map<Symbol, Prim> m_static_redefine;
    };

    std::vector<Pattern> m_patterns;
};

} // namespace Interm
} // namespace Gensys
} // namespace pegr


#endif // PEGR_GENSYS_INTERMTYPES_HPP
