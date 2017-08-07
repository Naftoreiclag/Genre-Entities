#ifndef PEGR_SCRIPT_SCRIPTUTIL_HPP
#define PEGR_SCRIPT_SCRIPTUTIL_HPP

#include <functional>
#include <string>
#include <vector>

#include "pegr/script/Script.hpp"

namespace pegr {
namespace Script {
namespace Util {

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

/**
 * @brief Returns the number value of the value on the Lua stack at given index.
 * Should be identical to lua_tonumber(), except it returns success rather than
 * silently returning zero on error.
 * @param idx Value to parse as number
 * @param num Where to store the interpreted number
 * @return Success
 */
bool to_number_safe(int idx, lua_Number& num);

/**
 * @brief Pushes a new weak table onto the stack, with the provided mode:
 * If the mode has the character "k", the keys are weak.
 * If the mode has the character "v", the values are weak.
 * [Balance is +1]
 * @param mode
 */
void push_new_weak_table(const char* mode);

/**
 * @class Unique_Regref_Manager
 * @brief A collection of Lua values, using as few unique registry references
 * as possible. Internally, uses a Lua table with the added values as keys and
 * the registry references as values. When adding a Lua value to this
 * collection, the value is compared with the keys in that table.
 */
class Unique_Regref_Manager {
public:
    Unique_Regref_Manager();
    
    Script::Regref add_lua_value(Script::Regref val_ref);
    
    const std::vector<Script::Unique_Regref>& get_lua_uniques() const;
    
    std::vector<Script::Unique_Regref> release();
    
    void clear();
    
private:
    
    Script::Unique_Regref m_lua_uniques_lookup;
    std::vector<Script::Unique_Regref> m_lua_uniques;
};

} // namespace Helper
} // namespace Script
} // namespace pegr

#endif // PEGR_SCRIPT_SCRIPTUTIL_HPP
