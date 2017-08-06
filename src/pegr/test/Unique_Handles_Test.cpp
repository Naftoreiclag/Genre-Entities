#include "pegr/render/Handle_Util.hpp"

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
