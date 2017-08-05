#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <vector>

#include "pegr/engine/Engine.hpp"
#include "pegr/engine/App_State.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/script/Script_Helper.hpp"
#include "pegr/gensys/Lua_Interf.hpp"
#include "pegr/gensys/Gensys.hpp"
#include "pegr/logger/Logger.hpp"

using namespace pegr;

class Main_State : public Engine::App_State {
public:
    Main_State()
    : Engine::App_State("Main") {}
    virtual ~Main_State() {}
    
    virtual void initialize() override {
        Script::Unique_Regref sandbox(Script::new_sandbox());
        Script::Unique_Regref init_fun;
        Script::Unique_Regref postinit_fun;
        try {
            init_fun = Script::load_lua_function("init.lua", sandbox);
            postinit_fun = Script::load_lua_function("postinit.lua", sandbox);
        } catch (std::runtime_error e) {
            Logger::log()->warn(e.what());
        }
        
        try {
            Script::Helper::run_simple_function(init_fun, 0);
        } catch (std::runtime_error e) {
            Logger::log()->warn(e.what());
        }
        Gensys::LI::stage_all();
        Gensys::compile();
        try {
            Script::Helper::run_simple_function(postinit_fun, 0);
        } catch (std::runtime_error e) {
            Logger::log()->warn(e.what());
        }
        
        Engine::pop_state();
    }
};

int main() {
    Engine::initialize(Engine::INIT_FLAG_ALL);
    Engine::push_state(std::make_unique<Main_State>());
    Engine::run();
    Engine::cleanup();
    return 0;
}
