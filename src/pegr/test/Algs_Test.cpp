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

#include "pegr/algs/Algs.hpp"
#include "pegr/test/Test_Util.hpp"

namespace pegr {
namespace Test {

//@Test Util algs test
void test_0000_algs() {
    {
        std::map<int, int> first;
        for (int i = 1; i <= 10; ++i) {
            first[i] = i * i;
        }
        
        std::map<int, int> second;
        for (int i = 1; i <= 10; ++i) {
            second[i * 2] = i / 2;
        }
        
        std::vector<int> intersect = 
                Algs::map_key_intersect(first, second);
        
        std::vector<int> expected = {2, 4, 6, 8, 10};
        
        verify_equals(expected, intersect);
    }
    
    {
        std::map<std::string, const char*> first;
        first["a"] = "apple";
        first["b"] = "banana";
        first["c"] = "cherry";
        first["d"] = "doughnut";
        
        std::map<std::string, std::string> second;
        second["b"] = "boron";
        second["c"] = "carbon";
        second["f"] = "fluorine";
        second["h"] = "hydrogen";
        
        std::vector<std::string> intersect = 
                Algs::map_key_intersect(first, second);
        
        std::vector<std::string> expected = {"b", "c"};
        
        verify_equals(expected, intersect);
    }
}

} // namespace Test
} // namespace pegr
