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

#include "pegr/algs/Partition_Tracker.hpp"

#include <algorithm>

namespace pegr {
namespace Algs {

void Partition_Tracker::occupy(std::size_t offset, std::size_t size) {
    std::size_t end_range = offset + size;
    if (end_range > m_data.size()) {
        m_data.resize(end_range, 0);
    }
    std::fill(m_data.begin() + offset, m_data.begin() + end_range, 1);
}

bool Partition_Tracker::can_occupy(std::size_t offset, std::size_t size) {
    std::size_t end_range = offset + size;
    if (offset > m_data.size()) {
        return true;
    }
    auto end_iter = 
            end_range > m_data.size() ? 
                    m_data.end() : 
                    m_data.begin() + end_range;
    return std::find(m_data.begin() + offset, end_iter, 1) == end_iter;
}

std::size_t Partition_Tracker::get_minimum_size() {
    // Finds the index of the last occupied byte
    auto last_occupied = 
            std::find(m_data.rbegin(), m_data.rend(), 1).base();
    return std::distance(m_data.begin(), last_occupied + 1);
}


} // namespace Algs
} // namespace pegr

