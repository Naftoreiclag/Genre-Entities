/*
 *  Copyright 2017 James Fong
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

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
