#ifndef PEGR_GENSYS_GENSYSUTIL_HPP
#define PEGR_GENSYS_GENSYSUTIL_HPP

#include <cstddef>
#include <map>
#include <vector>

#include "pegr/gensys/GensysIntermediate.hpp"
#include "pegr/gensys/PodChunk.hpp"

namespace pegr {
namespace Gensys {
namespace Util {

class Partition_Tracker {
public:
    void occupy(std::size_t offset, std::size_t size);
    
    bool can_occupy(std::size_t offset, std::size_t size);
    
    std::size_t get_minimum_size();
private:
    std::vector<char> m_data;
};

/**
 * @brief Makes a pod chunk from a map of symbols and primitives. Uses int64_t[]
 * as the underlying data chunk.
 * @param members A map of Symbol, Prim that represents the members to pack
 * @param symbol_to_offset A reference to an empty Symbol, size_t map to store
 * the offsets into the resulting chunk
 * @return 
 */
Pod::Chunk_Ptr new_pod_chunk_from_interm_prims(
        const std::map<Interm::Symbol, Interm::Prim>& members, 
        std::map<Interm::Symbol, std::size_t>& symbol_to_offset);

/**
 * @brief Copies the pod values in the provided members map into the
 * proper positions inside of the packed pod chunk, starting at dest_offset.
 * @param members
 * @param symbol_to_offset
 * @param chunk
 * @param dest_offset
 */
void copy_named_prims_into_pod_chunk(
        const std::map<Interm::Symbol, Interm::Prim>& members, 
        const std::map<Interm::Symbol, std::size_t>& symbol_to_offset, 
        Pod::Chunk_Ptr chunk, std::size_t dest_offset);

} // namespace Util
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_GENSYSUTIL_HPP
