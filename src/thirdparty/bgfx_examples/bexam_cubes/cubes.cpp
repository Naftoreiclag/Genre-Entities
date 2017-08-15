/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

/* This file contains code modified and extracted from the "01-cubes" example.
 * (`bgfx/examples/01-cubes/cubes.cpp` of https://github.com/bkaradzic/bgfx)
 */

#include "cubes.hpp"

#include <bx/pixelformat.h>

namespace Cubes_Example {

void PosColorVertex::init()
{
    ms_decl
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
        .end();
};

bgfx::VertexDecl PosColorVertex::ms_decl;

bgfx::VertexBufferHandle m_vbh;
bgfx::IndexBufferHandle m_ibh;

// Extracted from ExampleCubes::init
void pegr_init() {
    // Create vertex stream declaration.
    PosColorVertex::init();

    // Create static vertex buffer.
    m_vbh = bgfx::createVertexBuffer(
            // Static data can be passed with bgfx::makeRef
            bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices) )
            , PosColorVertex::ms_decl
            );

    // Create static index buffer.
    m_ibh = bgfx::createIndexBuffer(
            // Static data can be passed with bgfx::makeRef
            bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip) )
            );
}

// Extracted from ExampleCubes::update
void pegr_update(float time, float m_width, float m_height,
        bgfx::ProgramHandle m_program) {
    float at[3]  = { 0.0f, 0.0f,   0.0f };
    float eye[3] = { 0.0f, 0.0f, -35.0f };

    // Set view and projection matrix for view 0.
    const bgfx::HMD* hmd = bgfx::getHMD();
    if (NULL != hmd && 0 != (hmd->flags & BGFX_HMD_RENDERING) )
    {
        float view[16];
        bx::mtxQuatTranslationHMD(view, hmd->eye[0].rotation, eye);
        bgfx::setViewTransform(0, view, hmd->eye[0].projection, BGFX_VIEW_STEREO, hmd->eye[1].projection);

        // Set view 0 default viewport.
        //
        // Use HMD's width/height since HMD's internal frame buffer size
        // might be much larger than window size.
        bgfx::setViewRect(0, 0, 0, hmd->width, hmd->height);
    }
    else
    {
        float view[16];
        bx::mtxLookAt(view, eye, at);

        float proj[16];
        bx::mtxProj(proj, 60.0f, float(m_width)/float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(0, view, proj);

        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );
    }

    // This dummy draw call is here to make sure that view 0 is cleared
    // if no other draw calls are submitted to view 0.
    bgfx::touch(0);

    // Submit 11x11 cubes.
    for (uint32_t yy = 0; yy < 11; ++yy)
    {
        for (uint32_t xx = 0; xx < 11; ++xx)
        {
            float mtx[16];
            bx::mtxRotateXY(mtx, time + xx*0.21f, time + yy*0.37f);
            mtx[12] = -15.0f + float(xx)*3.0f;
            mtx[13] = -15.0f + float(yy)*3.0f;
            mtx[14] = 0.0f;

            // Set model matrix for rendering.
            bgfx::setTransform(mtx);

            // Set vertex and index buffer.
            bgfx::setVertexBuffer(0, m_vbh);
            bgfx::setIndexBuffer(m_ibh);

            // Set render states.
            bgfx::setState(0
                | BGFX_STATE_DEFAULT
                | BGFX_STATE_PT_TRISTRIP
                );

            // Submit primitive for rendering to view 0.
            bgfx::submit(0, m_program);
        }
    }
}

} // namespace Cubes_Example
