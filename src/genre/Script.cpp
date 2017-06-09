#include "Script.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

namespace pegr {
namespace Script {

const char* PEGR_MODULE_NAME = "pegr";
    
lua_State* m_l = nullptr;
bool m_initialized = false;
std::string m_lua_version;
Regref m_pristine_sandbox;

Regref m_pegr_table;
Regref m_pegr_table_safe;

typedef std::pair<const char*, std::vector<const char*> > Whitelist_Entry;

const Regref ROOT_ENVIRONMENT = LUA_GLOBALSINDEX;

const std::vector<Whitelist_Entry> m_global_whitelist = {
    { nullptr, {
        "_VERSION",
        "assert",
        //"collectgarbage", // unsafe, easy to abuse
        //"dofile", // unsafe, does not automatically sandbox
        //"dump", // unnecessary
        "error",
        //"gcinfo", //unsure what this does
        //"getfenv", // unsafe, get access to global table
        "getmetatable", // safe?
        "ipairs",
        //"load", // unsafe, does not automatically sandbox
        //"loadfile", // unsafe, does not automatically sandbox
        //"loadstring", // unsafe, does not automatically sandbox
        //"module", // ??
        //"newproxy", // ??
        "next",
        "pairs",
        "pcall",
        "print",
        //"rawequal", // ??
        //"rawget", // ??
        //"rawset", // ??
        "require", // temporary, TODO: reimplement
        "select",
        "setfenv", // safe?
        "setmetatable", // safe?
        "tonumber",
        "tostring",
        "type",
        "unpack", // safe?
        "xpcall"
    } },
    { "coroutine", {
        "create",
        "resume",
        "running",
        "status",
        "wrap",
        "yield"
    } },
    // Definitely no access to the "debug" library
    // Todo: reimplement "io" library?
    { "math", {
        "abs",
        "acos",
        "asin",
        "atan",
        "atan2",
        "ceil",
        "cos",
        "cosh",
        "deg",
        "exp",
        "floor",
        "fmod",
        "frexp",
        "huge",
        "ldexp",
        "log",
        "log10",
        "max",
        "min",
        "mod",
        "modf",
        "pi",
        "pow",
        "rad",
        "random",
        "randomseed",
        "sin",
        "sinh",
        "sqrt",
        "tan",
        "tanh",
    } },
    { "os", {
        "clock",
        "date",
        "difftime",
        "time"
    } },
    // What does the "package" library do anyhow?
    { "string", {
        "byte",
        "char",
        "find",
        "format",
        "gmatch",
        "gsub",
        "len",
        "lower",
        "match",
        "reverse",
        "sub",
        "upper"
    } },
    { "table", {
        "concat",
        "insert",
        "remove",
        "sort"
    } }
};

void initialize() {
    assert(!m_initialized);
    m_initialized = true;
    
    m_l = luaL_newstate();
    luaL_openlibs(m_l);
    
    lua_getglobal(m_l, "_VERSION");
    size_t strlen;
    const char* luastr = lua_tolstring(m_l, -1, &strlen);
    m_lua_version = std::string(luastr, strlen);
    std::cout << m_lua_version << std::endl;
    lua_pop(m_l, 1);
    
    lua_newtable(m_l);
    for (const Whitelist_Entry& whitelisted_table : m_global_whitelist) {
        const char* module = whitelisted_table.first;
        const auto& members = whitelisted_table.second;
        if (module) {
            lua_newtable(m_l);
            lua_getglobal(m_l, module);
            assert(!lua_isnil(m_l, -1) && "Required library not found!");
            for (const char* member : members) {
                lua_getfield(m_l, -1, member);
                //std::cout << module << "." << member << std::endl;
                assert(!lua_isnil(m_l, -1) && "Required lib func not found!");
                lua_setfield(m_l, -3, member);
            }
            lua_pop(m_l, 1);
            lua_setfield(m_l, -2, module);
        }
        else {
            for (const char* member : members) {
                lua_getglobal(m_l, member);
                //std::cout << member << std::endl;
                assert(!lua_isnil(m_l, -1) && "Required glob func not found!");
                lua_setfield(m_l, -2, member);
            }
        }
    }
    lua_newtable(m_l);
    lua_pushvalue(m_l, -1);
    m_pegr_table_safe = luaL_ref(m_l, LUA_REGISTRYINDEX);
    lua_setfield(m_l, -2, PEGR_MODULE_NAME);
    m_pristine_sandbox = luaL_ref(m_l, LUA_REGISTRYINDEX);
    
    lua_newtable(m_l);
    lua_pushvalue(m_l, -1);
    m_pegr_table = luaL_ref(m_l, LUA_REGISTRYINDEX);
    lua_setglobal(m_l, PEGR_MODULE_NAME);
    
    assert(lua_gettop(m_l) == 0);
}

void cleanup() {
    assert(m_initialized);
    m_initialized = false;
    lua_close(m_l);
}

Regref load_c_function(lua_CFunction func, int closure_size) {
    lua_pushcclosure(m_l, func, closure_size);
    return luaL_ref(m_l, LUA_REGISTRYINDEX);
}

const std::streamsize BLOCK_LENGTH = 4096;
struct FR_Closure {
    std::ifstream m_file;
    char m_block[BLOCK_LENGTH];
    
