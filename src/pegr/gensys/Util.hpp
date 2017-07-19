#ifndef PEGR_GENSYS_UTIL_HPP
#define PEGR_GENSYS_UTIL_HPP

#include <cstddef>
#include <map>
#include <vector>

#include "pegr/gensys/Interm_Types.hpp"
#include "pegr/gensys/Pod_Chunk.hpp"
#include "pegr/logger/Logger.hpp"

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
        


template<typename K, typename V>
V* find_something(std::map<K, std::unique_ptr<V> >& map, const K& key, 
        const char* err_msg) {
    auto iter = map.find(key);
    if (iter == map.end()) {
        Logger::log()->warn(err_msg, key);
        return nullptr;
    }
    return iter->second.get();
}

template<typename K, typename V>
void erase_something(std::map<K, V>& map, const K& key, const char* err_msg) {
    auto iter = map.find(key);
    if (iter == map.end()) {
        Logger::log()->warn(err_msg, key);
        return;
    }
    map.erase(iter);
}

/**
 * @brief Removes from a vector of unique pointers by comparing their .get()
 * by value
 * @param vec
 * @param ptr
 */
template<typename T, typename V>
void vector_remove_unique_ptrs(std::vector<std::unique_ptr<T> >& vec, V& ptr) {
    vec.erase(std::remove_if(vec.begin(), vec.end(), 
            [&](const std::unique_ptr<T>& elem) -> bool {
                return elem.get() == ptr;
            }), vec.end());
}

} // namespace Util
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_UTIL_HPP
