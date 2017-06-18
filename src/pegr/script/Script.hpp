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
 * @class Regref_Guard
 * @brief Handles the release of a value referenced in the lua registry (RAII)
 * Note that these must go out of scope before Script::cleanup() is called.
 */
class Regref_Guard {
public:
    /**
     * @brief Guards nothing
     */
    Regref_Guard();
    
    /**
     * @brief Guards the given reference. When this guard is deleted (for
     * example, by going out of scope) the reference is freed from the lua
     * registry, possibly causing that value to be gc'd.
     */
    Regref_Guard(Regref ref);
    
    /**
     * @brief Copy construction not allowed
     * (there should only be one guard for a single reference)
     */
    Regref_Guard(const Regref_Guard& other) = delete;
    
    /**
     * @brief Copy assignment not allowed 
     * (there should only be one guard for a single reference)
     */
    Regref_Guard& operator =(const Regref_Guard& other) = delete;
    
    /**
     * @brief Allow assignment of references directly to the guard. The
     * currently guarded object will be released.
     * @param ref The reference to guard
     */
    Regref_Guard& operator =(const Regref& value);
    
    /**
     * @brief Move construction
     */
    Regref_Guard(Regref_Guard&& other);
    
    /**
     * @brief Move assignment
     */
    Regref_Guard& operator=(Regref_Guard&& other);
    
    /**
     * @brief Deconstructor. Drops whatever reference it is guarding.
     */
    ~Regref_Guard();
    
    /**
     * @brief Get the Lua reference that this is guarding
     * @return the Lua reference that this is guarding
     */
    Regref regref() const;
    
    /**
     * @brief Implicit conversion to the Regref type
     */
    operator Regref() const;
    
private:
    void release_reference();
    
    Regref m_reference;
};

extern const char* PEGR_MODULE_NAME;

extern const Regref NO_SANDBOX;
extern Regref m_pristine_sandbox;

extern Regref m_luaglob_tostring;
extern Regref m_luaglob__VERISON;
extern Regref m_luaglob_print;

/**
 * @brief Creates the lua state and loads standard libraries
 */
void initialize();

/**
 * @brief Returns if the lua scripting system is initialized
 * @return if the lua scripting system is initialized
 */
bool is_initialized();

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
 * @param nargs The number of arguments this function will consume
 * @param nresults The number of values this function will return
 * @return success
 */
bool run_function(Regref func, int nargs, int nresults);

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
 * @brief Pushes the referenced value onto the lua stack.
 * @param reference The reference you would like to be pushed onto the stack
 */
void push_reference(Regref reference);

/**
 * @brief Makes a new reference to the element on the top of the stack and pops
 * it from the stack.
 * @return The new reference
 */
Regref grab_reference();

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
 * @brief Replacement print for lua, uses logger
 */
int li_print(lua_State* l);


} // namespace Script
} // namespace pegr

#endif // PEGR_SCRIPT_SCRIPT_HPP
