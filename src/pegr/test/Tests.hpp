#ifndef PEGR_TEST_TESTS_HPP
#define PEGR_TEST_TESTS_HPP

#include <functional>

namespace pegr {
namespace Test {

bool script_helper();

struct NamedTest {
    const char* m_name;
    std::function<bool()> m_test;
};

const NamedTest m_tests[] = {
    
    {"Testing Framework", []()->bool{return true;}},
    {"Script Helper", script_helper},
    
    // Sentinel
    {nullptr, std::function<bool()>()}
};

}
}

#endif // PEGR_TEST_TESTS_HPP
