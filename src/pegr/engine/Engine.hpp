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

#ifndef PEGR_ENGINE_ENGINE_HPP
#define PEGR_ENGINE_ENGINE_HPP

#include <cstdint>
#include <memory>

#include "pegr/engine/App_State.hpp"

namespace pegr {
namespace Engine {

extern const uint16_t INIT_FLAG_LOGGER;
extern const uint16_t INIT_FLAG_SCRIPT;
extern const uint16_t INIT_FLAG_GENSYS;
extern const uint16_t INIT_FLAG_SCHEDU;
extern const uint16_t INIT_FLAG_WINPUT;
extern const uint16_t INIT_FLAG_RESOUR;
extern const uint16_t INIT_FLAG_ALL;
extern const uint16_t INIT_FLAG_NONE;

bool logger_used();
bool script_used();
bool gensys_used();
bool schedu_used();
bool winput_used();
bool resour_used();

void initialize(uint16_t flags = INIT_FLAG_ALL);
void push_state(std::unique_ptr<App_State>&& state);
std::unique_ptr<App_State> pop_state();
std::unique_ptr<App_State> swap_state(std::unique_ptr<App_State>&& state);
void run();
void cleanup();

double get_frame_time();

void on_window_resize(int32_t width, int32_t height);
void quit();

} // namespace Engine
} // namespace pegr

#endif // PEGR_ENGINE_ENGINE_HPP
