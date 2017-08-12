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

#include "pegr/render/Shaders.hpp"

#include <cassert>

#include "pegr/except/Except.hpp"

namespace pegr {
namespace Render {
    
Unique_Shader load_shader(const char* filename) {
    throw Except::Runtime("not implemented");
}

Unique_Program load_program(const char* vert_fname, const char* frag_fname) {
    throw Except::Runtime("not implemented");
}

} // namespace Render
} // namespace pegr
