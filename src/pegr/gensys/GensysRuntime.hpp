#ifndef PEGR_GENSYS_RUNTIME_GENSYSRUNTIME_HPP
#define PEGR_GENSYS_RUNTIME_GENSYSRUNTIME_HPP

#include <cstdint>
#include <string>
#include <map>

namespace pegr {
namespace Gensys {
namespace Runtime {

/* Components no longer "exist" at this point. They have no data.
 * Components only served as a way to communicate asynchronously between authors
 * of archetypes and authors of genres.
 */

typedef std::string Symbol;

struct Prim {
    enum struct Type {
        STR,
        FUNC,
        F32, F64,
        I32, I64,
        ENUM_SIZE
    };
    Type m_type;
    
    // What this actually points to depends on the type and whether this is a
    // member of an archetype or a member of a genre
    std::size_t m_idx;
};

union Value64 {
    // There's currently no space-saving benefit for using 32-bit precision...
    int32_t m_i32;
    int64_t m_i64;
    float m_f32;
    double m_f64;
    
    Script::Regref m_func;
};

struct Arche {
    
    std::size_t m_default_64s_len;
    Value64* m_default_64s;
    
    const char* m_aggregate_default_strings;
    std::size_t* m_default_string_lens;
    
    std::map<Symbol, Prim> m_members;
};

struct Entity {
    const Arche* m_archetype;
    std::size_t m_size_bytes;
    const char* m_bytes;
    std::string* m_strings;
};

struct Genre {
    /* This map gives an offset into 
     */
    std::map<Symbol, Prim> m_virtual;
    std::map<Arche*, std::size_t> m_interpretations;
    
    
};

} // namespace Runtime
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_RUNTIME_GENSYSRUNTIME_HPP
