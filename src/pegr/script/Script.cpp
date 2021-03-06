/*
 *  Copyright 2017 James Fong
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "pegr/script/Script.hpp"

#include <cassert>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "pegr/debug/Debug_Macros.hpp"
#include "pegr/except/Except.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/script/Script_Util.hpp"

namespace pegr {
namespace Script {

Unique_Regref::Unique_Regref()
: m_reference(LUA_REFNIL) { }

Unique_Regref::Unique_Regref(Regref ref)
: m_reference(ref) { }

Unique_Regref::Unique_Regref(Unique_Regref&& rhs) {
    m_reference = rhs.m_reference;
    rhs.m_reference = LUA_REFNIL;
}

// Move assignment
Unique_Regref& Unique_Regref::operator =(Unique_Regref&& rhs) {
    // Either the references are different or both are nil
    assert(m_reference != rhs.m_reference || 
            (m_reference == LUA_REFNIL && rhs.m_reference == LUA_REFNIL));
    
    reset();
    m_reference = rhs.m_reference;
    rhs.m_reference = LUA_REFNIL;
    return *this;
}

void Unique_Regref::reset(Regref value) {
    drop_reference(m_reference);
    m_reference = value;
}

Unique_Regref::~Unique_Regref() {
    reset();
    assert(m_reference == LUA_REFNIL);
}

Regref Unique_Regref::get() const {
    return m_reference;
}

bool Unique_Regref::is_nil() const {
    return m_reference == LUA_REFNIL;
}

Regref Unique_Regref::release() {
    Regref old = m_reference;
    m_reference = LUA_REFNIL;
    return old;
}

Shared_Regref make_shared(Regref ref) {
    return std::make_shared<Unique_Regref>(ref);
}

Shared_Regref make_shared(Unique_Regref&& ref) {
    return std::make_shared<Unique_Regref>(std::move(ref));
}

const char* PEGR_MODULE_NAME = "pegr";

bool m_torndown = false;
lua_State* m_l = nullptr;
std::string m_lua_version;
Regref m_pristine_sandbox;

Pop_Guard::Pop_Guard(int n, lua_State* l)
: m_n(n)
, m_lstate(l ? l : Script::get_lua_state()) {
    assert(m_n >= 0);
}

Pop_Guard::~Pop_Guard() {
    assert(m_n >= 0);
    lua_pop(m_lstate, m_n);
}

void Pop_Guard::on_push(int n) {
    assert(n >= 0);
    m_n += n;
}

void Pop_Guard::on_pop(int n) {
    m_n -= n;
    assert(n >= 0);
}

void Pop_Guard::pop(int n) {
    assert(n <= lua_gettop(m_lstate));
    assert(n >= 0);
    lua_pop(m_lstate, m_n);
    m_n -= n;
}

// Keeps track of how many registry keys we are holding
int n_total_grab_delta = 0;

int debug_get_total_grab_delta() {
    return n_total_grab_delta;
}

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
    } },
    { "jit", {
        "version",
        "version_num",
        //"os",  // Unnecessary, privacy concerns
        //"arch"  // Unnecessary, privacy concerns
    } }
};

struct Lua_Global_Cacher{
    const char* const m_name;
    Regref* const m_cache;
};

void create_state_and_open_libs() {
    m_l = luaL_newstate();
    assert_balance(0);
    luaL_openlibs(m_l);
}

Regref m_luaglob__VERISON;
Regref m_luaglob_jit;
Regref m_luaglob_print;
Regref m_luaglob_tostring;

void cache_lua_std_lib() {
    assert_balance(0);
    const Lua_Global_Cacher cachers[] = {
        {"_VERSION",    &m_luaglob__VERISON},
        {"jit",         &m_luaglob_jit},
        {"print",       &m_luaglob_print},
        {"tostring",    &m_luaglob_tostring},
        
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
        n_total_grab_delta -= 1; /*Ignore these grabs*/
    }
}

void get_and_print_lua_version() {
    assert_balance(0);
    push_reference(m_luaglob_jit);
    if (lua_isnil(m_l, -1)) {
        lua_pop(m_l, 1);
        push_reference(m_luaglob__VERISON);
    } else {
        lua_getfield(m_l, -1, "version");
        lua_remove(m_l, -2);
    }
    std::size_t strlen;
    const char* luastr = lua_tolstring(m_l, -1, &strlen);
    m_lua_version = std::string(luastr, strlen);
    Logger::log()->info("Lua version: %v", m_lua_version);
    lua_pop(m_l, 1);
}

void replace_std_functions() {
    assert_balance(0);
    push_reference(m_luaglob_tostring);
    lua_pushcclosure(m_l, LI::li_print, 1);
    lua_setglobal(m_l, "print");
}

