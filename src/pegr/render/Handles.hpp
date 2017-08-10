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

#ifndef PEGR_RENDER_HANDLES_HPP
#define PEGR_RENDER_HANDLES_HPP

#include <cassert>

#include <bgfx/bgfx.h>

namespace pegr {
namespace Render {

/**
 * @class Unique_Handle
 * @brief Template for capturing bgfx EtcHandles into unique_ptr-like RAII
 * exception-safe objects.
 */
template<typename T>
class Unique_Handle {
public:
    /**
     * @brief Guards nothing
     */
    Unique_Handle() { 
        make_invalid(m_handle);
        assert(!bgfx::isValid(m_handle));
    }
    
    /**
     * @brief Guards the given reference. When this guard is deleted (for
     * example, by going out of scope) the reference is freed from the lua
     * registry, possibly causing that value to be gc'd.
     */
    Unique_Handle(T handle)
    : m_handle(handle) {}
    
    /**
     * @brief Copy construction not allowed
     * (there should only be one guard for a single reference)
     */
    Unique_Handle(const Unique_Handle<T>& rhs) = delete;
    
    /**
     * @brief Copy assignment not allowed 
     * (there should only be one guard for a single reference)
     */
    Unique_Handle& operator =(const Unique_Handle<T>& rhs) = delete;
    
    /**
     * @brief Move construction
     */
    Unique_Handle(Unique_Handle<T>&& rhs) {
        m_handle = rhs.m_handle;
        make_invalid(rhs.m_handle);
    }
    
    /**
     * @brief Move assignment
     */
    Unique_Handle& operator=(Unique_Handle<T>&& rhs) {
        // Either the references are different or both are invalid
        assert(!handles_equal(m_handle, rhs.m_handle) || 
                (!bgfx::isValid(m_handle) && !bgfx::isValid(rhs)));
        
        reset();
        m_handle = rhs.m_handle;
        make_invalid(rhs.m_handle);
        return *this;
    }
    
    /**
     * @brief Deconstructor. Drops whatever reference it is guarding.
     */
    ~Unique_Handle() {
        reset();
        assert(!is_valid());
    }
    
    /**
     * @return the handle
     */
    T get() const {
        return m_handle;
    }
    
    /**
     * @return If the handle is valid
     */
    bool is_valid() const {
        return bgfx::isValid(m_handle);
    }
    
    /**
     * @brief Replaces the current handle with another. Properly releases
     * previous handle
     * @param handle The replacement handle
     */
    void reset(T handle) {
        if (bgfx::isValid(m_handle)) {
            bgfx::destroy(m_handle);
        }
        m_handle = handle;
    }
    
    void reset() {
        if (bgfx::isValid(m_handle)) {
            bgfx::destroy(m_handle);
        }
        make_invalid(m_handle);
    }
    
    /**
     * @brief Releases ownership of the held handle (does not call deleter) and 
     * returns it
     * @return The formerly guarded handle
     */
    T release() {
        T old = m_handle;
        make_invalid(m_handle);
        return old;
    }
    
private:
    T m_handle;

    static bool handles_equal(const T& rhs, const T& lhs) {
        return rhs.idx == lhs.idx;
    }
    
    static void make_invalid(T& handle) {
        handle.idx = bgfx::kInvalidHandle;
        assert(!bgfx::isValid(handle));
    }
};

typedef Unique_Handle<bgfx::DynamicIndexBufferHandle> 
        Unique_Dynamic_Index_Buffer;
typedef Unique_Handle<bgfx::DynamicVertexBufferHandle> 
        Unique_Dynamic_Vertex_Buffer;
typedef Unique_Handle<bgfx::FrameBufferHandle> Unique_Frame_Buffer;
typedef Unique_Handle<bgfx::IndexBufferHandle> Unique_Index_Buffer;
typedef Unique_Handle<bgfx::IndirectBufferHandle> Unique_Indirect_Buffer;
typedef Unique_Handle<bgfx::OcclusionQueryHandle> Unique_Occlusion_Query;
typedef Unique_Handle<bgfx::ProgramHandle> Unique_Program;
typedef Unique_Handle<bgfx::ShaderHandle> Unique_Shader;
typedef Unique_Handle<bgfx::TextureHandle> Unique_Texture;
typedef Unique_Handle<bgfx::UniformHandle> Unique_Uniform;
typedef Unique_Handle<bgfx::VertexBufferHandle> Unique_Vertex_Buffer;
//typedef Unique_Handle<bgfx::VertexDeclHandle> Unique_Vertex_Decl; // unused

} // namespace Render
} // namespace pegr

#endif // PEGR_RENDER_HANDLES_HPP
