#include "pegr/gensys/PodChunkPtr.hpp"

namespace pegr {
namespace Gensys {

Pod_Chunk_Ptr::Pod_Chunk_Ptr(void* chunk)
: m_chunk(chunk) {}

int32_t* Pod_Chunk_Ptr::get_aligned_32(std::size_t off) {
    assert(off % 4 == 0);
    return &(static_cast<int32_t*>(m_chunk)[off / 4]);
}

int64_t* Pod_Chunk_Ptr::get_aligned_64(std::size_t off) {
    assert(off % 8 == 0);
    return &(static_cast<int64_t*>(m_chunk)[off / 8]);
}

int32_t Pod_Chunk_Ptr::get_int32(std::size_t off) {
    return *static_cast<int32_t*>(get_aligned_32(off));
}
int64_t Pod_Chunk_Ptr::get_int64(std::size_t off) {
    return *static_cast<int64_t*>(get_aligned_64(off));
}
float Pod_Chunk_Ptr::get_float(std::size_t off) {
    // TODO: compile-time check whether floats are actually 32 or 64 bit.
    // Assuming 32 bit
    return *static_cast<float*>(get_aligned_32(off));
    
}
double Pod_Chunk_Ptr::get_f64(std::size_t off) {
    // TODO: compile-time check whether doubles are actually 64 bit.or greater
    // Assuming 64 bit
    return *static_cast<double*>(get_aligned_64(off));
}
int64_t* Pod_Chunk_Ptr::get_chunk() {
    return m_chunk;
}
Pod_Chunk_Ptr new_pod_chunk(std::size_t size) {
    // Fewest number of int64's that can hold the requested number of bytes
    std::size_t len = (size / 8) + (size % 8 == 0 ? 0 : 1);
    int64_t* chunk = new int64_t[size];
    return Pod_Chunk_Ptr(chunk);
}
void delete_pod_chunk(Pod_Chunk_Ptr ptr) {
    delete[] static_cast<int64_t*>(ptr.m_chunk);
}
    
} // namespace Gensys
} // namespace pegr
