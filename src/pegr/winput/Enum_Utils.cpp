#include "pegr/winput/Enum_Utils.hpp"

namespace pegr {
namespace Winput {
namespace Util {

const char* to_string_syswm_type(SDL_SYSWM_TYPE t) {
    switch (t) {
        case SDL_SYSWM_WINDOWS: return "Windows";
        case SDL_SYSWM_X11: return "X11";
        case SDL_SYSWM_DIRECTFB: return "DirectFB";
        case SDL_SYSWM_COCOA: return "Cocoa";
        case SDL_SYSWM_UIKIT: return "UIKit";
        case SDL_SYSWM_WAYLAND: return "Wayland";
        case SDL_SYSWM_WINRT: return "WinRT";
        case SDL_SYSWM_ANDROID: return "Android";
        case SDL_SYSWM_VIVANTE: return "Vivante";
        default: return "Unknown";
    }
}

const char* to_string_bgfx_rt(bgfx::RendererType::Enum rent) {
    switch (rent) {
        case bgfx::RendererType::Noop: return "Noop (No rendering)";
        case bgfx::RendererType::Direct3D9: return "Direct3D 9";
        case bgfx::RendererType::Direct3D11: return "Direct3D 11";
        case bgfx::RendererType::Direct3D12: return "Direct3D 12";
        case bgfx::RendererType::Gnm: return "GNM";
        case bgfx::RendererType::Metal: return "Metal";
        case bgfx::RendererType::OpenGL: return "OpenGL";
        case bgfx::RendererType::OpenGLES: return "OpenGLES";
        case bgfx::RendererType::Vulkan: return "Vulkan";
        default: return "Unknown";
    }
}
    
} // namespace Util
} // namespace Winput
} // namespace pegr
