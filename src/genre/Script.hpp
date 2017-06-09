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

extern const Regref ROOT_ENVIRONMENT;
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
 * @brief Adds a lua value to the pegr table
 * @param key The key
 * @param value The value
 * @param safe If true, then this value is accessible in sandboxes
 */
void add_to_pegr_table(const char* key, Regref value, bool safe = true);

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
