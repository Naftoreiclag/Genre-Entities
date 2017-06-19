#include <stdexcept>

#include "pegr/script/ScriptHelper.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"

namespace pegr {
namespace Test {

//@Test Identifying syntax errors
void test_0010_check_script_loading() {
    lua_State* l = Script::get_lua_state();
    Script::Regref_Guard sandbox(Script::new_sandbox());
    try {
        Script::Regref_Guard
            error(Script::load_lua_function("test/error_syntax.lua", sandbox));
    }
    catch (std::runtime_error e) {
        lua_pop(l, 1);
        return;
    }
    lua_pop(l, 1);
    throw std::runtime_error("No syntax error");
}

//@Test Script regref memory leaks
void test_0010_check_memory_leaks() {
    lua_State* l = Script::get_lua_state();
    Logger::log()->info("Testing explicit...");
    
    Script::Regref rr = Script::new_sandbox();
    Logger::log()->info("Should drop reference...");
    Script::drop_reference(rr);
    Script::push_reference(rr);
    if (!lua_isnil(l, -1)) {
        lua_pop(l, 1);
        throw std::runtime_error("Could not release reference!");
    }
    lua_pop(l, 1);
    Logger::log()->info("Testing guard...");
    
    Script::Regref ref;
    {
        Script::Regref_Guard sandbox(Script::new_sandbox());
        ref = sandbox;
        Logger::log()->info("Should drop reference...");
    }
    lua_gc(l, LUA_GCCOLLECT, 0);
    Script::push_reference(ref);
    if (!lua_isnil(l, -1)) {
        lua_pop(l, 1);
        throw std::runtime_error("Guard did not release reference!");
    }
    lua_pop(l, 1);
    
    
    
}

} // namespace Test
} // namespace pegr
