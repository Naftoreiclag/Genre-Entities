#ifndef PEGR_TEST_TESTUTIL_HPP
#define PEGR_TEST_TESTUTIL_HPP

#include <stdexcept>
#include <sstream>
#include <vector>

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
        throw std::runtime_error(sss.str());
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
        throw std::runtime_error(sss.str());
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
        throw std::runtime_error(sss.str());
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
        throw std::runtime_error(sss.str());
    }
}
    
} // namespace Test
} // namespace pegr

#endif // PEGR_TEST_TESTUTIL_HPP
