#include <stdexcept>
#include <iostream>
#include <sstream>
#include <chrono>
#include <ratio>

#include "pegr/gensys/Gensys.hpp"
#include "pegr/gensys/Lua_Interf.hpp"
#include "pegr/scheduler/Lua_Interf.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/script/ScriptHelper.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/test/Tests.hpp"

using namespace pegr;

const char* const COLOR_RED = "\033[91m";
const char* const COLOR_GREEN = "\033[92m";
const char* const COLOR_YELLOW = "\033[93m";
const char* const COLOR_RESET = "\033[0m";

typedef std::chrono::time_point<std::chrono::high_resolution_clock> Time_Point;
Time_Point n_start_time;
bool n_timer_set = false;

int li_debug_stage_compile(lua_State* l) {
    Gensys::LI::stage_all();
    Gensys::compile();
    return 0;
}

int li_debug_collect_garbage(lua_State* l) {
    lua_gc(Script::get_lua_state(), LUA_GCCOLLECT, 0);
    return 0;
}

int li_debug_timer_start(lua_State* l) {
    n_start_time = std::chrono::high_resolution_clock::now();
    n_timer_set = true;
    return 0;
}

int li_debug_timer_end(lua_State* l) {
    Time_Point end_time = std::chrono::high_resolution_clock::now();
    
    if (!n_timer_set) {
        luaL_error(l, "Timer not set!");
    }
    
    const char* msg = luaL_checkstring(l, 1);
    lua_Number sample_size = luaL_checknumber(l, 2);
    luaL_argcheck(l, 2, sample_size > 0, "Sample size must be > 0");
    const char* resol = "s";
    if (lua_gettop(l) > 2) {
        resol = luaL_checkstring(l, 3);
    }
    
    std::chrono::duration<double> time_diff = end_time - n_start_time;
    double duration = time_diff.count();
    if (std::strcmp(resol, "s") == 0) {
        duration *= 1e0;
    } else if (std::strcmp(resol, "ms") == 0) {
        duration *= 1e3;
    } else if (std::strcmp(resol, "us") == 0) {
        duration *= 1e6;
    } else if (std::strcmp(resol, "ns") == 0) {
        duration *= 1e9;
    } else {
        luaL_error(l, "Unknown resolution: %s", resol);
    }
    duration /= sample_size;
    
    Logger::log()->info("%v--> %v%v for %v%v",
            COLOR_YELLOW, duration, resol, msg, COLOR_RESET);
    n_timer_set = false;
    return 0;
}

void setup() {
    Logger::initialize();
    
    Script::initialize();
    
    Gensys::LI::initialize();
    Sched::LI::initialize();
    
    const luaL_Reg test_api[] = {
        {"debug_stage_compile", li_debug_stage_compile},
        {"debug_collect_garbage", li_debug_collect_garbage},
        {"debug_timer_start", li_debug_timer_start},
        {"debug_timer_end", li_debug_timer_end},
        
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
            
            Logger::log()->info("%v\t...PASSED!%v", COLOR_GREEN, COLOR_RESET);
            ++num_passes;
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("%v\t...FAILED! %v%v", 
                    COLOR_RED, e.what(), COLOR_RESET);
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
            Script::Unique_Regref sandbox(Script::new_sandbox());
            std::stringstream sss;
            sss << "test/tests/" << test.m_lua_file;
            Script::Unique_Regref func(
                    Script::load_lua_function(
                            sss.str().c_str(), sandbox, test.m_name));
            Script::Helper::run_simple_function(func, 0);
            Logger::log()->info("%v\t...PASSED!%v", COLOR_GREEN, COLOR_RESET);
            ++num_passes;
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("%v\t...FAILED! %v%v", 
                    COLOR_RED, e.what(), COLOR_RESET);
            ++num_fails;
            continue;
        }
        Gensys::cleanup();
        Gensys::initialize();
        Gensys::LI::clear();
        lua_gc(Script::get_lua_state(), LUA_GCCOLLECT, 0);
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
    if (num_passes > 0) {
        Logger::log()->info("%v%v PASSED%v", 
                COLOR_GREEN, num_passes, COLOR_RESET);
    }
    if (num_fails > 0) {
        Logger::log()->info("%v%v FAILED%v", 
                COLOR_RED, num_fails, COLOR_RESET);
    } else {
        Logger::log()->info("%vNO FAILURES%v", 
                COLOR_GREEN, COLOR_RESET);
    }
    
    std::string answer;
    for(;;) {
        Logger::log()->info("Run another file? (in test/more/)");
        std::getline(std::cin, answer);
        if (answer == "" || answer == "n" || answer == "N") {
            break;
        }
        try {
            Script::Unique_Regref sandbox(Script::new_sandbox());
            std::stringstream sss;
            sss << "test/more/" << answer << ".lua";
            Script::Unique_Regref func(
                    Script::load_lua_function(
                            sss.str().c_str(), sandbox, sss.str().c_str()));
            Script::Helper::run_simple_function(func, 0);
            Logger::log()->info("%v\t...PASSED!%v", COLOR_GREEN, COLOR_RESET);
        }
        catch (std::runtime_error e) {
            Logger::log()->warn("%v\t...FAILED! %v%v", 
                    COLOR_RED, e.what(), COLOR_RESET);
        }
        Gensys::cleanup();
        Gensys::initialize();
        Gensys::LI::clear();
        lua_gc(Script::get_lua_state(), LUA_GCCOLLECT, 0);
    }
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
