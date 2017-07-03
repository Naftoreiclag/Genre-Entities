#include "pegr/scheduler/SchedLuaInterface.hpp"

namespace pegr {
namespace Sched {
namespace LI {
    
int schedule_task(lua_State* l) {
    luaL_checktype(l, 2, LUA_TTABLE);
    std::size_t strlen;
    const char* strdata = luaL_checklstring(l, 1, &strlen);
    std::string key(strdata, strlen);
    return 0;
}
    
} // namspace LI
} // namespace Sched
} // namespace pegr
