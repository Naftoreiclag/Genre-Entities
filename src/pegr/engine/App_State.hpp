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

#ifndef PEGR_ENGINE_APPSTATE_HPP
#define PEGR_ENGINE_APPSTATE_HPP

#include <cstdint>

namespace pegr {
namespace Engine {

class App_State {
public:
    App_State(const char* dbg_name);
    virtual ~App_State();
    
    virtual void initialize();
    
    virtual void pause(App_State* to);
    virtual void unpause(App_State* from);
    
    virtual void on_frame();
    virtual void on_window_resize(int32_t width, int32_t height);
    
    virtual void cleanup();
    
    const char* get_dbg_name();
    
private:
    const char* m_dbg_name;
};

} // namespace pegr
} // namespace Engine

#endif // PEGR_ENGINE_APPSTATE_HPP
