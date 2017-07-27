#ifndef PEGR_SCHEDULER_LUAINTERF_HPP
#define PEGR_SCHEDULER_LUAINTERF_HPP

#include "pegr/script/Script.hpp"

namespace pegr {
namespace Sched {
namespace LI {

void initialize();
void clear();
void cleanup();
    
int li_schedule_task(lua_State* l);
    
} // namspace LI
} // namespace Sched
} // namespace pegr

#endif // PEGR_SCHEDULER_LUAINTERF_HPP
