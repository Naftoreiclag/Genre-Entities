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

#include "pegr/winput/Dbgui.hpp"

#include <bexam/common/imgui/imgui.h>
#include <ocornut-imgui/imgui.h>

#include "pegr/render/Shaders.hpp"
#include "pegr/winput/Winput.hpp"

namespace pegr {
namespace Winput {

const uint8_t DBGUI_VIEW_ID = 0xff;
    
void Dbgui::initialize() {
    ImGuiIO& imio = ImGui::GetIO();
    imio.IniFilename = "config/imgui.ini";
    
    m_prog = Render::make_program(
            Render::find_shader("ocornut_imgui.vs"), 
            Render::find_shader("ocornut_imgui.fs"));
    
    m_img_prog = Render::make_program(
            Render::find_shader("imgui_image.vs"), 
            Render::find_shader("imgui_image.fs"));
    
    m_lod_enabled_unif = bgfx::createUniform(
            "u_imageLodEnabled", bgfx::UniformType::Vec4);
    
    m_tex_unif = bgfx::createUniform(
            "s_tex", bgfx::UniformType::Int1);
    
    m_vertdecl.begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
            
    {
        unsigned char* out_pixels;
        int out_width, out_height;
        imio.Fonts->GetTexDataAsRGBA32(&out_pixels, &out_width, &out_height);
        m_tex = bgfx::createTexture2D(
                out_width, out_height, /* Size */
                false, /* Mipmapping */
                1, /* Layers */
                bgfx::TextureFormat::RGBA8, /* Format */
                BGFX_TEXTURE_NONE, /* Flags */
                bgfx::copy(out_pixels, out_width * out_height * 4));
    }
            
    m_initialized = true;
    
}
void Dbgui::new_frame() {
    ImGuiIO& imio = ImGui::GetIO();
    
    imio.RenderDrawListsFn = nullptr;
    imio.DisplaySize.x = Winput::get_window_width();
    imio.DisplaySize.y = Winput::get_window_height();
    imio.DeltaTime = 0;
    
    ImGui::NewFrame();
}
void Dbgui::render() {
    ImGui::Render();
    Bexam_Imgui::render(ImGui::GetDrawData(), DBGUI_VIEW_ID, m_vertdecl, 
            m_tex.get(), 
            m_prog.get(), m_img_prog.get(), 
            m_tex_unif.get(), m_lod_enabled_unif.get());
}
void Dbgui::cleanup() {
    
    m_tex.reset();
    m_vertdecl = bgfx::VertexDecl();
    m_tex_unif.reset();
    m_lod_enabled_unif.reset();
    m_img_prog.reset();
    m_prog.reset();
    
    ImGui::Shutdown();
    
    m_initialized = false;
}

Dbgui::~Dbgui() {
    if (m_initialized) {
        cleanup();
    }
}

} // namespace Winput
} // namespace pegr
