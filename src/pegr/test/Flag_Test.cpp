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

#include "pegr/gensys/Runtime_Types.hpp"
#include "pegr/test/Test_Util.hpp"

namespace pegr {
namespace Test {
    
void verify_bitset(uint64_t bitset, uint64_t flags, bool status) {
    if (status) {
        verify_equals(flags, bitset & flags);
    } else {
        verify_equals(0, bitset & flags);
    }
}

//@Test Flag test
void test_0001_flags() {
    uint64_t flags = 0;
    
    verify_equals(0, flags);
    
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_SPAWNED, false);
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_KILLED, false);
    
    flags |= Gensys::Runtime::ENT_FLAG_KILLED;
    
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_SPAWNED, false);
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_KILLED, true);
    
    flags |= Gensys::Runtime::ENT_FLAG_SPAWNED;
    
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_SPAWNED, true);
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_KILLED, true);
    
    flags &= ~Gensys::Runtime::ENT_FLAG_SPAWNED;
    
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_SPAWNED, false);
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_KILLED, true);
    
    flags &= ~Gensys::Runtime::ENT_FLAG_KILLED;
    
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_SPAWNED, false);
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_KILLED, false);
}

} // namespace Test
} // namespace pegr