    FR_Closure(std::ifstream&& file) {
        m_file.swap(file);
    }
};

const char* file_reader(lua_State* L, void* data, size_t* size) {
    FR_Closure& closure = *static_cast<FR_Closure*>(data);
    if (closure.m_file.eof()) {
        (*size) = 0;
        return closure.m_block;
    }
    closure.m_file.read(closure.m_block, BLOCK_LENGTH);
    // TODO: error checking?
    (*size) = sizeof(char) * closure.m_file.gcount();
    return closure.m_block;
}

Regref load_lua_function(const char* filename, Regref environment,
                            const char* chunkname) {
    if (!chunkname) { chunkname = filename; }
    /* Note: as far as I know, both Lua 5.1 and LuaJIT recognize bytecode
     * by checking if the first char is 0x1B (escape character '\033')*/
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    // File isn't open
    if (!file.is_open()) {
        std::cout << "Error" << std::endl;
        std::stringstream ss;
        ss << "Could not open file for " << chunkname;
        lua_pushstring(m_l, ss.str().c_str());
    }
    int peek = file.peek();
    // File is empty
    if (peek == std::char_traits<char>::eof()) {
        std::cout << "Error" << std::endl;
        std::stringstream ss;
        ss << "file for " << chunkname << " is empty";
        lua_pushstring(m_l, ss.str().c_str());
    }
    // Bytecode
    if (peek == 0x1B) {
        std::cout << "Error" << std::endl;
        std::stringstream ss;
        ss << "file for " << chunkname << " is bytecode";
        lua_pushstring(m_l, ss.str().c_str());
    }
    int status;
    {
        FR_Closure closure(std::move(file));
        status = lua_load(m_l, file_reader, &closure, chunkname);
    }
    switch (status) {
        case LUA_ERRSYNTAX: {
            std::cout << "Error" << std::endl;
            break;
        }
        case LUA_ERRMEM: {
            std::cout << "Error" << std::endl;
            break;
        }
        default: break;
    }
    lua_rawgeti(m_l, LUA_REGISTRYINDEX, environment);
    lua_setfenv(m_l, -2);
    return luaL_ref(m_l, LUA_REGISTRYINDEX);
}

void run_function(Regref func) {
    lua_rawgeti(m_l, LUA_REGISTRYINDEX, func);
    int status = lua_pcall(m_l, 0, LUA_MULTRET, 0);
    
    switch (status) {
        case LUA_ERRRUN:
        case LUA_ERRMEM:
        case LUA_ERRERR: {
            size_t strlen;
            const char* luastr = lua_tolstring(m_l, -1, &strlen);
            std::cout << std::string(luastr, strlen) << std::endl;
        }
    }
}

Regref new_sandbox() {
    // Make a deep copy of the pristine sandbox
    lua_rawgeti(m_l, LUA_REGISTRYINDEX, m_pristine_sandbox);
    stk_simple_deep_copy();
    // Set the "_G" member to itself
    lua_pushvalue(m_l, -1);
    lua_setfield(m_l, -2, "_G");
    Regref ret_val = luaL_ref(m_l, LUA_REGISTRYINDEX);
    lua_pop(m_l, 1); // Pop the pristine sandbox
    return ret_val;
}

void drop_reference(Regref reference) {
    luaL_unref(m_l, LUA_REGISTRYINDEX, reference);
}

lua_State* get_lua_state() {
    return m_l;
}

void add_to_pegr_table(const char* key, Regref value, bool safe) {
    lua_rawgeti(m_l, LUA_REGISTRYINDEX, m_pegr_table);
    lua_rawgeti(m_l, LUA_REGISTRYINDEX, value);
    if (safe) {
        lua_rawgeti(m_l, LUA_REGISTRYINDEX, m_pegr_table_safe);
        lua_rawgeti(m_l, LUA_REGISTRYINDEX, value);
        lua_setfield(m_l, -2, key);
        lua_pop(m_l, 1);
    }
    lua_setfield(m_l, -2, key);
}

void stk_simple_deep_copy(int idx) {
    if (idx < 0) {
        idx = lua_gettop(m_l) + idx + 1;
    }
    lua_newtable(m_l);
    lua_pushnil(m_l);
    while (lua_next(m_l, idx) != 0) {
        // Copy the reference to the key so there is one for iterating with
        lua_pushvalue(m_l, -2);
        
        // Copy the value if it is a table
        if (lua_type(m_l, -2) == LUA_TTABLE) {
            stk_simple_deep_copy(-2);
        } else {
            lua_pushvalue(m_l, -2);
        }
        
        // Set the key-value pair
        lua_settable(m_l, -5);
        
        // Remove the original value reference
        lua_pop(m_l, 1);
        
        // (A reference to the key is still on the stack for iteration)
    }
}

} // namespace Script
} // namespace pegr

