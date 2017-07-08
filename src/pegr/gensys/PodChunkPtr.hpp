#ifndef PEGR_GENSYS_PODCHUNKPTR_HPP
#define PEGR_GENSYS_PODCHUNKPTR_HPP

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
     * aligned to store 32-bit values.
     * @param off Offset in bytes
     * @return Pointer to 4-byte aligned address
     */
    void* get_aligned_32(std::size_t off);
    
    /**
     * @brief Returns the memory address in the chunk with the given offset,
     * aligned to store 64-bit values.
     * @param off Offset in bytes
     * @return Pointer to 8-byte aligned address
     */
    void* get_aligned_64(std::size_t off);

    /**
     * @brief Interprets the data at the given offset in the chunk as a 32-bit
     * integer.
     * @param off Offset in bytes
     * @return 32-bit integer representation of the data
     */
    int32_t get_int32(std::size_t off);

    /**
     * @brief Interprets the data at the given offset in the chunk as a 64-bit
     * integer.
     * @param off Offset in bytes
     * @return 64-bit integer representation of the data
     */
    int64_t get_int64(std::size_t off);

    /**
     * @brief Interprets the data at the given offset in the chunk as an "int".
     * @param off Offset in bytes
     * @return float "int" of the data
     */
    int get_int(std::size_t off);

    /**
     * @brief Interprets the data at the given offset in the chunk as a float.
     * @param off Offset in bytes
     * @return float representation of the data
     */
    float get_float(std::size_t off);

    /**
     * @brief Interprets the data at the given offset in the chunk as a double.
     * @param off Offset in bytes
     * @return double representation of the data
     */
    double get_double(std::size_t off);
    
    void set_int32(std::size_t off, int32_t val);
    void set_int64(std::size_t off, int64_t val);
    void set_int(std::size_t off, int val);
    void set_float(std::size_t off, float val);
    void set_double(std::size_t off, double val);
    
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

#endif // PEGR_GENSYS_PODCHUNKPTR_HPP
