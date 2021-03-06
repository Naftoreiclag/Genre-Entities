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

#ifndef PEGR_DEBUG_DEBUGASSERTLUABALANCE_HPP
#define PEGR_DEBUG_DEBUGASSERTLUABALANCE_HPP

#include <vector>

namespace pegr {
namespace Debug {

class LuaBalanceGuard {
public:
    LuaBalanceGuard(std::vector<int> deltas = {0}, 
            const char* msg = nullptr, int line = -1);
    ~LuaBalanceGuard();
private:
    int m_original_size;
    const int m_line;
    std::vector<int> m_deltas;
    const char* m_msg;
};

} // namespace Debug
} // namespace pegr

#endif // PEGR_DEBUG_DEBUGASSERTLUABALANCE_HPP
