#ifndef PEGR_TEST_TESTUTIL_HPP
#define PEGR_TEST_TESTUTIL_HPP

#include <stdexcept>
#include <sstream>

namespace pegr {
namespace Test {

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
    
} // namespace Test
} // namespace pegr

#endif // PEGR_TEST_TESTUTIL_HPP
