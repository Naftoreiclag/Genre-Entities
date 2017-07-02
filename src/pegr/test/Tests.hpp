#ifndef PEGR_TEST_TESTS_HPP
#define PEGR_TEST_TESTS_HPP

#include <functional>

namespace pegr {
namespace Test {

void test_0000_init_sanity();
void test_0005_init_sanity();
void test_0010_check_guard_memory_leaks();
void test_0010_check_guard_memory_leaks_shared();
void test_0010_check_pop_guard();
void test_0010_check_script_loading();
void test_0028_for_pairs();
void test_0028_for_pairs_exception();
void test_0028_for_pairs_number_sorted();
void test_0028_simple_deep_copy();
void test_0028_simple_deep_copy_recursive();
void test_0028_to_number_safe();
void test_0028_to_string();
void test_0030_gensys_primitive();
void test_0030_gensys_primitive_multiple();
void test_0080_00_gensys_primitive();

struct NamedTest {
    const char* m_name;
    std::function<void()> m_test;
};

const NamedTest m_tests[] = {
    
    {"Testing Framework", [](){}},

    {"Initialization Sanity Test", test_0000_init_sanity},
    {"Assertion test", test_0005_init_sanity},
    {"Script Regref_Guard memory leaks", test_0010_check_guard_memory_leaks},
    {"Script Regref_Shared memory leaks", test_0010_check_guard_memory_leaks_shared},
    {"Script Pop_Guard memory leaks", test_0010_check_pop_guard},
    {"Identifying syntax errors", test_0010_check_script_loading},
    {"Script Helper for_pairs", test_0028_for_pairs},
    {"Script Helper for_pairs with exception", test_0028_for_pairs_exception},
    {"Script Helper for_number_pairs_sorted", test_0028_for_pairs_number_sorted},
    {"Script simple_deep_copy", test_0028_simple_deep_copy},
    {"Script simple_deep_copy_recursive", test_0028_simple_deep_copy_recursive},
    {"Script to_number_safe", test_0028_to_number_safe},
    {"Script Helper to_string", test_0028_to_string},
    {"Single gensys primitive", test_0030_gensys_primitive},
    {"Reassignment of gensys primitives", test_0030_gensys_primitive_multiple},
    {"Gensys primitive from Lua values", test_0080_00_gensys_primitive},
    
    // Sentinel
    {nullptr, std::function<void()>()}
};

}
}

#endif // PEGR_TEST_TESTS_HPP
