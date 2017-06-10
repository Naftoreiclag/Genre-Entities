#include "Gensys.hpp"

#include <cstddef>
#include <iostream>

namespace pegr {
namespace Gensys {

int li_add_component(lua_State* l) {
    luaL_checktype(l, 2, LUA_TTABLE);
    std::size_t strlen;
    const char* luastr = luaL_checklstring(l, 1, &strlen);
    std::string name(luastr, strlen);
    
    std::cout << name << std::endl;
    
    if (name.length() > 5) {
        luaL_error(l, "Name exceeds length 5");
    }
    
    return 0;
}

} // namespace Gensys
} // namespace pegr
