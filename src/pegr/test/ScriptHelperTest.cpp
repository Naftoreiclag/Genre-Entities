#include "pegr/script/ScriptHelper.hpp"
#include "pegr/script/Script.hpp"

namespace pegr {
namespace Test {

//@Test Script Helper
bool script_helper() {
    lua_State* l = Script::get_lua_state();
    return true;
}

}
}
