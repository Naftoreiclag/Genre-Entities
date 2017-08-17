/*
 *  Copyright 2017 James Fong
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef PEGR_GENSYS_PODCHUNK_HPP
#define PEGR_GENSYS_PODCHUNK_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace pegr {
namespace Gensys {
namespace Pod {

/**
 * @class Chunk_Ptr
 * @brief This class acts similar to an ordinary C pointer to an imaginary
 * "Chunk" class instance, which is really just a raw area of memory. Note that
 * this means that Chunk_Ptr has all of the dangers of ordinary C pointers, such
 * as needing to manually manage memory. For a unique_ptr<Chunk> equivalent,
 * see class Unique_Chunk_Ptr.
 * 
 * Satisfies NullablePointer requirements
 */
class Chunk_Ptr {
public:
    /**
     * @brief Constructs a nullptr
     */
    Chunk_Ptr();
    
    /**
     * @brief Constructor. You should not directly call this in most
     * circumstances. Instead, use the factory function new_pod_chunk()
     * declared below. This constructor does not assume deletion responsibility.
     */
    Chunk_Ptr(void* chunk, std::size_t size);
    
    Chunk_Ptr(const Chunk_Ptr& rhs) = default;
    Chunk_Ptr(Chunk_Ptr&& rhs) = default;
    Chunk_Ptr& operator =(const Chunk_Ptr& rhs) = default;
    Chunk_Ptr& operator =(Chunk_Ptr&& rhs) = default;
    ~Chunk_Ptr() = default;
    
    /**
     * @return If this is pointing to nothing 
     */
    bool is_nullptr() const;
    
    /**
     * @brief Make this pointer point to nothing. Calling is_nullptr()
     * immediately after calling this will return true.
     */
    void make_nullptr();

    /**
     * @brief Returns the memory address in the chunk with the given offset,
     * aligned to store values of type "T"
     * @param off Offset in bytes
     * @return Pointer to properly aligned address
     */
    template <typename T>
    void* get_aligned(std::size_t off) const {
        assert(off % sizeof(T) == 0);
        return &(static_cast<T*>(m_voidptr)[off / sizeof(T)]);
    }

    /**
     * @brief Interprets the data at the given offset in the chunk as a "T" and
     * returns the bytes interpreted as a "T"
     * @param off Offset in bytes
     * @return "T" representation of the data
     */
    template <typename T>
    T get_value(std::size_t off) const {
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
    void* get_raw() const;
    
    /**
     * @return the size of the chunk in bytes, including any final padding.
     * Guaranteed to be a multiple of 8
     */
    std::size_t get_size() const;
    
    bool operator ==(const Chunk_Ptr& rhs) const;
    bool operator !=(const Chunk_Ptr& rhs) const;
    
    explicit operator bool() const;
    
    friend bool operator ==(std::nullptr_t, const Chunk_Ptr& rhs);
    friend bool operator ==(const Chunk_Ptr& lhs, std::nullptr_t);
    friend bool operator !=(std::nullptr_t, const Chunk_Ptr& rhs);
    friend bool operator !=(const Chunk_Ptr& lhs, std::nullptr_t);

private:
    void* m_voidptr;
    std::size_t m_size;
};

struct Chunk_Ptr_Deleter {
    typedef Chunk_Ptr pointer;
    void operator ()(pointer ptr) const;
};

typedef std::unique_ptr<Chunk_Ptr, Chunk_Ptr_Deleter> Unique_Chunk_Ptr;

/**
 * @brief Creates a new chunk with the requested size. Rounds up to nearest 8
 * bytes (64 bit alignment). Chunks of size zero can be created. Such chunks
 * are not nullptr, and are not equal to each other.
 * @param size The requested size in bytes
 * @return "Pointer"
 */
Chunk_Ptr new_pod_chunk(std::size_t size);

/**
 * @brief Deletes a chunk that was created by new_pod_chunk()
 */
void delete_pod_chunk(Chunk_Ptr ptr);

/**
 * @brief Copies the data from one chunk into another
 * @param src The source chunk
 * @param src_start Offset to begin reading from (must be multiple of 8)
 * @param dest The destination chunk
 * @param dest_start Offset to begin overwriting at (must be multiple of 8)
 * @param num_bytes The number of bytes to copy (must be multiple of 8)
 */
void copy_pod_chunk(
        Chunk_Ptr src, std::size_t src_start,
        Chunk_Ptr dest, std::size_t dest_start, std::size_t num_bytes);


} // namespace Pod
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_PODCHUNK_HPP
