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

#include <functional>

#include "pegr/gensys/Entity_Collection.hpp"
#include "pegr/gensys/Runtime_Types.hpp"
#include "pegr/resource/Oid.hpp"

namespace pegr {
namespace Gensys {
namespace Runtime {

void initialize();
void cleanup();

Entity_Collection& get_entities();

/**
 * @param num A 64 bit unsigned value. When passing the entity's unique id to
 * Lua, we can only rely on the integrity of the least significant 52 bits, 
 * as Lua only uses doubles. However, as the bottom 52 bits of the entity's
 * unique id are guaranteed unique, the id's uniqueness is preserved.
 * @return The lower 52 bits
 */
uint64_t bottom_52(uint64_t num);

Comp* find_comp(Resour::Oid oid);
Arche* find_arche(Resour::Oid oid);
Genre* find_genre(Resour::Oid oid);

const char* prim_to_dbg_string(Prim::Type ty);

/* The to_string_X convert various objects into human-readable strings. Used
 * mainly for tostring(...) in Lua
 */
std::string to_string_comp(Runtime::Comp* comp);
std::string to_string_arche(Runtime::Arche* arche);
std::string to_string_genre(Runtime::Genre* genre);
std::string to_string_entity(Runtime::Entity_Handle ent);
std::string to_string_cview(Runtime::Cview cview);
std::string to_string_genview(Runtime::Genview genview);

} // namespace Runtime
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_RUNTIME_HPP
