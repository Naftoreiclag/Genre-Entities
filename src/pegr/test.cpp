#include <stdexcept>
#include <sstream>

#include "pegr/gensys/Gensys.hpp"
#include "pegr/gensys/Lua_Interf.hpp"
#include "pegr/scheduler/Lua_Interf.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/script/ScriptHelper.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/test/Tests.hpp"

using namespace pegr;

int li_debug_stage_compile(lua_State* l) {
    Logger::log()->info("Compile triggered by Lua script");
    Gensys::LI::stage_all();
    Gensys::compile();
}

int li_debug_collect_garbage(lua_State* l) {
    Logger::log()->info("Compile triggered by Lua script");
    lua_gc(Script::get_lua_state(), LUA_GCCOLLECT, 0);
}

void setup() {
    Logger::initialize();
    
    Script::initialize();
    
    Gensys::LI::initialize();
    Sched::LI::initialize();
    
    const luaL_Reg test_api[] = {
        {"debug_stage_compile", li_debug_stage_compile},
        
        // Sentinel
        {nullptr, nullptr}
    };
    
    Script::multi_expose_c_functions(test_api);
    
    Gensys::initialize();
}

void log_header(const char* name, char divider = '.') {
    int more = 80 - std::strlen(name);
    if (more < 1) {
        Logger::log()->info(name);
    }
    else {
        std::stringstream sss;
        sss << name
            << ' '
            << std::string(more, divider);
        Logger::log()->info(sss.str());
    }
}

void run_tests(int& num_passes, int& num_fails) {
    lua_State* l = Script::get_lua_state();
    for (std::size_t idx = 0; /*Until sentiel reached*/; ++idx) {
        int original_stack_size = lua_gettop(l);
        const Test::NamedTest& test = Test::n_tests[idx];
        if (!test.m_name) {
            break;
        }
        log_header(test.m_name);
        try {
            test.m_test();
            
            if (lua_gettop(l) != original_stack_size) {
                std::stringstream ss;
                ss << "Mandatory: Unbalanced test! (";
                int diff = lua_gettop(l) - original_stack_size;
                if (diff < 0) {
                    ss << diff;
                } else {
                    ss << '+' << diff;
                    lua_pop(l, diff);
                }
                ss << ") Later tests may fail inexplicably!";
                throw std::runtime_error(ss.str());
            }
            
            Logger::log()->info("\t...PASSED!");
            ++num_passes;
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("\t...FAILED! %v", e.what());
            ++num_fails;
            continue;
        }
    }
}

void run_lua_tests(int& num_passes, int& num_fails) {
    for (std::size_t idx = 0; /*Until sentiel reached*/; ++idx) {
        const Test::NamedLuaTest& test = Test::n_lua_tests[idx];
        if (!test.m_name) {
            break;
        }
        log_header(test.m_name);
        try {
            Script::Regref_Guard sandbox(Script::new_sandbox());
            std::stringstream sss;
            sss << "test/tests/" << test.m_lua_file;
            Script::Regref_Guard func(
                    Script::load_lua_function(
                            sss.str().c_str(), sandbox, test.m_name));
            Script::Helper::run_simple_function(func, 0);
            
            Gensys::cleanup();
            Gensys::initialize();
            Gensys::LI::clear();
            lua_gc(Script::get_lua_state(), LUA_GCCOLLECT, 0);
            
            Logger::log()->info("\t...PASSED!");
            ++num_passes;
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("\t...FAILED! %v", e.what());
            ++num_fails;
            continue;
        }
    }
}

void run() {
    int num_passes = 0;
    int num_fails = 0;
    log_header("=== C++ TESTS", '=');
    run_tests(num_passes, num_fails);
    log_header("=== Lua TESTS", '=');
    run_lua_tests(num_passes, num_fails);
    log_header("===== RESULTS", '=');
    Logger::log()->info("%v passed\t%v failed", num_passes, num_fails);
}

void cleanup() {
    Gensys::cleanup();
    
    Sched::LI::cleanup();
    Gensys::LI::cleanup();
    
    Script::cleanup();
    
    Logger::cleanup();
}

int main() {
    setup();
    run();
    cleanup();
    return 0;
}
