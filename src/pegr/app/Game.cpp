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

#include "pegr/app/Game.hpp"

#include <cassert>
#include <algorithm>
#include <vector>

#include <bgfx/bgfx.h>

#include "pegr/engine/Engine.hpp"
#include "pegr/engine/App_State.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/script/Script_Util.hpp"
#include "pegr/gensys/Lua_Interf.hpp"
#include "pegr/gensys/Gensys.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/winput/Winput.hpp"
#include "pegr/except/Except.hpp"

namespace pegr {
namespace App {

Game_State::Game_State()
: Engine::App_State("Main") {}
Game_State::~Game_State() {}

void Game_State::initialize() {
    Script::Unique_Regref sandbox(Script::new_sandbox());
    Script::Unique_Regref init_fun;
    Script::Unique_Regref postinit_fun;
    try {
        init_fun = Script::load_lua_function("init.lua", sandbox);
        postinit_fun = Script::load_lua_function("postinit.lua", sandbox);
    } catch (Except::Runtime e) {
        Logger::log()->warn(e.what());
    }
    
    try {
        Script::Util::run_simple_function(init_fun, 0);
    } catch (Except::Runtime e) {
        Logger::log()->warn(e.what());
    }
    Gensys::LI::stage_all();
    Gensys::compile();
    try {
        Script::Util::run_simple_function(postinit_fun, 0);
    } catch (Except::Runtime e) {
        Logger::log()->warn(e.what());
    }
    
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
            0x009e79ff, 1.f, 0);
    bgfx::setDebug(BGFX_DEBUG_TEXT);
    bgfx::frame();
    bgfx::setViewRect(0, 0, 0, 
            Winput::get_window_width(), 
            Winput::get_window_height());
    
}

void Game_State::on_frame() {
    bgfx::touch(0);
    bgfx::dbgTextClear();
    bgfx::dbgTextPrintf(0, 0, 0x0f, "Hello world");
    bgfx::frame();
}

void Game_State::on_window_resize(int32_t width, int32_t height) {
    bgfx::setViewRect(0, 0, 0, width, height);
    
}

} // namespace App
} // namespace pegr
