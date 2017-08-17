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

#ifndef PEGR_RENDER_SHADERS_HPP
#define PEGR_RENDER_SHADERS_HPP

#include "pegr/render/Handles.hpp"
#include "pegr/resource/Resources.hpp"

namespace pegr {
namespace Render {

const bgfx::Memory* read_file(const boost::filesystem::path& file);

/**
 * @brief Appends the proper subtype suffix to the OID, (#glsl, #spirv, etc)
 * @param oid
 * @return A new oid with the proper subtype suffix
 */
Resour::Oid add_platform_subtype(const Resour::Oid& oid);

/**
 * @brief Finds and loads a shader in bgfx
 * @param oid
 * @return A bgfx shader
 */
Shared_Shader find_shader(const Resour::Oid& oid);

/**
 * @brief Constructs a new shader program from a vertex and fragment shader
 * pair
 * @param vert_shader
 * @param frag_shader
 * @return A bgfx shader program
 */
Unique_Program make_program(
        const Shared_Shader& vert_shader, const Shared_Shader& frag_shader);

void clear_cached_shaders();
void clear_cached_programs();

} // namespace Render
} // namespace pegr

#endif // PEGR_RENDER_SHADERS_HPP
