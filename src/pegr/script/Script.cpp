#include "pegr/script/Script.hpp"

#include <stdexcept>
#include <cassert>
#include <cstddef>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "pegr/logger/Logger.hpp"
#include "pegr/script/ScriptHelper.hpp"

namespace pegr {
namespace Script {

Regref_Guard::Regref_Guard()
: m_reference(LUA_REFNIL) { }

Regref_Guard::Regref_Guard(Regref ref)
: m_reference(ref) { }

Regref_Guard::Regref_Guard(Regref_Guard&& other) {
    release_reference();
    m_reference = other.m_reference;
    other.m_reference = LUA_REFNIL;
}

// Move assignment
Regref_Guard& Regref_Guard::operator =(Regref_Guard&& other) {
    release_reference();
    m_reference = other.m_reference;
    other.m_reference = LUA_REFNIL;
    return *this;
}

// Assignment of value
Regref_Guard& Regref_Guard::operator =(const Regref& value) {
    replace(value);
    return *this;
}

void Regref_Guard::replace(Regref value) {
    release_reference();
    m_reference = value;
}

Regref_Guard::~Regref_Guard() {
    release_reference();
}

Regref Regref_Guard::regref() const {
    return m_reference;
}
    
Regref_Guard::operator Regref() const {
    return regref();
}

void Regref_Guard::release_reference() {
    drop_reference(m_reference);
}

Regref_Shared make_shared(Regref ref) {
    return std::make_shared<Regref_Guard>(ref);
}

const char* PEGR_MODULE_NAME = "pegr";

bool m_torndown = false;
lua_State* m_l = nullptr;
std::string m_lua_version;
Regref m_pristine_sandbox;

Pop_Guard::Pop_Guard(int n)
: m_n(n) { }

Pop_Guard::~Pop_Guard() {
    lua_pop(m_l, m_n);
}

// Keeps track of how many registry keys we are holding
int m_total_grab_delta = 0;

Regref m_pegr_table;
Regref m_pegr_table_safe;

typedef std::pair<const char*, std::vector<const char*> > Whitelist_Entry;

const Regref NO_SANDBOX = LUA_GLOBALSINDEX;

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

struct Lua_Global_Cacher{
    const char* const m_name;
    Regref* const m_cache;
};

void create_state_and_open_libs() {
    m_l = luaL_newstate();
    luaL_openlibs(m_l);
}

Regref m_luaglob__VERISON;
Regref m_luaglob_tostring;
Regref m_luaglob_print;

void cache_lua_std_lib() {
    const Lua_Global_Cacher cachers[] = {
        {"_VERSION",    &m_luaglob__VERISON},
        {"tostring",    &m_luaglob_tostring},
        {"print",       &m_luaglob_print},
        
        // End of the list
        {nullptr, nullptr}
    };
    
    for (int idx = 0; /*Have not reached sentinel*/; ++idx) {
        const Lua_Global_Cacher& cacher = cachers[idx];
        if (!cacher.m_name) {
            break;
        }
        lua_getglobal(m_l, cacher.m_name);
        Logger::log()->verbose(1, "Caching %v", cacher.m_name);
        (*cacher.m_cache) = grab_reference();
        m_total_grab_delta -= 1; /*Ignore these grabs*/
    }
}

void get_and_print_lua_version() {
    push_reference(m_luaglob__VERISON);
    std::size_t strlen;
    const char* luastr = lua_tolstring(m_l, -1, &strlen);
    m_lua_version = std::string(luastr, strlen);
    Logger::log()->info("Lua version: %v", m_lua_version);
    lua_pop(m_l, 1);
}

void replace_std_functions() {
    push_reference(m_luaglob_tostring);
    lua_pushcclosure(m_l, li_print, 1);
    lua_setglobal(m_l, "print");
}

void setup_basic_pristine_sandbox() {
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
                assert(!lua_isnil(m_l, -1) && "Required lib func not found!");
                lua_setfield(m_l, -3, member);
            }
            lua_pop(m_l, 1);
            lua_setfield(m_l, -2, module);
        }
        else {
            for (const char* member : members) {
                lua_getglobal(m_l, member);
                assert(!lua_isnil(m_l, -1) && "Required glob func not found!");
                lua_setfield(m_l, -2, member);
            }
        }
    }
    lua_newtable(m_l);
    lua_pushvalue(m_l, -1);
    m_pegr_table_safe = grab_reference();
    lua_setfield(m_l, -2, PEGR_MODULE_NAME);
    m_pristine_sandbox = grab_reference();
    m_total_grab_delta -= 2; /*Ignore these grabs*/
}

