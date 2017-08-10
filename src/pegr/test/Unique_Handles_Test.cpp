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

#include "pegr/render/Handles.hpp"

#include "pegr/test/Test_Util.hpp"

namespace pegr {
namespace Test {

//@Test Unique render handles templates
void test_0100_unique_render_handles() {
    // Test is technically done at compile time.
    
    Render::Unique_Dynamic_Index_Buffer  obj00;
    Render::Unique_Dynamic_Vertex_Buffer obj01;
    Render::Unique_Frame_Buffer          obj02;
    Render::Unique_Index_Buffer          obj03;
    Render::Unique_Indirect_Buffer       obj04;
    Render::Unique_Occlusion_Query       obj05;
    Render::Unique_Program               obj06;
    Render::Unique_Shader                obj07;
    Render::Unique_Texture               obj08;
    Render::Unique_Uniform               obj09;
    Render::Unique_Vertex_Buffer         obj10;
    //Render::Unique_Vertex_Decl           obj11;
}

//@Test Unique handle validity
void test_0100_unique_handle_validity() {
    Render::Unique_Shader shader;
    
    verify_equals(false, shader.is_valid());
}

} // namespace Test
} // namespace pegr
