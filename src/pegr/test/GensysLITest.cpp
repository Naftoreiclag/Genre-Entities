#include <stdexcept>

#include "pegr/script/ScriptHelper.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/gensys/Gensys.hpp"
#include "pegr/gensys/GensysIntermediate.hpp"
#include "pegr/gensys/GensysLuaInterface.hpp"

namespace pegr {
namespace Test {

//@Test Gensys primitive from Lua values
void test_0080_00_gensys_primitive() {
    Gensys::Interm::Prim prim;
    
    lua_State* l = Script::get_lua_state();
    
    Script::Regref_Guard sandbox(Script::new_sandbox());
    Script::Regref_Guard func(
        Script::load_lua_function("test/prim_cstr_table.lua", sandbox));
        
    Script::Helper::run_simple_function(func, 1);
    
    prim = Gensys::LI::translate_primitive(-1);
    lua_pop(l, 1);
    
    if (prim.get_type() != Gensys::Interm::Prim::Type::FUNC) {
        throw std::runtime_error("Expected FUNC!");
    }
}

} // namespace Test
} // namespace pegr