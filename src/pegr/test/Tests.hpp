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

#ifndef PEGR_TEST_TESTS_HPP
#define PEGR_TEST_TESTS_HPP

#include <functional>

/*******************************************************************************
 * WARNING! This file is automatically generated by a Python script! Any manual
 * edits to this file are likely to be overwritten automatically! To make any
 * changes other than adding/removing tests, modify TestsBoilerplate.hpp
 * located in the tool/ dir and run GenerateTestList.py
 ******************************************************************************/

namespace pegr {
namespace Test {

void test_0000_algs();
void test_0000_memory_test();
void test_0000_ptr_cast();
void test_0001_flags();
void test_0001_init_sanity();
void test_0002_app_state_machine_test();
void test_0002_unique_ptr_test();
void test_0003_lambda_closure();
void test_0005_assertion_test();
void test_0010_check_guard_memory_leaks();
void test_0010_check_guard_memory_leaks_shared();
void test_0010_check_pop_guard();
void test_0010_check_script_loading();
void test_0010_check_unique_regref();
void test_0028_for_pairs();
void test_0028_for_pairs_exception();
void test_0028_for_pairs_number_sorted();
void test_0028_simple_deep_copy();
void test_0028_simple_deep_copy_recursive();
void test_0028_to_number_safe();
void test_0028_to_string();
void test_0028_unique_regref_manager();
void test_0029_lua_as_lambda();
void test_0030_gensys_primitive();
void test_0030_gensys_primitive_multiple();
void test_0080_00_gensys_primitive();
void test_0085_00_podchunk_test();
void test_0099_gensys_runtime();
void test_0100_unique_handle_validity();
void test_0100_unique_render_handles();
void test_0101_resource_oid_test();

struct NamedTest {
    const char* m_name;
    std::function<void()> m_test;
};

const NamedTest n_tests[] = {
    
    {"Testing Framework", [](){}},

    {"Util algs test", test_0000_algs},
    {"Memory Test", test_0000_memory_test},
    {"Pointer cast", test_0000_ptr_cast},
    {"Flag test", test_0001_flags},
    {"Initialization Sanity Test", test_0001_init_sanity},
    {"App State Machine Test", test_0002_app_state_machine_test},
    {"Unique Ptr Test", test_0002_unique_ptr_test},
    {"Lambda closure", test_0003_lambda_closure},
    {"Assertion test", test_0005_assertion_test},
    {"Script Unique_Regref memory leaks", test_0010_check_guard_memory_leaks},
    {"Script Shared_Regref memory leaks", test_0010_check_guard_memory_leaks_shared},
    {"Script Pop_Guard memory leaks", test_0010_check_pop_guard},
    {"Identifying syntax errors", test_0010_check_script_loading},
    {"More Unique_Regref tests", test_0010_check_unique_regref},
    {"Script Helper for_pairs", test_0028_for_pairs},
    {"Script Helper for_pairs with exception", test_0028_for_pairs_exception},
    {"Script Helper for_number_pairs_sorted", test_0028_for_pairs_number_sorted},
    {"Script simple_deep_copy", test_0028_simple_deep_copy},
    {"Script simple_deep_copy_recursive", test_0028_simple_deep_copy_recursive},
    {"Script to_number_safe", test_0028_to_number_safe},
    {"Script Helper to_string", test_0028_to_string},
    {"Script Helper unique regref manager", test_0028_unique_regref_manager},
    {"Lua as Lambda Test", test_0029_lua_as_lambda},
    {"Single gensys primitive", test_0030_gensys_primitive},
    {"Reassignment of gensys primitives", test_0030_gensys_primitive_multiple},
    {"Gensys primitive from Lua values", test_0080_00_gensys_primitive},
    {"PodChunk test", test_0085_00_podchunk_test},
    {"Gensys Runtime Test", test_0099_gensys_runtime},
    {"Unique handle validity", test_0100_unique_handle_validity},
    {"Unique render handles templates", test_0100_unique_render_handles},
    {"Resource OID test", test_0101_resource_oid_test},
    
    // Sentinel
    {nullptr, std::function<void()>()}
};

struct NamedLuaTest {
    const char* m_name;
    const char* m_lua_file;
};

const NamedLuaTest n_lua_tests[] = {
    
    {"The simplest test possible", "0000_basic.lua"},
    {"Simple sandbox test", "0001_sandbox_test.lua"},
    {"Basic Gensys test", "0005_gensys_test.lua"},
    {"Gensys test Lua garbage collection", "0005_gensys_test_gc.lua"},
    {"Gensys genre matching", "0005_gensys_test_genres.lua"},
    {"Gensys component matching", "0005_gensys_test_matching.lua"},
    {"Gensys string test", "0005_gensys_test_strings.lua"},
    
    // Sentinel
    {nullptr, nullptr}
};

} // namespace pegr
} // namespace Test

#endif // PEGR_TEST_TESTS_HPP
