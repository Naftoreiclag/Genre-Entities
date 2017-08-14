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
    
Resour::Oid add_platform_subtype(const Resour::Oid& oid);
    
Shared_Shader find_shader(const Resour::Oid& oid, bool do_cache = true);

Shared_Program find_program(
        const Resour::Oid& vert_oid, const Resour::Oid& frag_oid);

void clear_cached_shaders();
void clear_cached_programs();

} // namespace Render
} // namespace pegr

#endif // PEGR_RENDER_SHADERS_HPP
