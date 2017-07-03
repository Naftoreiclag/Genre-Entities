#ifndef PEGR_SCHEDULER_SCHEDLUAINTERFACE_HPP
#define PEGR_SCHEDULER_SCHEDLUAINTERFACE_HPP

#include "pegr/script/Script.hpp"

namespace pegr {
namespace Sched {
namespace LI {
    
int schedule_task(lua_State* l);
    
} // namspace LI
} // namespace Sched
} // namespace pegr

#endif // PEGR_SCHEDULER_SCHEDLUAINTERFACE_HPP
