#include "pegr/render/Shaders.hpp"

#include <stdexcept>
#include <cassert>

namespace pegr {
namespace Render {
    
Unique_Shader load_shader(const char* filename) {
    throw std::runtime_error("not implemented");
}

Unique_Program load_program(const char* vert_fname, const char* frag_fname) {
    throw std::runtime_error("not implemented");
}

} // namespace Render
} // namespace pegr
