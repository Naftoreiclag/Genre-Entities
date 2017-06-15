#ifndef PEGR_SCRIPT_LUAHELPER_HPP
#define PEGR_SCRIPT_LUAHELPER_HPP

#include <functional>

namespace pegr {
namespace Script {
namespace Helper {

/**
 * @brief Calls the provided function for every key,value pair in the lua table
 * at position table_idx on the main stack. Before each call, the key is placed
 * at position -2 and the value at position -1. The loop breaks if the function
 * returns false.
 * @param table_idx the location of the table on the lua stack. Can be negative.
 * Does not pop that value off the stack.
 * @param func The function to call for every pair in the table.
 * @param pops_value If true, the function may consume the value provided before
 * its execution (the stack is one element smaller after calling the function),
 * or otherwise the function is balanced (the stack size does not change after
 * call).
 */
void for_pairs(int table_idx, std::function<bool()> func, 
                bool pops_value = false);
    
/**
 * @brief Makes a "semi-deep" copy of the table at given index:
 * Does try to copy the keys. Only tries to copy the values if they are tables.
 * The copy is pushed onto the stack at position -1.
 * @param idx The index in the stack of the table you want to copy. Can be 
 * negative. Does not pop that value off the stack.
 */
void simple_deep_copy(int idx);

} // namespace Helper
} // namespace Script
} // namespace pegr

#endif // PEGR_SCRIPT_LUAHELPER_HPP
