#ifndef PEGR_RENDER_SHADERS_HPP
#define PEGR_RENDER_SHADERS_HPP

#include "pegr/render/Handles.hpp"

namespace pegr {
namespace Render {
    
Unique_Shader load_shader(const char* filename);

Unique_Program load_program(const char* vert_fname, const char* frag_fname);


} // namespace Render
} // namespace pegr

#endif // PEGR_RENDER_SHADERS_HPP
