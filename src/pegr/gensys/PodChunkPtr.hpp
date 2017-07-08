#ifndef PEGR_GENSYS_PODCHUNKPTR_HPP
#define PEGR_GENSYS_PODCHUNKPTR_HPP

#include <cstddef>
#include <cstdint>

namespace pegr {
namespace Gensys {

class Pod_Chunk_Ptr {
public:
    /**
     * @brief Constructor. Does not assume deletion responsibility.
     */
    Pod_Chunk_Ptr(void* chunk);

    int32_t* get_aligned_32(std::size_t off);
    int64_t* get_aligned_64(std::size_t off);

    int32_t get_int32(std::size_t off);
    int64_t get_int64(std::size_t off);
    float get_float(std::size_t off);
    double get_double(std::size_t off);
    
    void* get_chunk();

private
    void* m_chunk;
};

Pod_Chunk_Ptr new_pod_chunk(std::size_t size);
void delete_pod_chunk(Pod_Chunk_Ptr ptr);

} // namespace Gensys
} // namespace pegr

#endif PEGR_GENSYS_PODCHUNKPTR_HPP