void setup_basic_pegr_module() {
    lua_newtable(m_l);
    lua_pushvalue(m_l, -1);
    m_pegr_table = grab_reference();
    m_total_grab_delta -= 1; /*Ignore these grabs*/
    lua_setglobal(m_l, PEGR_MODULE_NAME);
}

void initialize() {
    assert(!is_initialized());
    assert(!m_torndown);
    
    create_state_and_open_libs();
    cache_lua_std_lib();
    get_and_print_lua_version();
    replace_std_functions();
    setup_basic_pristine_sandbox();
    setup_basic_pegr_module();
    
    assert(lua_gettop(m_l) == 0);
}

bool is_initialized() {
    // Make sure that we aren't torn down and initialized at the same time
    assert(!(m_torndown && !!m_l));
    return m_l;
}

void cleanup() {
    assert(is_initialized());
    assert(!m_torndown);
    if (m_total_grab_delta != 0) {
        Logger::log()->warn("Non-zero grab delta: %v", m_total_grab_delta);
    }
    lua_close(m_l);
    m_l = nullptr;
    m_torndown = true;
}

Regref load_c_function(lua_CFunction func, int closure_size) {
    assert(is_initialized());
    lua_pushcclosure(m_l, func, closure_size);
    return grab_reference();
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
    assert(is_initialized());
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

// TODO: check if this file has already been loaded ... ?
Regref load_lua_function(const char* filename, Regref environment,
                            const char* chunkname) {
    assert(is_initialized());
    if (!chunkname) { chunkname = filename; }
    /* Note: as far as I know, both Lua 5.1 and LuaJIT recognize bytecode
     * by checking if the first char is 0x1B (escape character '\033')*/
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    // File isn't open
    if (!file.is_open()) {
        std::stringstream ss;
        ss << "Could not open file for " << chunkname;
        lua_pushstring(m_l, ss.str().c_str());
        throw std::runtime_error(ss.str());
    }
    int peek = file.peek();
    // File is empty
    if (peek == std::char_traits<char>::eof()) {
        std::stringstream ss;
        ss << "File for " << chunkname << " is empty";
        lua_pushstring(m_l, ss.str().c_str());
        throw std::runtime_error(ss.str());
    }
    // Bytecode
    if (peek == 0x1B) {
        std::stringstream ss;
        ss << "File for " << chunkname << " is bytecode";
        lua_pushstring(m_l, ss.str().c_str());
        throw std::runtime_error(ss.str());
    }
    int status;
    {
        FR_Closure closure(std::move(file));
        status = lua_load(m_l, file_reader, &closure, chunkname);
    }
    switch (status) {
        case LUA_ERRSYNTAX:
        case LUA_ERRMEM: {
            const char* errmsg = lua_tostring(m_l, -1);
            throw std::runtime_error(errmsg);
        }
        default: break;
    }
    if (environment != NO_SANDBOX) {
        push_reference(environment);
        lua_setfenv(m_l, -2);
    }
    return grab_reference();
}

void run_function(int nargs, int nresults) {
    assert(is_initialized());
    switch (lua_pcall(m_l, nargs, nresults, 0)) {
        case LUA_ERRRUN:
        case LUA_ERRMEM:
        case LUA_ERRERR: {
            size_t strlen;
            const char* luastr = lua_tolstring(m_l, -1, &strlen);
            throw std::runtime_error(std::string(luastr, strlen));
        }
    }
}

Regref new_sandbox() {
    assert(is_initialized());
    // Make a deep copy of the pristine sandbox
    push_reference(m_pristine_sandbox);
    Helper::simple_deep_copy(-1);
    // Set the "_G" member to itself
    lua_pushvalue(m_l, -1);
    lua_setfield(m_l, -2, "_G");
    Regref ret_val = grab_reference();
    lua_pop(m_l, 1); // Pop the pristine sandbox
    return ret_val;
}

void drop_reference(Regref ref) {
    assert(is_initialized());
    --m_total_grab_delta;
    luaL_unref(m_l, LUA_REGISTRYINDEX, ref);
}

void push_reference(Regref ref) {
    assert(is_initialized());
    lua_rawgeti(m_l, LUA_REGISTRYINDEX, ref);
}

Regref grab_reference() {
    assert(is_initialized());
    ++m_total_grab_delta;
    return luaL_ref(m_l, LUA_REGISTRYINDEX);
}

lua_State* get_lua_state() {
    assert(is_initialized());
    return m_l;
}

void expose_referenced_value(const char* key, Regref value, bool safe) {
    assert(is_initialized());
    push_reference(m_pegr_table);
    push_reference(value);
    if (safe) {
        push_reference(m_pegr_table_safe);
        lua_pushvalue(m_l, -1);
        lua_setfield(m_l, -2, key);
        lua_pop(m_l, 1);
    }
    lua_setfield(m_l, -2, key);
}
void expose_number(const char* key, lua_Number num, bool safe) {
    assert(is_initialized());
    push_reference(m_pegr_table);
    lua_pushnumber(m_l, num);
    if (safe) {
        push_reference(m_pegr_table_safe);
        lua_pushvalue(m_l, -1);
        lua_setfield(m_l, -2, key);
        lua_pop(m_l, 1);
    }
    lua_setfield(m_l, -2, key);
}
void expose_string(const char* key, const char* str, bool safe) {
    assert(is_initialized());
    push_reference(m_pegr_table);
    lua_pushstring(m_l, str);
    if (safe) {
        push_reference(m_pegr_table_safe);
        lua_pushvalue(m_l, -1);
        lua_setfield(m_l, -2, key);
        lua_pop(m_l, 1);
    }
    lua_setfield(m_l, -2, key);
}
void multi_expose_c_functions(const luaL_Reg* api, bool safe) {
    assert(is_initialized());
    push_reference(m_pegr_table);
    if (safe) {
        push_reference(m_pegr_table_safe);
    }
    for (std::size_t idx = 0; /*reg.name is not nullptr*/; ++idx) {
        const luaL_Reg& reg = api[idx];
        if (reg.name == nullptr) {
            break;
        }
        lua_pushcfunction(m_l, reg.func);
        if (safe) {
            lua_pushvalue(m_l, -1);
            lua_setfield(m_l, -4, reg.name); // Add to the normal table
        }
        lua_setfield(m_l, -2, reg.name); // Add to the remaining table
    }
}

int li_print(lua_State* l) {
    /* Expects upvalues:
     * 1: default lua tostring() function
     */
    int nargs = lua_gettop(l);
    std::stringstream log;
    for (int idx = 1; idx <= nargs; ++idx) {
        const char* str = lua_tostring(l, idx);
        if (!str) {
            lua_pushvalue(l, lua_upvalueindex(1));
            lua_pushvalue(l, idx);
            lua_call(l, 1, 1);
            str = lua_tostring(l, -1);
            if (!str) {
                luaL_argerror(l, idx, 
                    "calling 'tostring' on this value did not return a string");
            }
            lua_pop(l, 1);
        }
        log << str;
        if (idx < nargs) {
            log << "\t";
        }
    }
    Logger::alog("addon")->info(log.str());
    return 0;
}

int absolute_idx(int idx) {
    if (idx < 0) {
        idx = lua_gettop(m_l) + idx + 1;
    }
    return idx;
}

} // namespace Script
} // namespace pegr

