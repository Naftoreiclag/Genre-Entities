#ifndef PEGR_GENSYS_GENSYSINTERMEDIATE_HPP
#define PEGR_GENSYS_GENSYSINTERMEDIATE_HPP

#include <cstdint>
#include <vector>
#include <map>

#include "pegr/script/Script.hpp"
#include "pegr/gensys/PodChunk.hpp"

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
    const Script::Regref_Shared& get_function() const;
    float get_f32() const;
    double get_f64() const;
    int32_t get_i32() const;
    int64_t get_i64() const;
    
    void set_string(std::string str);
    void set_function(Script::Regref_Shared func);
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
    
    // TODO: add to union
    std::string m_str;
    Script::Regref_Shared m_func;
    
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

struct Comp_Def {
    // Used only in error messages
    std::string m_error_msg_name;
    
    // Named members with primitive values
    std::map<Symbol, Prim> m_members;
    
    // --- For compiling stage ---
    
    // Finding the offsets in this chunk
    std::map<Symbol, size_t> m_compiled_offsets;
    
    // Chunk holding preconstructed values
    Pod::Chunk_Ptr m_compiled_chunk;
};

struct Arche {
    // Used only in error messages
    std::string m_error_msg_name;
    
    struct Implement {
        std::string m_error_msg_name;
        
        const Comp_Def* m_component;
        std::map<Symbol, Prim> m_values;
    };
    
    // Key: symbol, Value: implementations for the component's members
    std::map<Symbol, Implement> m_implements;
    
    // --- For compiling stage ---
    
    // Chunk holding preconstructed values
    Pod::Chunk_Ptr m_compiled_chunk;
};

struct Genre {
    // Used only in error messages
    std::string m_error_msg_name;
    
    std::map<Symbol, Prim> m_interface;
    struct Pattern {
        // Used only in error messages
        double m_error_msg_idx;
        
        enum struct Type {
            FROM_COMP,
            FROM_GENRE,
            FUNC
        };
        
        // TODO: unionize?
        Type m_type;
        
        Script::Regref_Shared m_function;
        Comp_Def* m_from_component;
        Genre* m_from_genre;
        std::map<Symbol, Symbol> m_aliases;
    };
    
    std::vector<Pattern> m_patterns;
};

} // namespace Interm
} // namespace Gensys
} // namespace pegr


#endif // PEGR_GENSYS_GENSYSINTERMEDIATE_HPP
