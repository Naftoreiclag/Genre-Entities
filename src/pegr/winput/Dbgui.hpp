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

#ifndef PEGR_WINPUT_DBGUI_HPP
#define PEGR_WINPUT_DBGUI_HPP

#include <cstdint>

#include <bgfx/bgfx.h>

#include "pegr/render/Handles.hpp"

namespace pegr {
namespace Winput {

extern const uint8_t DBGUI_VIEW_ID;
    
class Dbgui {
public:
    void initialize();
    void new_frame();
    void render();
    void cleanup();
    
    ~Dbgui();

private:
    Render::Unique_Program m_prog;
    Render::Unique_Program m_img_prog;
    Render::Unique_Uniform m_lod_enabled_unif;
    Render::Unique_Uniform m_tex_unif;
    bgfx::VertexDecl m_vertdecl;
    Render::Unique_Texture m_tex;
    bool m_initialized;
};

} // namespace Winput
} // namespace pegr

#endif // PEGR_WINPUT_DBGUI_HPP
