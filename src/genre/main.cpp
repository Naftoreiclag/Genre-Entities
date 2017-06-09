#include <iostream>

#include "Gensys.hpp"
#include "Script.hpp"

using namespace pegr;

int test(lua_State* l) {
    std::cout << "Hello from C++" << std::endl;
    return 0;
}

int main() {
    Script::initialize();
    Script::Regref func = Script::load_c_function(test);
    Script::add_to_pegr_table("test", func);
    std::cout << "asdfasdf" << std::endl;
    
    
    Script::Regref environment = Script::new_sandbox();
    Script::Regref script = Script::load_lua_function("test.lua", environment);
    Script::run_function(script);
    /*
    Script::run_function(script);
    std::cout << sizeof(std::string) << std::endl;
    std::cout << sizeof(float) << std::endl;
    std::cout << sizeof(double) << std::endl;
    */
    /*
    const char* deg;
    {
        std::string egg;
        std::getline(std::cin, egg);
        deg = egg.c_str();
    }
    std::cout << deg << std::endl;
    */
    Script::cleanup();
    return 0;
}