void setup_basic_pristine_sandbox() {
    assert_balance(0);
    lua_newtable(m_l);
    for (const Whitelist_Entry& whitelisted_table : m_global_whitelist) {
        const char* module = whitelisted_table.first;
        const auto& members = whitelisted_table.second;
        if (module) {
            lua_newtable(m_l); // Make a new empty table to copy the module into
            lua_getglobal(m_l, module); // Get the global module
            if (std::strcmp("jit", module) == 0) {
                if (lua_isnil(m_l, -1)) {
                    lua_pop(m_l, 2); // Remove the global module and nil
                    Logger::log()->info("No jit module!");
                    continue;
                }
            } else {
                assert(!lua_isnil(m_l, -1) && "Required library not found!");
            }
            for (const char* member : members) {
                lua_getfield(m_l, -1, member); // Get the func from the module
                assert(!lua_isnil(m_l, -1) && "Required lib func not found!");
                lua_setfield(m_l, -3, member); // Store into module copy
            }
            lua_pop(m_l, 1); // Remove the global module
            lua_setfield(m_l, -2, module); // Store module copy into sandbox
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
    n_total_grab_delta -= 2; /*Ignore these grabs*/
}

void setup_basic_pegr_module() {
    assert_balance(0);
    lua_newtable(m_l);
    lua_pushvalue(m_l, -1);
    m_pegr_table = grab_reference();
    n_total_grab_delta -= 1; /*Ignore these grabs*/
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
}

bool is_initialized() {
    // Make sure that we aren't torn down and initialized at the same time
    assert(!(m_torndown && !!m_l));
    return m_l;
}

void cleanup() {
    assert(is_initialized());
    assert(!m_torndown);
    if (n_total_grab_delta != 0) {
        Logger::log()->warn("Non-zero grab delta: %v", n_total_grab_delta);
    }
    lua_close(m_l);
    m_l = nullptr;
    m_torndown = true;
}

Unique_Regref load_c_function(lua_CFunction func, int closure_size) {
    assert(is_initialized());
    lua_pushcclosure(m_l, func, closure_size);
    return grab_unique_reference();
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
Unique_Regref load_lua_function(const char* filename, Regref environment,
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
        throw Except::Runtime(ss.str());
    }
    int peek = file.peek();
    // File is empty
    if (peek == std::char_traits<char>::eof()) {
        std::stringstream ss;
        ss << "File for " << chunkname << " is empty";
        lua_pushstring(m_l, ss.str().c_str());
        throw Except::Runtime(ss.str());
    }
    // Bytecode
    if (peek == 0x1B) {
        std::stringstream ss;
        ss << "File for " << chunkname << " is bytecode";
        lua_pushstring(m_l, ss.str().c_str());
        throw Except::Runtime(ss.str());
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
            throw Except::Runtime(errmsg);
        }
        default: break;
    }
    if (environment != NO_SANDBOX) {
        push_reference(environment);
        lua_setfenv(m_l, -2);
    }
    return grab_unique_reference();
}

void run_function(int nargs, int nresults) {
    assert(is_initialized());
    switch (lua_pcall(m_l, nargs, nresults, 0)) {
        case LUA_ERRRUN:
        case LUA_ERRMEM:
        case LUA_ERRERR: {
            size_t strlen;
            const char* luastr = lua_tolstring(m_l, -1, &strlen);
            throw Except::Runtime(std::string(luastr, strlen));
        }
    }
}

Unique_Regref new_sandbox() {
    assert(is_initialized());
    push_reference(m_pristine_sandbox);
    Script::Pop_Guard pg(1);
    // Make a deep copy of the pristine sandbox
    Util::simple_deep_copy(-1);
    // Set the "_G" member to itself
    lua_pushvalue(m_l, -1);
    lua_setfield(m_l, -2, "_G");
    return grab_unique_reference();
}

void drop_reference(Regref ref) {
    // Check before the assertion, since we may drop nils after the Lua state
    // is cleaned, due to Unique_Regref deconstructors.
    if (ref == LUA_REFNIL) return;
    assert(is_initialized() && "Maybe you forgot some Unique_Regref's?");
    --n_total_grab_delta;
    luaL_unref(m_l, LUA_REGISTRYINDEX, ref);
}

void push_reference(Regref ref) {
    assert(is_initialized());
    assert_balance(1);
    lua_rawgeti(m_l, LUA_REGISTRYINDEX, ref);
}

Regref grab_reference() {
    assert(is_initialized());
    assert_balance(-1);
    if (lua_isnil(m_l, -1)) {
        lua_pop(m_l, 1);
        return LUA_REFNIL;
    }
    ++n_total_grab_delta;
    Regref retval = luaL_ref(m_l, LUA_REGISTRYINDEX);
    return retval;
}

Unique_Regref grab_unique_reference() {
    assert(is_initialized());
    assert_balance(-1);
    return Unique_Regref(grab_reference());
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
    assert_balance();
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
    if (safe) {
        lua_pop(m_l, 2);
    } else {
        lua_pop(m_l, 1);
    }
}

int absolute_idx(int idx) {
    if (idx < 0) {
        idx = lua_gettop(m_l) + idx + 1;
    }
    return idx;
}

namespace LI {

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
            log << str;
            lua_pop(l, 1);
        } else {
            log << str;
        }
        if (idx < nargs) {
            log << "\t";
        }
    }
    Logger::alog("addon")->info(log.str());
    return 0;
}

} // namespace LI
} // namespace Script
} // namespace pegr

