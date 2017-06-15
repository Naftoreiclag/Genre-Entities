#ifndef PEGR_TEST_TESTS_HPP
#define PEGR_TEST_TESTS_HPP

#include <functional>

namespace pegr {
namespace Test {

bool init_sanity_test();
bool script_helper();

struct NamedTest {
    const char* m_name;
    std::function<bool()> m_test;
};

const NamedTest m_tests[] = {
    
    {"Testing Framework", []()->bool{return true;}},

    {"000 Initialization Sanity Test", init_sanity_test},
    {"Script Helper", script_helper},
    
    // Sentinel
    {nullptr, std::function<bool()>()}
};

}
}

#endif // PEGR_TEST_TESTS_HPP
