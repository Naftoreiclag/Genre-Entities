#include <stdexcept>

#include "pegr/script/ScriptHelper.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"

namespace pegr {
namespace Test {

//@Test Identifying syntax errors
void test_0010_check_script_loading() {
    lua_State* l = Script::get_lua_state();
    Script::Unique_Regref sandbox(Script::new_sandbox());
    try {
        Script::Unique_Regref error(
            Script::load_lua_function("test/common/error_syntax.lua", sandbox));
    }
    catch (std::runtime_error e) {
        lua_pop(l, 1);
        return;
    }
    lua_pop(l, 1);
    throw std::runtime_error("No syntax error");
}

//@Test Script Pop_Guard memory leaks
void test_0010_check_pop_guard() {
    lua_State* l = Script::get_lua_state();
    lua_pushnil(l);
    Script::Pop_Guard pg(1);
}

//@Test Script Unique_Regref memory leaks
void test_0010_check_guard_memory_leaks() {
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
        Script::Unique_Regref sandbox(Script::new_sandbox());
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

//@Test Script Shared_Regref memory leaks
void test_0010_check_guard_memory_leaks_shared() {
    lua_State* l = Script::get_lua_state();
    Script::Regref ref;
    ref = Script::new_sandbox();
    
    {
        Script::Shared_Regref shared1;
        {
            Script::Shared_Regref shared2 = Script::make_shared(ref);
            shared1 = shared2;
        }
    }
    
    Script::push_reference(ref);
    if (!lua_isnil(l, -1)) {
        lua_pop(l, 1);
        throw std::runtime_error("Shared guard did not release reference!");
    }
    lua_pop(l, 1);
    
    
}

} // namespace Test
} // namespace pegr
