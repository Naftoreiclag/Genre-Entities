#ifndef PEGR_TEST_TESTS_HPP
#define PEGR_TEST_TESTS_HPP

#include <functional>

namespace pegr {
namespace Test {

void test_0000_init_sanity();
void test_0030_gensys_primitive();
void test_0030_gensys_primitive_multiple();
void test_script_helper();

struct NamedTest {
    const char* m_name;
    std::function<void()> m_test;
};

const NamedTest m_tests[] = {
    
    {"Testing Framework", [](){}},

    {"Initialization Sanity Test", test_0000_init_sanity},
    {"Single gensys primitive", test_0030_gensys_primitive},
    {"Reassignment of gensys primitives", test_0030_gensys_primitive_multiple},
    {"Script Helper", test_script_helper},
    
    // Sentinel
    {nullptr, std::function<void()>()}
};

}
}

#endif // PEGR_TEST_TESTS_HPP
