#include "pegr/gensys/PodChunk.hpp"

namespace pegr {
namespace Gensys {

Pod_Chunk_Ptr::Pod_Chunk_Ptr(void* chunk)
: m_chunk(chunk) {}

void* Pod_Chunk_Ptr::get_chunk() {
    return m_chunk;
}
Pod_Chunk_Ptr new_pod_chunk(std::size_t size) {
    // Fewest number of int64's that can hold the requested number of bytes
    std::size_t len = (size / 8) + (size % 8 == 0 ? 0 : 1);
    int64_t* chunk = new int64_t[size];
    return Pod_Chunk_Ptr(chunk);
}
void delete_pod_chunk(Pod_Chunk_Ptr ptr) {
    delete[] static_cast<int64_t*>(ptr.get_chunk());
}
    
} // namespace Gensys
} // namespace pegr
