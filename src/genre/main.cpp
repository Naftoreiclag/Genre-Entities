#include "Script.hpp"

using namespace pegr;

int main() {
    Script::initialize();
    Script::Regref environment = Script::new_sandbox();
    Script::Regref script = Script::load_function("test.lua", environment);
    Script::run_function(script);
    Script::cleanup();
    return 0;
}
