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

#include "pegr/algs/QIFU_Map.hpp"
#include "pegr/test/Test_Util.hpp"

namespace pegr {
namespace Test {
    
struct Inverness {
    int m_macbeth;
    float m_banquo;
    
    bool operator ==(const Inverness& rhs) {
        return m_banquo == rhs.m_banquo && m_macbeth == rhs.m_macbeth;
    }
    bool operator !=(const Inverness& rhs) {
        return m_banquo != rhs.m_banquo || m_macbeth != rhs.m_macbeth;
    }
};

std::ostream& operator <<(std::ostream& sss, const Inverness& val) {
    sss << '('
        << val.m_macbeth
        << ", "
        << val.m_banquo
        << ')';
    return sss;
}

typedef Algs::QIFU_Map<int, Inverness> Egg_Map;
    
//@Test QIFU_Map test
void test_0003_qifu_map_test() {
    
    Egg_Map mymap;
    
    {
        Inverness inv;
        inv.m_macbeth = 606;
        inv.m_banquo = 2.71;
        
        int handle = mymap.add(inv);
        
        verify_equals(inv, *(mymap.find(handle)));
        
        mymap.remove(handle);
        
        verify_equals(true, !mymap.find(handle));
    }
    
    std::unordered_map<int, Inverness> real;
    for (int i = 0; i < 100; ++i) {
        Inverness inv;
        inv.m_banquo = i;
        inv.m_macbeth = 1.f / ((float) (i + 1));
        
        int handle = mymap.add(inv);
        
        real[handle] = inv;
    }
        
    for (auto& iter : real) {
        int handle = iter.first;
        Inverness inv = iter.second;
        Inverness* other = mymap.find(handle);
        verify_equals(true, other != nullptr, "Could not find inverness");
        verify_equals(inv, *other, "Did not retrieve correct inverness");
    }
    
    mymap.for_each([&](Inverness* inv, int handle) {
        Inverness www;
        www.m_banquo = inv->m_banquo + 100;
        www.m_macbeth = inv->m_macbeth * 2;
        
        int handw = mymap.add(www);
        
        real[handw] = www;
        
        verify_equals(true, mymap.find(handw) != nullptr, "Deferred failed");
        
        // Test removing the current object
        if (inv->m_macbeth == 43) {
            mymap.remove(handle);
            verify_equals(true, mymap.find(handle) != nullptr, "Remove failed");
        }
        
        // Test removing an object we just added
        if (www.m_macbeth == 26) {
            mymap.remove(handw);
            verify_equals(true, mymap.find(handw) != nullptr, "Remove failed2");
        }
    });
    
    verify_equals(200, real.size());
    
    for (auto& iter : real) {
        int handle = iter.first;
        Inverness inv = iter.second;
        Inverness* other = mymap.find(handle);
        
        if (inv.m_macbeth == 43 || inv.m_macbeth == 26) {
            verify_equals(true, other == nullptr, "Should not have been found");
            continue;
        }
        
        verify_equals(true, other != nullptr, "Could not find inverness 2");
        verify_equals(inv, *other, "Did not retrieve correct inverness 2");
    }
}

} // namespace Test
} // namespace pegr
