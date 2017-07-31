#ifndef PEGR_UTIL_ALGS_HPP
#define PEGR_UTIL_ALGS_HPP

#include <cassert>
#include <algorithm>
#include <vector>
#include <map>

namespace pegr {
namespace Util {

template<typename K, typename U, typename V>
std::vector<K> map_key_intersect(std::map<K, U> map_a, std::map<K, V> map_b) {
    std::vector<K> list_a;
    list_a.reserve(map_a.size());
    for (auto iter : map_a) {
        list_a.push_back(iter.first);
    }
    std::vector<K> list_b;
    list_b.reserve(map_b.size());
    for (auto iter : map_b) {
        list_b.push_back(iter.first);
    }
    std::vector<K> intersect;
    auto inter_end = std::set_intersection(
            list_a.begin(), list_a.end(),
            list_b.begin(), list_b.end(),
            std::back_inserter(intersect));
    return intersect;
}

} // namespace Util
} // namespace pegr

#endif // PEGR_UTIL_ALGS_HPP
