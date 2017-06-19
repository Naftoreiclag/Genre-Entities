#ifndef PEGR_TEST_TESTS_HPP
#define PEGR_TEST_TESTS_HPP

#include <functional>

namespace pegr {
namespace Test {

bool test_000_init_sanity();
bool test_gensys_primitive_test();
bool test_script_helper();

struct NamedTest {
    const char* m_name;
    std::function<bool()> m_test;
};

const NamedTest m_tests[] = {
    
    {"Testing Framework", []()->bool{return true;}},

    {"Initialization Sanity Test", test_000_init_sanity},
    {"Gensys Intermediates", test_gensys_primitive_test},
    {"Script Helper", test_script_helper},
    
    // Sentinel
    {nullptr, std::function<bool()>()}
};

}
}

#endif // PEGR_TEST_TESTS_HPP
