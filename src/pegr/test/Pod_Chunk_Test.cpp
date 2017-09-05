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

#include <sstream>

#include "pegr/algs/Pod_Chunk.hpp"
#include "pegr/except/Except.hpp"

namespace pegr {
namespace Test {

//@Test PodChunk test
void test_0085_00_podchunk_test() {
    Algs::Podc_Ptr pcp = Algs::Podc_Ptr::new_podc(256);
    
    pcp.set_value<int32_t>(0, 12345);
    pcp.set_value<int64_t>(8, 123456789);
    pcp.set_value<float>(16, 3.14159f);
    pcp.set_value<double>(24, 2.71828);
    
    {
        int32_t got = pcp.get_value<int32_t>(0);
        if (got != 12345) {
            std::stringstream sss;
            sss << "Wrong int32 "
                << got;
            throw Except::Runtime(sss.str());
        }
    }
    {
        int64_t got = pcp.get_value<int64_t>(8);
        if (got != 123456789) {
            std::stringstream sss;
            sss << "Wrong int64 "
                << got;
            throw Except::Runtime(sss.str());
        }
    }
    {
        float got = pcp.get_value<float>(16);
        if (got != 3.14159f) {
            std::stringstream sss;
            sss << "Wrong float "
                << got;
            throw Except::Runtime(sss.str());
        }
    }
    {
        double got = pcp.get_value<double>(24);
        if (got != 2.71828) {
            std::stringstream sss;
            sss << "Wrong double "
                << got;
            throw Except::Runtime(sss.str());
        }
    }
    
    Algs::Podc_Ptr::delete_podc(pcp);
}

} // namespace Test
} // namespace pegr
