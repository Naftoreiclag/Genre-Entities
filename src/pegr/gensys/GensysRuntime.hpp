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

struct Arche {
    std::size_t m_size_pures;
    int32_t* m_default_pures;
    
    std::size_t m_num_strings;
    const char* m_aggregated_default_strings;
    std::size_t* m_string_lens;
    
    std::size_t m_num_funcs;
    Script::Regref* m_func_regrefs;
};

struct Entity {
    const Arche* m_archetype;
    int32_t* m_data;
    std::string* m_instance_strings;
};

struct Genre {
    // Interpretation of archetypes, offsets into various arrays
    struct Interp {
        // Whether the value goes into the pures, functions, or strings array
        // is determined by the type of the primitive.
        std::map<Symbol, std::size_t> m_offsets;
    };
    
    // Set of Archetype pointers and interpretation
    std::map<Arche*, Interp> m_interpretations;
};

} // namespace Runtime
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_RUNTIME_GENSYSRUNTIME_HPP
