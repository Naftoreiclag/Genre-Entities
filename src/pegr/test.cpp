#include <stdexcept>

#include "pegr/gensys/Gensys.hpp"
#include "pegr/gensys/GensysLuaInterface.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/test/Tests.hpp"

using namespace pegr;

void setup_logger() {
    Logger::initialize();
}

void setup_scripting() {
    Script::initialize();
}

void setup_gensys() {
    Gensys::initialize();
    const luaL_Reg api_safe[] = {
        {"add_archetype", Gensys::LI::add_archetype},
        {"edit_archetype", Gensys::LI::edit_archetype},
        {"add_genre", Gensys::LI::add_genre},
        {"edit_genre", Gensys::LI::edit_genre},
        {"add_component", Gensys::LI::add_component},
        {"edit_component", Gensys::LI::edit_component},
        
        // End of the list
        {nullptr, nullptr}
    };
    Script::multi_expose_c_functions(api_safe);
}

void setup() {
    setup_logger();
    setup_scripting();
    setup_gensys();
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
                throw std::runtime_error("Mandatory: Unbalanced test!");
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
