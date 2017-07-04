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

/**
 * @class Entity_Handle
 * @brief The entity's "real data" doesn't exist in a single self-contained
 * struct (it can't), and so what would be called an "Entity" is really a
 * collection of relevant pointers. Therefore, these are constant-size handles
 * to be copied and passed around indefinitely. Memory management of the
 * continaed pointers is not the responsibility of this class.
 */
struct Entity_Handle {
    const Arche* m_archetype;
    
    // Instance data
    const char* m_bytes;
    
    // Array of strings that replace the defaults.
    // TODO: a bitfield for checking if the strings are actually different
    // from the defaults (save space)
    std::string* m_strings;
    std::size_t m_num_strings;
};

struct Genre {
    /* This map gives an offset into 
     */
    std::map<Symbol, Prim> m_virtual;
    std::map<Arche*, std::size_t> m_interpretations;
    
    
};

/**
 * @class Archetype_View
 * @brief Enables mutation of an entity through its archetype. This view is the
 * most permissive possible.
 */
struct Archetype_View {
    Entity_Handle m_entity;
};

} // namespace Runtime
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_RUNTIME_GENSYSRUNTIME_HPP
