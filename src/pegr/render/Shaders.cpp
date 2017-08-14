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
#include <sstream>

#include "pegr/except/Except.hpp"
#include "pegr/winput/Enum_Utils.hpp"

namespace pegr {
namespace Render {
    
std::map<Resour::Oid, Shared_Shader> n_cached_shaders;
std::map<Resour::Oid, Shared_Program> n_cached_programs;
    
Resour::Oid add_platform_subtype(const Resour::Oid& oid) {
    bgfx::RendererType::Enum rt = bgfx::getRendererType();
    switch (rt) {
        case bgfx::RendererType::Direct3D9: return oid.get_subtype("dx9");
        case bgfx::RendererType::Direct3D11: return oid.get_subtype("dx11");
        case bgfx::RendererType::Direct3D12: return oid.get_subtype("dx11");
        case bgfx::RendererType::Gnm: return oid.get_subtype("pssl");
        case bgfx::RendererType::Metal: return oid.get_subtype("metal");
        case bgfx::RendererType::OpenGL: return oid.get_subtype("glsl");
        case bgfx::RendererType::OpenGLES: return oid.get_subtype("essl");
        case bgfx::RendererType::Vulkan: return oid.get_subtype("spirv");
        default: {
            std::stringstream sss;
            sss << "No shaders available for rendertype "
                << Winput::Util::to_string_bgfx_rt(rt);
            throw Except::Runtime(sss.str());
        }
    }
}

const bgfx::Memory* read_file(const boost::filesystem::path& file) {
    std::ifstream file_istr(file.string().c_str(), 
            std::ios::binary | std::ios::ate);
    std::ios::pos_type size = file_istr.tellg();
    std::vector<char> vec(size);
    file_istr.seekg(0, std::ios::beg);
    file_istr.read(vec.data(), size);
    if (file_istr.fail()) {
        std::stringstream sss;
        sss << "Failed to read from file: "
            << file;
        throw Except::Runtime(sss.str());
    }
    return bgfx::copy(vec.data(), vec.size());
}

Shared_Shader find_shader(const Resour::Oid& oid) {
    auto cache_iter = n_cached_shaders.find(oid);
    if (cache_iter != n_cached_shaders.end()) {
        return cache_iter->second;
    }
    Resour::Object obj = Resour::find_object(add_platform_subtype(oid), 
            Resour::Object::Type::SHADER);
    Shared_Shader shader = std::make_shared<const Unique_Shader>(
            bgfx::createShader(read_file(obj.m_fname)));
    n_cached_shaders.emplace(oid, shader);
    return shader;
}

Shared_Program make_program(
        const Shared_Shader& vert_shader, const Shared_Shader& frag_shader) {
    Shared_Program program = std::make_shared<const Unique_Program>(
            bgfx::createProgram(vert_shader->get(), frag_shader->get()));
    return program;
}

void clear_cached_shaders() {
    n_cached_shaders.clear();
}

void clear_cached_programs() {
    n_cached_programs.clear();
}

} // namespace Render
} // namespace pegr
