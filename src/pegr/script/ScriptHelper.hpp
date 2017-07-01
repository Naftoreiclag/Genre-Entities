#ifndef PEGR_SCRIPT_LUAHELPER_HPP
#define PEGR_SCRIPT_LUAHELPER_HPP

#include <functional>
#include <string>
#include <vector>

#include "pegr/script/Script.hpp"

namespace pegr {
namespace Script {
namespace Helper {

/**
 * @brief Calls the provided function for every key,value pair in the lua table
 * at position table_idx on the main stack. Before each call, the key is placed
 * at position -2 and the value at position -1. The loop breaks if the function
 * returns false. The provided function may not modify or remove the key from
 * the stack. Doing so may destroy the loop. The function may consume the value
 * associated with that key, but only if pops_value is true. Whether or not
 * this function successfully balances itself is dependent on whether or not
 * the provided function's balance agrees with the value of pops_value.
 * @param table_idx the location of the table on the lua stack. Can be negative.
 * Does not pop that value off the stack.
 * @param func The function to call for every pair in the table. Allowed to
 * throw exceptions
 * @param pops_value If true, the function may consume the value provided before
 * its execution (the stack is one element smaller after calling the function),
 * or otherwise the function is balanced (the stack size does not change after
 * call).
 */
void for_pairs(int table_idx, std::function<bool()> func, 
                bool pops_value = false);

/**
 * @brief Retrieves all of the number keys in the table stored at table_idx.
 * There are no guarantees on the order of these keys.
 * [BALANCED]
 * @param table_idx the location of the table on the Lua stack. Can be negative.
 * Does not pop that value off the stack.
 * @return A vector of the number keys, in any order.
 */
std::vector<lua_Number> get_number_keys(int table_idx);

/**
 * @brief Same as for_pairs, except it only iterates over number keys, and those
 * numebers are first sorted in order from least to greatest.
 * @param table_idx Same as for_pairs
 * @param func Same as for_pairs
 * @param pops_value Same as for_pairs
 * @param reversed_order If true, iterate from greatest to least.
 */
void for_number_pairs_sorted(int table_idx, std::function<bool()> func, 
                bool pops_value = false, bool reversed_order = false);

/**
 * @brief Makes a "semi-deep" copy of the table at given index:
 * Does not try to copy the keys. 
 * Only tries to copy the values if they are tables.
 * The copy is pushed onto the stack at position -1.
 * @param idx The index in the stack of the table you want to copy. Can be 
 * negative. Does not pop that value off the stack.
 */
void simple_deep_copy(int idx);

/**
 * @brief Runs a function that takes no arguments.
 * @param ref The registry reference for the said function
 * @param nresults The number of results to return.
 */
void run_simple_function(Script::Regref ref, int nresults);

/**
 * @brief Turns the provided value on the main Lua stack into a standard string.
 * Unlike using lua_tolstring(), this function also calls the built-in Lua
 * function tostring() if the provided value cannot immediately be turned into
 * a string. If the value returned by tostring() is not convertable to a Lua
 * string by lua_tolstring(), then this process is repeated on that returned
 * value at most max_recusions times.
 * [BALANCED]
 * @param idx The location of the relevant value on the main stack. Can be
 * negative
 * @param def The default string to use if there are too many tostring() layers.
 * @param max_recusions The max number of recusions that can be performed,
 * otherwise an exception is thrown or the default string is returned
 * @return The string
 */
std::string to_string(int idx, const char* def = nullptr,
        int max_recusions = 32);

/**
 * @brief Generic default string for calls to to_string()
 */
extern const char* GENERIC_TO_STRING_DEFAULT;

} // namespace Helper
} // namespace Script
} // namespace pegr

#endif // PEGR_SCRIPT_LUAHELPER_HPP
