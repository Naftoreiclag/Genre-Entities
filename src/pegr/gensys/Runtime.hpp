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

#ifndef PEGR_GENSYS_RUNTIME_HPP
#define PEGR_GENSYS_RUNTIME_HPP

#include "pegr/gensys/Runtime_Types.hpp"

namespace pegr {
namespace Gensys {
namespace Runtime {

void initialize();
void cleanup();

Entity_Handle reserve_new_handle();

Runtime::Comp* find_component(std::string id);
Runtime::Arche* find_archetype(std::string id);
Runtime::Genre* find_genre(std::string id);

const char* prim_typename(Runtime::Prim::Type t);

} // namespace Runtime
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_RUNTIME_HPP
