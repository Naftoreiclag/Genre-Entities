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
namespace Schedu {
namespace LI {
    
class Scripted_Event : public Event {
public:
    Scripted_Event();
    virtual ~Scripted_Event();
    
    // TODO

    virtual Type get_type() const override;
    virtual void trigger() override;
};

void initialize();
void clear();
void cleanup();

int li_add_event(lua_State* l);
int li_edit_event(lua_State* l);
    
int li_call_event(lua_State* l);
int li_hook_listener(lua_State* l);

} // namspace LI
} // namespace Schedu
} // namespace pegr

#endif // PEGR_SCHEDULER_LUAINTERF_HPP
