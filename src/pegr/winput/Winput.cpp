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

#include "pegr/winput/Winput.hpp"

#include <cstdint>
#include <sstream>

#include <SDL.h>
#include <SDL_syswm.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include "pegr/engine/Engine.hpp"
#include "pegr/except/Except.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/render/Shaders.hpp"
#include "pegr/winput/Dbgui.hpp"
#include "pegr/winput/Enum_Utils.hpp"

namespace pegr {
namespace Winput {

const char* const WINDOW_DEFAULT_TITLE = "Engine";
const int32_t WINDOW_DEFAULT_WIDTH = 640;
const int32_t WINDOW_DEFAULT_HEIGHT = 480;

extern const std::uint32_t MOUSE_BUTTON_LEFT = SDL_BUTTON_LMASK;
extern const std::uint32_t MOUSE_BUTTON_MIDDLE = SDL_BUTTON_MMASK;
extern const std::uint32_t MOUSE_BUTTON_RIGHT = SDL_BUTTON_RMASK;
extern const std::uint32_t MOUSE_BUTTON_EXTRA_1 = SDL_BUTTON_X1MASK;
extern const std::uint32_t MOUSE_BUTTON_EXTRA_2 = SDL_BUTTON_X2MASK;

Dbgui n_dbgui;

SDL_Window* n_window;
SDL_SysWMinfo n_syswm_info;

std::int32_t n_mouse_x = 0;
std::int32_t n_mouse_y = 0;

std::int32_t get_mouse_x() { return n_mouse_x; }
std::int32_t get_mouse_y() { return n_mouse_y; }

std::uint32_t get_mouse_buttons() {
    return SDL_GetMouseState(nullptr, nullptr);
}

std::int32_t n_window_width = WINDOW_DEFAULT_WIDTH;
std::int32_t n_window_height = WINDOW_DEFAULT_HEIGHT;

std::int32_t get_window_width() { return n_window_width; }
std::int32_t get_window_height() { return n_window_height; }

bgfx::PlatformData extract_plat_specific(const SDL_SysWMinfo& syswm_info) {
    bgfx::PlatformData plat_specific;
    plat_specific.backBuffer = nullptr;
    plat_specific.backBufferDS = nullptr;
    plat_specific.context = nullptr;
    plat_specific.ndt = nullptr;
    plat_specific.nwh = nullptr;
    plat_specific.session = nullptr;
    
    bool success = false;
    
    #if defined(SDL_VIDEO_DRIVER_WINDOWS)
        Logger::log()->info("Using Windows driver");
        plat_specific.nwh = n_syswm_info.info.win.window;
        success = true;
    #elif defined(SDL_VIDEO_DRIVER_X11)
        Logger::log()->info("Using X11 driver");
        plat_specific.ndt = n_syswm_info.info.x11.display;
        plat_specific.nwh = n_syswm_info.info.x11.window;
        success = true;
    #elif defined( SDL_VIDEO_DRIVER_COCOA)
        Logger::log()->info("Using Cocoa driver");
        plat_specific.nwh = n_syswm_info.info.coca.window;
        success = true;
    #elif defined( SDL_VIDEO_DRIVER_VIVANTE)
        Logger::log()->info("Using Vivante driver");
        plat_specific.ndt = n_syswm_info.info.vivante.display;
        plat_specific.nwh = n_syswm_info.info.vivante.window;
        success = true;
    #endif
    
    if (!success) {
        std::stringstream sss;
        sss << "Binding bgfx to "
            << Util::to_string_syswm_type(n_syswm_info.subsystem)
            << " currently unimplemented";
        throw Except::Runtime(sss.str());
    }
    
    return plat_specific;
}

void initialize() {
    Logger::log()->info("Initializing window and input");
    
    if (SDL_Init(0) < 0) {
        std::stringstream sss;
        sss << "Could not initalize SDL: "
            << SDL_GetError();
        throw Except::Runtime(sss.str());
    }
    
    // Create the window
    n_window = SDL_CreateWindow(WINDOW_DEFAULT_TITLE, 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        n_window_width, n_window_height, 
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN);
    {
        int width, height;
        SDL_GetWindowSize(n_window, &width, &height);
        n_window_width = width;
        n_window_height = height;
    }
    if (!n_window) {
        throw Except::Runtime("Could not create SDL window");
    }
    
    SDL_version version;
    SDL_VERSION(&version);
    
    Logger::log()->info("SDL version %v.%v.%v", 
            (int32_t) version.major, 
            (int32_t) version.minor, 
            (int32_t) version.patch);
    
    n_syswm_info.version = version;
    
    if (!SDL_GetWindowWMInfo(n_window, &n_syswm_info)) {
        std::stringstream sss;
        sss << "Could not retrieve window info from SDL: "
            << SDL_GetError();
        throw Except::Runtime(sss.str());
    }
    
    Logger::log()->info("Running SDL on %v", 
            Util::to_string_syswm_type(n_syswm_info.subsystem));
    
    bgfx::PlatformData pdat = extract_plat_specific(n_syswm_info);
    Logger::log()->info("Applying platform data for bgfx");
    bgfx::setPlatformData(pdat);
    
    if (!bgfx::init(bgfx::RendererType::Count, BGFX_PCI_ID_NONE)) {
        throw Except::Runtime("Failed to init bgfx");
    }
    bgfx::reset(n_window_width, n_window_height, 
            BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X4);
    SDL_ShowWindow(n_window);
    
    Logger::log()->info("bgfx renderer type: %v", 
            Util::to_string_bgfx_rt(bgfx::getRendererType()));
    
    n_dbgui.initialize();
}

void on_sdl_mouse_button(const SDL_MouseButtonEvent& button) {
    // ...
}

void on_sdl_mouse_motion(const SDL_MouseMotionEvent& motion) {
    n_mouse_x = motion.x;
    n_mouse_y = motion.y;
}

void on_sdl_window_resize(const SDL_WindowEvent& window) {
    n_window_width = window.data1;
    n_window_height = window.data2;
    Logger::log()->info("Window resized to %vx%v", 
            n_window_width, n_window_height);
    bgfx::reset(n_window_width, n_window_height, 
            BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X4);
    Engine::on_window_resize(n_window_width, n_window_height);
}

void on_sdl_quit(const SDL_QuitEvent& quit) {
    Engine::quit();
}

void pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEBUTTONDOWN: {
                on_sdl_mouse_button(event.button);
                break;
            }
            case SDL_MOUSEMOTION: {
                on_sdl_mouse_motion(event.motion);
                break;
            }
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED: {
                        on_sdl_window_resize(event.window);
                        break;
                    }
                }
                break;
            }
            case SDL_QUIT: {
                on_sdl_quit(event.quit);
                break;
            }
            default: break;
        }
    }
}
void cleanup() {
    
    Logger::log()->info("Cleaning window and input");
    
    n_dbgui.cleanup();
    
    Render::clear_cached_programs();
    Render::clear_cached_shaders();
    
    bgfx::shutdown();
    
    n_window_width = WINDOW_DEFAULT_WIDTH;
    n_window_height = WINDOW_DEFAULT_HEIGHT;
    
    SDL_DestroyWindow(n_window);
    SDL_Quit();
}

void pre_frame() {
    n_dbgui.new_frame();
}

void submit_frame() {
    bgfx::touch(0);
    n_dbgui.render();
    bgfx::frame();
}

} // namespace Winput
} // namespace pegr
