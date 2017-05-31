#ifndef PEGR_SCRIPT_SCRIPT_HPP
#define PEGR_SCRIPT_SCRIPT_HPP

#include <lua.hpp>

namespace pegr {
namespace Script {
    
/**
 * @brief An index into the lua registry, created using luaL_ref().
 */
typedef int Regref;

/**
 * @brief Creates the lua state and loads standard libraries
 */
void initialize();

/**
 * @brief Frees all lua resources, invalidates all registry references
 */
void cleanup();

/**
 * @brief Loads a function from a lua file
 * @param filename The file to read
 * @param environment The environment (closure upvalue table) for this function
 * @param chunkname The name of the chunk used for debugging purposes
 * @return A lua reference to the new function
 */
Regref load_function(const char* filename, Regref environment,
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

}
}

#endif // PEGR_SCRIPT_SCRIPT_HPP
