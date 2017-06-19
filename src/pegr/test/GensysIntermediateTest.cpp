#include <sstream>
#include <iomanip>
#include <limits>

#include "pegr/gensys/GensysIntermediate.hpp"
#include "pegr/logger/Logger.hpp"

namespace pegr {
namespace Test {

//@Test Gensys Intermediates
bool test_gensys_primitive_test() {
    lua_State* l = Script::get_lua_state();
    int stack_size = lua_gettop(l);
    
    Gensys::Interm::Prim prim;
    
    if (!prim.is_error()) {
        Logger::log()->warn("Prim is not an error!");
        return false;
    }
    
    prim.set_f32(0.5f);
    
    if (prim.get_type() != Gensys::Interm::Prim::Type::F32) {
        Logger::log()->warn("Type must be F32!");
        return false;
    }
    
    double pi = 3.14159265358979323846264338328d;
    {    
        float pi_f = pi;
        
        std::stringstream ss1;
        ss1 << std::setprecision(100) << pi;
        Logger::log()->info("Double precision: %v", ss1.str());
        std::stringstream ss2;
        ss2 << std::setprecision(100) << pi_f;
        Logger::log()->info("Float precision: %v", ss2.str());
        
        Logger::log()->info("Size of float: %v", sizeof(float));
        Logger::log()->info("Size of double: %v", sizeof(double));
        Logger::log()->info("Size of Prim: %v", sizeof(Gensys::Interm::Prim));
    }
    prim.set_f64(pi);
    
    if (prim.get_type() != Gensys::Interm::Prim::Type::F64) {
        Logger::log()->warn("Type must be F64!");
        return false;
    }
    
    if (prim.get_f64() != pi) {
        Logger::log()->warn("Wrong double: %v", prim.get_f64());
        return false;
    }
    std::stringstream ss;
    ss << prim.get_f64();
    Logger::log()->info("Correct double: %v", ss.str());
    
    Script::Regref_Guard sandbox(Script::new_sandbox());
    Script::Regref table_fun = 
            Script::load_lua_function("test/simple_table.lua", sandbox);
    
    prim.set_function(Script::make_shared(table_fun));
    
    if (prim.get_type() != Gensys::Interm::Prim::Type::FUNC) {
        Logger::log()->warn("Type must be function!");
        return false;
    }
    
    Script::run_function(*prim.get_function(), 0, 1);
    lua_getfield(l, -1, "a");
    
    std::size_t strsize;
    const char* strdata = lua_tolstring(l, -1, &strsize);
    
    std::string a_val(strdata, strsize);
    
    if (a_val != "apple") {
        Logger::log()->warn("Loaded function failed: %v", a_val);
        return false;
    }
    Logger::log()->info("Correct function return val: %v", a_val);
    
    lua_pop(l, 1);
    lua_pop(l, 1);
    
    std::string str = "Hello world!";
    
    prim.set_string(str);
    
    if (prim.get_type() != Gensys::Interm::Prim::Type::STR) {
        Logger::log()->warn("Type must be string!");
        return false;
    }
    
    if (prim.get_string() != str) {
        Logger::log()->warn("Wrong string returned: %v", prim.get_string());
        return false;
    }
    Logger::log()->info("Correct string retrieved: %v", prim.get_string());
    
    if (lua_gettop(l) != stack_size) {
        Logger::log()->warn("Unbalanced!");
        return false;
    }
}

}
}
