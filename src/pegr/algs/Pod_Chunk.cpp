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

#include "pegr/algs/Pod_Chunk.hpp"

#include <algorithm>
#include <cassert>

namespace pegr {
namespace Algs {

Podc_Ptr::Podc_Ptr()
: m_voidptr(nullptr)
, m_size(0) {}

Podc_Ptr Podc_Ptr::new_podc(std::size_t req_size) {
    /* Special case if there is no size: Create a "zero-length" array
     * NOT nullptr! This ensures that no two independent return values for this 
     * function are equal!
     */
    if (req_size == 0) {
        int64_t* chunk = new int64_t[1];
        return Podc_Ptr(chunk, 0);
    }
    
    assert(sizeof(int64_t) == 8);
    
    // Fewest number of int64's that can hold the requested number of bytes
    std::size_t num_64s = (req_size / 8) + (req_size % 8 == 0 ? 0 : 1);
    int64_t* chunk = new int64_t[num_64s];
    return Podc_Ptr(chunk, num_64s * 8);
}

void Podc_Ptr::delete_podc(Podc_Ptr ptr) {
    if (ptr.is_nullptr()) return;
    delete[] static_cast<int64_t*>(ptr.get_raw());
}

void Podc_Ptr::copy_podc(
        Podc_Ptr src, std::size_t src_start,
        Podc_Ptr dest, std::size_t dest_start, std::size_t num_bytes) {

    assert(src_start % 8 == 0);
    assert(dest_start % 8 == 0);
    assert(num_bytes % 8 == 0);
    assert(src_start >= 0);
    assert(dest_start >= 0);
    assert(num_bytes >= 0);
    assert(!dest.is_nullptr());
    assert(num_bytes <= src.get_size() - src_start);
    assert(num_bytes <= dest.get_size() - dest_start);
    
    if (src.is_nullptr()) {
        assert(num_bytes == 0);
        return;
    }
    
    int64_t* src_array = static_cast<int64_t*>(src.get_raw());
    int64_t* dest_array = static_cast<int64_t*>(dest.get_raw());
    
    std::copy(
        src_array + (src_start / 8), 
        src_array + ((src_start + num_bytes) / 8), 
        dest_array + (dest_start / 8));
}

Podc_Ptr::Podc_Ptr(void* chunk, std::size_t size)
: m_voidptr(chunk)
, m_size(size) {
    assert(m_size % 8 == 0);
}

bool Podc_Ptr::is_nullptr() const {
    return m_voidptr == nullptr;
}
void Podc_Ptr::make_nullptr() {
    m_voidptr = nullptr;
    m_size = 0;
}

void* Podc_Ptr::get_raw() const {
    return m_voidptr;
}
std::size_t Podc_Ptr::get_size() const {
    return m_size;
}

bool Podc_Ptr::operator ==(const Podc_Ptr& rhs) const {
    return m_voidptr == rhs.m_voidptr;
}
bool Podc_Ptr::operator !=(const Podc_Ptr& rhs) const {
    return m_voidptr != rhs.m_voidptr;
}
Podc_Ptr::operator bool() const {
    return m_voidptr == nullptr;
}
    
bool operator ==(std::nullptr_t, const Podc_Ptr& rhs) {
    return rhs.m_voidptr == nullptr;
}
bool operator ==(const Podc_Ptr& lhs, std::nullptr_t) {
    return lhs.m_voidptr == nullptr;
}
bool operator !=(std::nullptr_t, const Podc_Ptr& rhs) {
    return rhs.m_voidptr != nullptr;
}
bool operator !=(const Podc_Ptr& lhs, std::nullptr_t) {
    return lhs.m_voidptr != nullptr;
}

void Chunk_Ptr_Deleter::operator ()(pointer ptr) const {
    Podc_Ptr::delete_podc(ptr);
}

} // namespace Algs
} // namespace pegr
