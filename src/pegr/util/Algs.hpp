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

#ifndef PEGR_UTIL_ALGS_HPP
#define PEGR_UTIL_ALGS_HPP

#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

namespace pegr {
namespace Util {

/**
 * @brief Like std::sort, except it returns a sorted copy rather than sorting
 * in-place.
 * @param vec The vector
 * @return sorted copy of the vector
 */
template<typename T>
std::vector<T> sorted(const std::vector<T>& vec) {
    std::vector<T> retval = vec;
    std::sort(retval.begin(), retval.end());
    return retval;
}

/**
 * @brief Returns a vector of all the keys in common between the two maps
 * @param map_a
 * @param map_b
 */
template<typename K, typename U, typename V>
std::vector<K> map_key_intersect(
        const std::map<K, U>& map_a, 
        const std::map<K, V>& map_b) {
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
    assert(std::is_sorted(list_a.begin(), list_a.end()));
    assert(std::is_sorted(list_b.begin(), list_b.end()));
    std::vector<K> intersect;
    auto inter_end = std::set_intersection(
            list_a.begin(), list_a.end(),
            list_b.begin(), list_b.end(),
            std::back_inserter(intersect));
    return intersect;
}

/**
 * @brief Determines whether or not the first vector is a subset of the second.
 * @param subset Assumed sorted
 * @param superset Assumed sorted
 * @return True iff the first vector is a subset of the second.
 */
template<typename T>
bool is_subset_of_presorted(
        const std::vector<T>& subset, 
        const std::vector<T>& superset) {
    assert(std::is_sorted(subset.begin(), subset.end()));
    assert(std::is_sorted(superset.begin(), superset.end()));
    return std::includes(
            superset.begin(), superset.end(), 
            subset.begin(), subset.end());
}

/**
 * @brief Determines whether or not the first vector is a subset of the second.
 * If you know in advance that the vectors are both sorted, use the above
 * function instead.
 * @param subset
 * @param superset
 * @return True iff the first vector is a subset of the second.
 */
template<typename T>
bool is_subset_of(
        const std::vector<T>& subset, 
        const std::vector<T>& superset) {
    return is_subset_of_assuming_sorted(
            sorted(subset),
            sorted(superset));
}

} // namespace Util
} // namespace pegr

#endif // PEGR_UTIL_ALGS_HPP
