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

#ifndef PEGR_TEST_TESTUTIL_HPP
#define PEGR_TEST_TESTUTIL_HPP

#include <sstream>
#include <vector>

#include "pegr/except/Except.hpp"

namespace pegr {
namespace Test {

template<typename E>
std::ostream& operator <<(std::ostream& sss, const std::vector<E>& vec) {
    const char* delim = "{";
    for (auto&& iter : vec) {
        sss << delim
            << iter;
        delim = ", ";
    }
    sss << '}';
    return sss;
}

template<typename E, typename G>
void verify_equals(E&& expected, G&& got, const char* msg = nullptr) {
    if (expected != got) {
        std::stringstream sss;
        if (msg) {
            sss << msg << ' ';
        }
        sss << "Expected: "
            << expected
            << " Got: "
            << got;
        throw Except::Runtime(sss.str());
    }
    if (got != expected) {
        std::stringstream sss;
        if (msg) {
            sss << msg << ' ';
        }
        sss << "Expected: "
            << expected
            << " Got: "
            << got
            << " (got != expected, but expected == got)";
        throw Except::Runtime(sss.str());
    }
}

template<typename E, typename G>
void verify_not_equals(E&& unexpected, G&& got, const char* msg = nullptr) {
    if (unexpected == got) {
        std::stringstream sss;
        if (msg) {
            sss << msg << ' ';
        }
        sss << "Unexpected: "
            << unexpected
            << " Got: "
            << got;
        throw Except::Runtime(sss.str());
    }
    if (got == unexpected) {
        std::stringstream sss;
        if (msg) {
            sss << msg << ' ';
        }
        sss << "Unexpected: "
            << unexpected
            << " Got: "
            << got
            << " (got == unexpected, but unexpected ! got)";
        throw Except::Runtime(sss.str());
    }
}
    
} // namespace Test
} // namespace pegr

#endif // PEGR_TEST_TESTUTIL_HPP
