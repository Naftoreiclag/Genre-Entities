/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

/* This file contains code modified and extracted from the "01-cubes" example.
 * (`bgfx/examples/01-cubes/cubes.cpp` of https://github.com/bkaradzic/bgfx)
 */

#include <cstdint>

#include <bgfx/bgfx.h>

namespace Cubes_Example {

struct PosColorVertex
{
    float m_x;
    float m_y;
    float m_z;
    uint32_t m_abgr;

    static void init();

    static bgfx::VertexDecl ms_decl;
};

const PosColorVertex s_cubeVertices[] =
{
    {-1.0f,  1.0f,  1.0f, 0xff000000 },
    { 1.0f,  1.0f,  1.0f, 0xff0000ff },
    {-1.0f, -1.0f,  1.0f, 0xff00ff00 },
    { 1.0f, -1.0f,  1.0f, 0xff00ffff },
    {-1.0f,  1.0f, -1.0f, 0xffff0000 },
    { 1.0f,  1.0f, -1.0f, 0xffff00ff },
    {-1.0f, -1.0f, -1.0f, 0xffffff00 },
    { 1.0f, -1.0f, -1.0f, 0xffffffff },
};

const uint16_t s_cubeTriList[] =
{
    0, 1, 2, // 0
    1, 3, 2,
    4, 6, 5, // 2
    5, 6, 7,
    0, 2, 4, // 4
    4, 2, 6,
    1, 5, 3, // 6
    5, 7, 3,
    0, 4, 1, // 8
    4, 5, 1,
    2, 3, 6, // 10
    6, 3, 7,
};

const uint16_t s_cubeTriStrip[] =
{
    0, 1, 2,
    3,
    7,
    1,
    5,
    0,
    4,
    2,
    6,
    7,
    4,
    5,
};

extern bgfx::VertexBufferHandle m_vbh;
extern bgfx::IndexBufferHandle m_ibh;

// Extracted from ExampleCubes::init
void pegr_init();

// Extracted from ExampleCubes::update
void pegr_update(float time, float m_width, float m_height,
        bgfx::ProgramHandle m_program);

} // namespace Cubes_Example
