#ifndef PEGR_SCRIPT_SCRIPT_HPP
#define PEGR_SCRIPT_SCRIPT_HPP

#include <lua.hpp>

namespace pegr {
namespace Script {
    
/**
 * @brief An index into the lua registry, created using luaL_ref().
 */
typedef int Regref;

extern const char* PEGR_MODULE_NAME;

extern const Regref NO_SANDBOX;
extern Regref m_pristine_sandbox;

/**
 * @brief Creates the lua state and loads standard libraries
 */
void initialize();

/**
 * @brief Frees all lua resources, invalidates all registry references
 */
void cleanup();

/**
 * @brief Loads a c function
 * @param func Pointer to the function to add
 * @param closure_size 
 * @return 
 */
Regref load_c_function(lua_CFunction func, int closure_size = 0);

/**
 * @brief Loads a function from a lua file
 * @param filename The file to read
 * @param environment The environment (closure upvalue table) for this function
 * @param chunkname The name of the chunk used for debugging purposes
 * @return A lua reference to the new function
 */
Regref load_lua_function(const char* filename, Regref environment,
                            const char* chunkname = nullptr);

/**
 * @brief Executes a function given by a registry reference.
 * @param func The registry reference for this function
 */
void run_function(Regref func);

/**
 * @brief Produces a new sandbox environment for running user scripts in.
 * @return A lua reference to the new sandbox
 */
Regref new_sandbox();

/**
 * @brief Releases the reference to the lua value, allowing for it to be gc'd
 * @param reference The reference you would like to be liberated
 */
void drop_reference(Regref reference);

/**
 * @brief Get the global lua state
 * @return The global lua state
 */
lua_State* get_lua_state();

/**
 * @brief Adds a lua value to the pegr table, making it accessible in lua.
 * @param key The key
 * @param value Lua reference to the value
 * @param safe If true, then this value is accessible in sandboxes
 */
void expose_referenced_value(const char* key, Regref value, bool safe = true);

/**
 * @brief Adds a number to the pegr table
 * @param key The key
 * @param num The number
 * @param safe If true, then this value is accessible in sandboxes
 */
void expose_number(const char* key, lua_Number num, bool safe = true);

/**
 * @brief Adds a string to the pegr table
 * @param key The key
 * @param str The string
 * @param safe If true, then this value is accessible in sandboxes
 */
void expose_string(const char* key, const char* str, bool safe = true);

/**
 * @brief Exposes multiple c functions to lua. Faster than repeatedly calling
 * expose_referenced_value with referenced functions
 * @param api Multiple registry entries. Array must end with a pair of nullptrs.
 * @param safe If true, then these values are accessible in sandboxes
 */
void multi_expose_c_functions(const luaL_Reg* api, bool safe = true);

/**
 * @brief Makes a "semi-deep" copy of the table at given index:
 * Does try to copy the keys. Only tries to copy the values if they are tables.
 * The copy is pushed onto the stack.
 * @param idx The index in the stack of the table you want to copy. Does not
 * pop that value off the stack.
 */
void stk_simple_deep_copy(int idx = -1);


} // namespace Script
} // namespace pegr

#endif // PEGR_SCRIPT_SCRIPT_HPP
