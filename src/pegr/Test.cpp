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

#include <chrono>
#include <iostream>
#include <ratio>
#include <sstream>

#include "pegr/debug/Debug_Macros.hpp"
#include "pegr/engine/Engine.hpp"
#include "pegr/except/Except.hpp"
#include "pegr/gensys/Gensys.hpp"
#include "pegr/gensys/Lua_Interf.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/scheduler/Lua_Interf.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/script/Script_Util.hpp"
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
    const luaL_Reg test_api[] = {
        {"debug_stage_compile", li_debug_stage_compile},
        {"debug_collect_garbage", li_debug_collect_garbage},
        {"debug_timer_start", li_debug_timer_start},
        {"debug_timer_end", li_debug_timer_end},
        
        // Sentinel
        {nullptr, nullptr}
    };
    Script::multi_expose_c_functions(test_api);
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

bool run_test(const Test::NamedTest& test) {
    lua_State* l = Script::get_lua_state();
    int original_stack_size = lua_gettop(l);
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
            throw Except::Runtime(ss.str());
        }
        
        Logger::log()->info("%v\t...PASSED!%v", COLOR_GREEN, COLOR_RESET);
        return true;
    }
    catch (Except::Runtime& e) {
        Logger::log()->warn("%v\t...FAILED! %v%v", 
                COLOR_RED, e.what(), COLOR_RESET);
        return false;
    }
}

bool run_lua_test(const Test::NamedLuaTest& test) {
    log_header(test.m_name);
    bool success;
    try {
        Script::Unique_Regref sandbox(Script::new_sandbox());
        std::stringstream sss;
        sss << "test/tests/" << test.m_lua_file;
        Script::Unique_Regref func(
                Script::load_lua_function(
                        sss.str().c_str(), sandbox.get(), test.m_name));
        Script::Util::run_simple_function(func.get(), 0);
        Logger::log()->info("%v\t...PASSED!%v", COLOR_GREEN, COLOR_RESET);
        success = true;
    }
    catch (Except::Runtime& e) {
        Logger::log()->warn("%v\t...FAILED! %v%v", 
                COLOR_RED, e.what(), COLOR_RESET);
        success = false;
    }
    Gensys::cleanup();
    Gensys::initialize();
    Gensys::LI::clear();
    lua_gc(Script::get_lua_state(), LUA_GCCOLLECT, 0);
    return success;
}

std::vector<Test::NamedTest> n_failed_tests;
void run_tests(int& num_passes, int& num_fails) {
    for (std::size_t idx = 0; /*Until sentiel reached*/; ++idx) {
        Test::NamedTest test = Test::n_tests[idx];
        if (!test.m_name) {
            break;
        }
        bool success = run_test(test);
        if (success) {
            ++num_passes;
        } else {
            n_failed_tests.push_back(test);
            ++num_fails;
        }
    }
}

std::vector<Test::NamedLuaTest> n_failed_lua_tests;
void run_lua_tests(int& num_passes, int& num_fails) {
    for (std::size_t idx = 0; /*Until sentiel reached*/; ++idx) {
        Test::NamedLuaTest test = Test::n_lua_tests[idx];
        if (!test.m_name) {
            break;
        }
        bool success = run_lua_test(test);
        if (success) {
            ++num_passes;
        } else {
            n_failed_lua_tests.push_back(test);
            ++num_fails;
        }
    }
}

void run_extras() {
    std::string answer;
    lua_State* l = Script::get_lua_state();
    for(;;) {
        Logger::log()->info("Run another file? (in test/more/)");
        std::getline(std::cin, answer);
        if (answer == "" || answer == "n" || answer == "N") {
            break;
        }
        std::istringstream iss(answer);
        try {
            Script::Unique_Regref sandbox(Script::new_sandbox());
            std::string finput;
            iss >> finput;
            std::stringstream sss;
            sss << "test/more/" << finput << ".lua";
            Script::Unique_Regref func(
                    Script::load_lua_function(
                            sss.str().c_str(), 
                            sandbox.get(), 
                            sss.str().c_str()));
            Script::Util::run_simple_function(func.get(), 1);
            Script::Pop_Guard pg(1);
            
            if (lua_isfunction(l, -1)) {
                int num_args = 0;
                std::string arg;
                while (iss >> arg) {
                    lua_pushstring(l, arg.c_str());
                    ++ num_args;
                }
                Script::run_function(num_args, 0);
                pg.on_pop(1);
            }
            
            Logger::log()->info("%v\t...PASSED!%v", COLOR_GREEN, COLOR_RESET);
        }
        catch (Except::Runtime& e) {
            Logger::log()->warn("%v\t...FAILED! %v%v", 
                    COLOR_RED, e.what(), COLOR_RESET);
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
    if (n_failed_tests.size() > 0) {
        log_header("=== FAILED C++ TESTS", '=');
        for (Test::NamedTest test : n_failed_tests) {
            run_test(test);
        }
    }
    if (n_failed_lua_tests.size() > 0) {
        log_header("=== FAILED Lua TESTS", '=');
        for (Test::NamedLuaTest test : n_failed_lua_tests) {
            run_lua_test(test);
        }
    }
    log_header("===== RESULTS", '=');
    if (num_passes > 0) {
        Logger::log()->info("%v%v PASSED%v", 
                COLOR_GREEN, num_passes, COLOR_RESET);
    }
    if (num_fails > 0) {
        Logger::log()->info("%v%v FAILED%v", 
                COLOR_RED, num_fails, COLOR_RESET);
        if (n_failed_tests.size() > 0) {
            Logger::log()->info("%vFAILED C++ TESTS:%v", 
                    COLOR_RED, COLOR_RESET);
            for (Test::NamedTest test : n_failed_tests) {
                Logger::log()->info("%v\t%v%v", 
                        COLOR_RED, test.m_name, COLOR_RESET);
            }
        }
        if (n_failed_lua_tests.size() > 0) {
            Logger::log()->info("%vFAILED Lua TESTS:%v", 
                    COLOR_RED, COLOR_RESET);
            for (Test::NamedLuaTest test : n_failed_lua_tests) {
                Logger::log()->info("%v\t%v%v", 
                        COLOR_RED, test.m_name, COLOR_RESET);
            }
        }
    } else {
        Logger::log()->info("%vNO FAILURES%v", 
                COLOR_GREEN, COLOR_RESET);
    }
    run_extras();
}

class Test_State : public Engine::App_State {
public:
    Test_State()
    : Engine::App_State("Test") {}
    virtual ~Test_State() {}
    
    virtual void initialize() override {
        setup();
        run();
        Engine::pop_state();
    }
};

int main() {
    Engine::initialize(Engine::INIT_FLAG_LOGGER 
            | Engine::INIT_FLAG_GENSYS 
            | Engine::INIT_FLAG_SCRIPT 
            | Engine::INIT_FLAG_SCHEDU
            | Engine::INIT_FLAG_RESOUR);
    Engine::push_state(std::make_unique<Test_State>());
    Engine::run();
    Engine::cleanup();
    return 0;
}
