/*
 * Copyright 2014-2015 Daniel Collin. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

/* This file is a heavily modified version of the original distributed with
 * bgfx. The main goal was to isolate just the bgfx-imgui render function.
 */

#include <bgfx/bgfx.h>
#include <bx/allocator.h>
#include <bx/math.h>
#include <bx/timer.h>
#include <ocornut-imgui/imgui.h>

#include "../bgfx_utils.h"
#include "imgui.h"

namespace Bexam_Imgui {
    
void render(ImDrawData* _drawData, uint8_t m_viewId, 
        bgfx::VertexDecl m_decl, 
        bgfx::TextureHandle m_texture, 
        bgfx::ProgramHandle m_program, 
        bgfx::ProgramHandle m_imageProgram,
        bgfx::UniformHandle s_tex,
        bgfx::UniformHandle u_imageLodEnabled)
{
    const ImGuiIO& io = ImGui::GetIO();
    const float width  = io.DisplaySize.x;
    const float height = io.DisplaySize.y;

    bgfx::setViewName(m_viewId, "ImGui");
    bgfx::setViewMode(m_viewId, bgfx::ViewMode::Sequential);

    const bgfx::HMD*  hmd  = bgfx::getHMD();
    const bgfx::Caps* caps = bgfx::getCaps();
    if (NULL != hmd && 0 != (hmd->flags & BGFX_HMD_RENDERING) )
    {
        float proj[16];
        bx::mtxProj(proj, hmd->eye[0].fov, 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);

        static float time = 0.0f;
        time += 0.05f;

        const float dist = 10.0f;
        const float offset0 = -proj[8] + (hmd->eye[0].viewOffset[0] / dist * proj[0]);
        const float offset1 = -proj[8] + (hmd->eye[1].viewOffset[0] / dist * proj[0]);

        float ortho[2][16];
        const float viewOffset = width/4.0f;
        const float viewWidth  = width/2.0f;
        bx::mtxOrtho(ortho[0], viewOffset, viewOffset + viewWidth, height, 0.0f, 0.0f, 1000.0f, offset0, caps->homogeneousDepth);
        bx::mtxOrtho(ortho[1], viewOffset, viewOffset + viewWidth, height, 0.0f, 0.0f, 1000.0f, offset1, caps->homogeneousDepth);
        bgfx::setViewTransform(m_viewId, NULL, ortho[0], BGFX_VIEW_STEREO, ortho[1]);
        bgfx::setViewRect(m_viewId, 0, 0, hmd->width, hmd->height);
    }
    else
    {
        float ortho[16];
        bx::mtxOrtho(ortho, 0.0f, width, height, 0.0f, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
        bgfx::setViewTransform(m_viewId, NULL, ortho);
        bgfx::setViewRect(m_viewId, 0, 0, uint16_t(width), uint16_t(height) );
    }

    // Render command lists
    for (int32_t ii = 0, num = _drawData->CmdListsCount; ii < num; ++ii)
    {
        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer tib;

        const ImDrawList* drawList = _drawData->CmdLists[ii];
        uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
        uint32_t numIndices  = (uint32_t)drawList->IdxBuffer.size();

        if (!checkAvailTransientBuffers(numVertices, m_decl, numIndices) )
        {
            // not enough space in transient buffer just quit drawing the rest...
            break;
        }

        bgfx::allocTransientVertexBuffer(&tvb, numVertices, m_decl);
        bgfx::allocTransientIndexBuffer(&tib, numIndices);

        ImDrawVert* verts = (ImDrawVert*)tvb.data;
        bx::memCopy(verts, drawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert) );

        ImDrawIdx* indices = (ImDrawIdx*)tib.data;
        bx::memCopy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx) );

        uint32_t offset = 0;
        for (const ImDrawCmd* cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
        {
            if (cmd->UserCallback)
            {
                cmd->UserCallback(drawList, cmd);
            }
            else if (0 != cmd->ElemCount)
            {
                uint64_t state = 0
                    | BGFX_STATE_RGB_WRITE
                    | BGFX_STATE_ALPHA_WRITE
                    | BGFX_STATE_MSAA
                    ;

                bgfx::TextureHandle th = m_texture;
                bgfx::ProgramHandle program = m_program;

                if (NULL != cmd->TextureId)
                {
                    union { ImTextureID ptr; struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; } texture = { cmd->TextureId };
                    state |= 0 != (IMGUI_FLAGS_ALPHA_BLEND & texture.s.flags)
                        ? BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
                        : BGFX_STATE_NONE
                        ;
                    th = texture.s.handle;
                    if (0 != texture.s.mip)
                    {
                        const float lodEnabled[4] = { float(texture.s.mip), 1.0f, 0.0f, 0.0f };
                        bgfx::setUniform(u_imageLodEnabled, lodEnabled);
                        program = m_imageProgram;
                    }
                }
                else
                {
                    state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
                }

                const uint16_t xx = uint16_t(bx::fmax(cmd->ClipRect.x, 0.0f) );
                const uint16_t yy = uint16_t(bx::fmax(cmd->ClipRect.y, 0.0f) );
                bgfx::setScissor(xx, yy
                        , uint16_t(bx::fmin(cmd->ClipRect.z, 65535.0f)-xx)
                        , uint16_t(bx::fmin(cmd->ClipRect.w, 65535.0f)-yy)
                        );

                bgfx::setState(state);
                bgfx::setTexture(0, s_tex, th);
                bgfx::setVertexBuffer(0, &tvb, 0, numVertices);
                bgfx::setIndexBuffer(&tib, offset, cmd->ElemCount);
                bgfx::submit(m_viewId, program);
            }

            offset += cmd->ElemCount;
        }
    }
}

} // namespace Bexam_Imgui
