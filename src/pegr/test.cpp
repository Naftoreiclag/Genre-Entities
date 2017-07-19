#include <stdexcept>
#include <sstream>

#include "pegr/gensys/Gensys.hpp"
#include "pegr/gensys/Lua_Interf.hpp"
#include "pegr/scheduler/SchedLuaInterface.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/test/Tests.hpp"

using namespace pegr;

void setup() {
    Logger::initialize();
    Script::initialize();
    Gensys::initialize();
    Gensys::LI::initialize();
}

void run() {
    int num_passes = 0;
    int num_fails = 0;
    lua_State* l = Script::get_lua_state();
    for (std::size_t idx = 0; /*Until sentiel reached*/; ++idx) {
        int original_stack_size = lua_gettop(l);
        const Test::NamedTest& test = Test::m_tests[idx];
        if (!test.m_name) {
            break;
        }
        Logger::log()->info(test.m_name);
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
    Logger::log()->info("%v passed\t%v failed", num_passes, num_fails);
}

void cleanup() {
    Gensys::LI::initialize();
    Gensys::cleanup();
    Script::cleanup();
    Logger::cleanup();
}

int main() {
    setup();
    run();
    cleanup();
    return 0;
}
