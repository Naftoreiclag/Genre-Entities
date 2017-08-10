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

#ifndef PEGR_APP_GAME_HPP
#define PEGR_APP_GAME_HPP

#include <bgfx/bgfx.h>

#include "pegr/engine/App_State.hpp"
#include "pegr/render/Handles.hpp"

namespace pegr {
namespace App {

class Game_State : public Engine::App_State {
public:
    Game_State();
    virtual ~Game_State();
    virtual void initialize() override;
    
    virtual void on_frame() override;
    virtual void on_window_resize(int32_t width, int32_t height) override;
    
private:
    Render::Unique_Program m_shader_prog;
};

} // namespace App
} // namespace pegr

#endif // PEGR_APP_GAME_HPP
