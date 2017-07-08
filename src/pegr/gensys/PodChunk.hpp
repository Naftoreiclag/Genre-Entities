#ifndef PEGR_GENSYS_PODCHUNK_HPP
#define PEGR_GENSYS_PODCHUNK_HPP

#include <cstddef>
#include <cstdint>

namespace pegr {
namespace Gensys {

class Pod_Chunk_Ptr {
public:
    /**
     * @brief Constructor. You should not directly call this in most
     * circumstances. Instead, use the factory function new_pod_chunk()
     * declared below. This constructor does not assume deletion responsibility.
     */
    Pod_Chunk_Ptr(void* chunk);

    /**
     * @brief Returns the memory address in the chunk with the given offset,
     * aligned to store values of type "T"
     * @param off Offset in bytes
     * @return Pointer to properly aligned address
     */
    template <typename T>
    void* get_aligned(std::size_t off) {
        assert(off % sizeof(T) == 0);
        return &(static_cast<T*>(m_chunk)[off / sizeof(T)]);
    }

    /**
     * @brief Interprets the data at the given offset in the chunk as a "T" and
     * returns the bytes interpreted as a "T"
     * @param off Offset in bytes
     * @return "T" representation of the data
     */
    template <typename T>
    T get_value(std::size_t off) {
        if (sizeof(T) <= 1) {
            return *static_cast<T*>(get_aligned<int8_t>(off));
        } else if (sizeof(T) <= 2) {
            return *static_cast<T*>(get_aligned<int16_t>(off));
        } else if (sizeof(T) <= 4) {
            return *static_cast<T*>(get_aligned<int32_t>(off));
        } else {
            return *static_cast<T*>(get_aligned<int64_t>(off));
        }
    }
    
    /**
     * @brief Interprets the data at the given offset in the chunk as a "T" and
     * set the bytes interpreted as a "T" as val
     * @param off Offset in bytes
     * @param val The value to set the bytes
     */
    template <typename T>
    void set_value(std::size_t off, T val) {
        if (sizeof(T) <= 1) {
            *static_cast<T*>(get_aligned<int8_t>(off)) = val;
        } else if (sizeof(T) <= 2) {
            *static_cast<T*>(get_aligned<int16_t>(off)) = val;
        } else if (sizeof(T) <= 4) {
            *static_cast<T*>(get_aligned<int32_t>(off)) = val;
        } else {
            *static_cast<T*>(get_aligned<int64_t>(off)) = val;
        }
    }
    
    /**
     * @return the internal chunk pointed to. Is an array of 64-bit-aligned
     * values, such as an int64_t[]
     */
    void* get_chunk();

private:
    void* m_chunk;
};

/**
 * @brief Creates a new chunk with the requested size. Rounds up to nearest 8
 * bytes (64 bit alignment).
 * @param size The requested size in bytes
 * @return "Pointer"
 */
Pod_Chunk_Ptr new_pod_chunk(std::size_t size);

/**
 * @brief Deletes a chunk that was created by new_pod_chunk()
 */
void delete_pod_chunk(Pod_Chunk_Ptr ptr);

} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_PODCHUNK_HPP
