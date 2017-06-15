#ifndef PEGR_TEST_TESTS_HPP
#define PEGR_TEST_TESTS_HPP

#include <functional>

namespace pegr {
namespace Test {

/*### TESTS FWD ###*/

struct NamedTest {
    const char* m_name;
    std::function<bool()> m_test;
};

const NamedTest m_tests[] = {
    
    {"Testing Framework", []()->bool{return true;}},

/*### TESTS LIST ###*/
    
    // Sentinel
    {nullptr, std::function<bool()>()}
};

}
}

#endif // PEGR_TEST_TESTS_HPP
