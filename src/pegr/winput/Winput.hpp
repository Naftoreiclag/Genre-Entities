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

#ifndef PEGR_WINPUT_WINPUT_HPP
#define PEGR_WINPUT_WINPUT_HPP

#include <cstdint>

namespace pegr {
namespace Winput {

extern const char* const WINDOW_DEFAULT_TITLE;
extern const int32_t WINDOW_DEFAULT_WIDTH;
extern const int32_t WINDOW_DEFAULT_HEIGHT;

void initialize();
void pollEvents();
void cleanup();

int32_t get_window_width();
int32_t get_window_height();

} // namespace Winput
} // namespace pegr

#endif // PEGR_WINPUT_WINPUT_HPP
