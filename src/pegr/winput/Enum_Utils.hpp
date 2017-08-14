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

#ifndef PEGR_WINPUT_ENUMUTILS_HPP
#define PEGR_WINPUT_ENUMUTILS_HPP

#include <SDL.h>
#include <SDL_syswm.h>
#include <bgfx/bgfx.h>

namespace pegr {
namespace Winput {
namespace Util {

const char* to_string_syswm_type(SDL_SYSWM_TYPE t);
const char* to_string_bgfx_rt(bgfx::RendererType::Enum rt);
    
} // namespace Util
} // namespace Winput
} // namespace pegr

#endif // PEGR_WINPUT_ENUMUTILS_HPP
