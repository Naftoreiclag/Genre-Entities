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

#include <cstdint>
#include <cstdint>
#include <cstdlib>
#include <sstream>

#include "pegr/except/Except.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/test/Test_Util.hpp"

namespace pegr {
namespace Test {

//@Test Memory Test
void test_0000_memory_test() {
    //                 00000000001111111111222222
    //                 01234567890123456789012345
    const char* str = "abcdefghijklmnopqrstuvwxyz";
    
    std::size_t bee = 1;
    std::size_t sea = 2;
    std::size_t eye = 8;
    std::size_t jay = 9;
    std::size_t pea = 15;
    std::size_t queue = 16;
    std::size_t are = 17;
    std::size_t tea = 19;
    std::size_t you = 20;
    std::size_t why = 24;
    
    std::stringstream sss;
    sss << *str << *(str + bee) << *(str + sea) << *(str + eye) << *(str + jay)
        << *(str + pea) << *(str + queue) << *(str + are) << *(str + tea) 
        << *(str + you) << *(str + why);
    
    std::string expected = "abcijpqrtuy";
    std::string got = sss.str();
    
    Logger::log()->info(got);
    if (expected != got) {
        std::stringstream ers;
        ers << "Expected: " << expected;
        throw Except::Runtime(ers.str());
    }
}

//@Test Pointer cast
void test_0000_ptr_cast() {
    
    int num_ints = 13;
    
    std::size_t int_offset = 0;
    std::size_t double_offset = int_offset + num_ints * sizeof(int32_t);
    std::size_t total_size = double_offset + 4 * sizeof(double);
    
    void* memory = std::calloc(1, total_size);
    void* int_memory = static_cast<void*>(
        static_cast<char*>(memory) + int_offset);
    void* double_memory = static_cast<void*>(
        static_cast<char*>(memory) + double_offset);
    
    int32_t* int_arr = static_cast<int32_t*>(int_memory);
    double* double_arr = static_cast<double*>(double_memory);
    
    int_arr[0] = 1;
    for (int32_t i = 1; i < num_ints; ++i) {
        int_arr[i] = int_arr[i - 1] * 5;
    }
    double_arr[0] = 1.0;
    for (int i = 1; i < 4; ++i) {
        double_arr[i] = double_arr[i - 1] * 2.7182818285;
    }
    
    for (int32_t i = 0; i < num_ints; ++i) {
        Logger::log()->info(int_arr[i]);
    }
    for (int32_t i = 0; i < 4; ++i) {
        Logger::log()->info(double_arr[i]);
    }
    
    std::free(static_cast<void*>(memory));
}

//@Test Assigning negative to unsigned
void test_0000_signed_unsigned() {
    std::uint32_t foo = -1;
    Logger::log()->info(foo);
    verify_equals(0xFFFFFFFF, foo);
}

} // namespace Test
} // namespace pegr
