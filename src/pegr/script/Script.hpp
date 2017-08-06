#ifndef PEGR_SCRIPT_SCRIPT_HPP
#define PEGR_SCRIPT_SCRIPT_HPP

#include <memory>

#include <lua.hpp>

namespace pegr {
namespace Script {
    
/**
 * @brief An index into the lua registry, created using luaL_ref().
 */
typedef int Regref;

/**
 * @brief An integer index into a Lua array (a table with consecutive integer 
 * keys starting at 1)
 */
typedef lua_Number Arridx;

/**
 * @class Unique_Regref
 * @brief Handles the release of a value referenced in the lua registry (RAII)
 * Note that these must go out of scope before Script::cleanup() is called.
 */
class Unique_Regref {
public:
    /**
     * @brief Guards nothing
     */
    Unique_Regref();
    
    /**
     * @brief Guards the given reference. When this guard is deleted (for
     * example, by going out of scope) the reference is freed from the lua
     * registry, possibly causing that value to be gc'd.
     */
    Unique_Regref(Regref ref);
    
    /**
     * @brief Copy construction not allowed
     * (there should only be one guard for a single reference)
     */
    Unique_Regref(const Unique_Regref& rhs) = delete;
    
    /**
     * @brief Copy assignment not allowed 
     * (there should only be one guard for a single reference)
     */
    Unique_Regref& operator =(const Unique_Regref& rhs) = delete;
    
    /**
     * @brief Move construction
     */
    Unique_Regref(Unique_Regref&& rhs);
    
    /**
     * @brief Move assignment
     */
    Unique_Regref& operator =(Unique_Regref&& rhs);
    
    /**
     * @brief Deconstructor. Drops whatever reference it is guarding.
     */
    ~Unique_Regref();
    
    /**
     * @brief Get the Lua reference that this is guarding
     * @return the Lua reference that this is guarding
     */
    Regref get() const;
    
    /**
     * @return Iff this is guarding nothing (nil via LUA_REFNIL)
     */
    bool is_nil() const;
    
    /**
     * @brief Replaces the currently guarded reference with this one. Properly
     * release the previously guarded value
     * @param value Reference to the new value to be guarded.
     */
    void reset(Regref value = LUA_REFNIL);
    
    /**
     * @brief Implicit conversion to the Regref type
     */
    operator Regref() const;
    
    /**
     * @brief Releases ownership of the held Regref (does not release the value
     * from the Lua registry) and returns it
     * @return The formerly guarded Regref
     */
    Regref release();
private:
    
    Regref m_reference;
};

/**
 * @brief A shared pointer for a guard. Useful for having multiple references
 * to the same Lua object. Uses reference counting.
 */
typedef std::shared_ptr<Unique_Regref> Shared_Regref;

/**
 * @brief Make a referenced Lua value into a shared RAII object
 * @param ref The Lua reference to make shared
 * @return A shared RAII object for the provided reference
 */
Shared_Regref make_shared(Regref ref);

extern const char* PEGR_MODULE_NAME;

extern const Regref NO_SANDBOX;
extern Regref m_pristine_sandbox;

extern Regref m_luaglob_tostring;
extern Regref m_luaglob__VERISON;
extern Regref m_luaglob_print;

/**
 * @class Pop_Guard
 * @brief RAII for stack size. When this object is destructed, the specified
 * number of elements are popped off the Lua stack (calls lua_pop)
 */
class Pop_Guard {
public:
    Pop_Guard(int n = 0, lua_State* l = nullptr);
    ~Pop_Guard();
    
    /**
     * @brief Pops n values prematurely (before destructor is called)
     * @param n The number of values to pop. Must be <= m_n
     */
    void pop(int n);
    
    /**
     * @brief Increases total number of values to pop when the destructor
     * is called. Effectively increases how many stack entries are 
     * being guarded.
     * @param n The number of unguarded values that have just been pushed 
     * onto the stack.
     */
    void on_push(int n);
    
    /**
     * @brief Decreases total number of values to pop when the destructor
     * is called. Effectively decreases how many stack entries are 
     * being guarded.
     * @param n The number of previously guarded values that have just 
     * been popped off the stack.
     */
    void on_pop(int n);
private:
    int m_n;
    lua_State* m_lstate;
};

int debug_get_total_grab_delta();

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
 * @brief Runs the function on the stack. Behaves exactly like lua_pcall,
 * except Lua errors are turned into thrown runtime exceptions.
 * Guaranteed to pop off the expected number of values from the stack regardless
 * of errors.
 * @param nargs The number of arguments this function will consume
 * @param nresults The number of values this function will return
 */
void run_function(int nargs, int nresults);

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
 * expose_referenced_value with referenced functions.
 * [BALANCED]
 * @param api Multiple registry entries. Array must end with a pair of nullptrs.
 * @param safe If true, then these values are accessible in sandboxes
 */
void multi_expose_c_functions(const luaL_Reg* api, bool safe = true);

/**
 * @brief Turns an index relative to the top of the stack (negative indices)
 * into ones relative to the bottom of the stack (absolute indices)
 * @param idx The index, can be negative
 * @return An index that points to the same position on the stack
 */
int absolute_idx(int idx);

namespace LI {

/**
 * @brief Replacement print for lua, uses logger
 */
int li_print(lua_State* l);

} // namespace LI
} // namespace Script
} // namespace pegr

#endif // PEGR_SCRIPT_SCRIPT_HPP
