#include "pegr/script/Lua_Interf_Util.hpp"

#include <algorithm>
#include <stdexcept>
#include <cassert>

#include "pegr/debug/Debug_Macros.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/script/Script_Util.hpp"
#include "pegr/logger/Logger.hpp"

namespace pegr {
namespace Script {
namespace Util {

void generic_li_add_to_res_table(lua_State* l, Regref registry, 
        std::function<void(lua_State*)> post_copy) {
    // Make sure argument 2 is a table
    luaL_checktype(l, 2, LUA_TTABLE);
    
    // Get the string from argument 1
    std::size_t strlen;
    const char* strdata = luaL_checklstring(l, 1, &strlen);
    std::string key(strdata, strlen);
    
    // TODO: resolve namespace issues in key
    
    // Push the target table onto the stack
    Script::push_reference(registry);
    // Push the key
    lua_pushstring(l, key.c_str());
    // Push the value (a simple copy of the table argument)
    Script::Util::simple_deep_copy(2);
    Script::Pop_Guard pg(3);
    
    // Can throw exception...
    post_copy(l);
    
    // Set (pops the key and value)
    lua_settable(l, -3);
    pg.on_pop(2);
}
void generic_li_add_to_res_table(lua_State* l, Regref registry) {
    generic_li_add_to_res_table(l, registry, [](lua_State* l){});
}

void generic_li_edit_from_res_table(lua_State* l, Regref registry) {
    // Get the string from argument 1
    std::size_t strlen;
    const char* strdata = luaL_checklstring(l, 1, &strlen);
    std::string key(strdata, strlen);
    
    // TODO: resolve namespace issues in key
    
    // Push the target table onto the stack
    Script::push_reference(registry);
    
    // Push the key
    lua_pushstring(l, key.c_str());
    
    // Get the value
    lua_gettable(l, -2);
    
    // Remove the target table
    lua_remove(l, -2);
}

} // namespace Util
} // namespace Script
} // namespace pegr
