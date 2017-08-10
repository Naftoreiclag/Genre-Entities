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

#ifndef PEGR_GENSYS_COMPILER_HPP
#define PEGR_GENSYS_COMPILER_HPP

#include "pegr/gensys/Interm_Types.hpp"
#include "pegr/gensys/Runtime.hpp"

namespace pegr {
namespace Gensys {
namespace Compiler {

/**
 * @brief Initializes the gensys. Sets up workspaces for adding new components,
 * genres, archetypes, etc. Initially in editing (working) mode.
 */
void initialize();

void cleanup();

/**
 * @brief Transitions to executable mode, turning all of the staged intermediate
 * elements into their post-process types
 */
void compile();

/**
 * @brief Stages an intermediate component definition for compilation. 
 * This also hands off deletion responsibility to Gensys.
 * @param id
 * @param comp the component definition
 */
void stage_component(std::string id, std::unique_ptr<Interm::Comp>&& comp);

/**
 * @brief Returns a currently staged component from the id. If the component
 * cannot be found, nullptr is returned.
 * @param id
 * @return nullptr or staged Comp_Def
 */
Interm::Comp* get_staged_component(std::string id);

/**
 * @brief Unstages an intermediate component definition for compilation.
 * Also deletes it.
 * @param id
 */
void unstage_component(std::string id);

/**
 * @brief Stages an intermediate archetype for compilation
 * @param id
 * @param arche the archetype
 */
void stage_archetype(std::string id, std::unique_ptr<Interm::Arche>&& arche);

/**
 * @brief Returns a currently staged archetype from the id. If the archetype
 * cannot be found, nullptr is returned.
 * @param id
 * @return nullptr or staged Arche
 */
Interm::Arche* get_staged_archetype(std::string id);

/**
 * @brief Unstages an intermediate archetype for compilation.
 * Also deletes it.
 * @param id
 */
void unstage_archetype(std::string id);

/**
 * @brief Stages an intermediate genre for compilation
 * @param id
 * @param genre
 */
void stage_genre(std::string id, std::unique_ptr<Interm::Genre>&& genre);

/**
 * @brief Returns a currently staged genre from the id. If the genre
 * cannot be found, nullptr is returned.
 * @param id
 * @return nullptr or staged Genre
 */
Interm::Genre* get_staged_genre(std::string id);

/**
 * @brief Unstages an intermediate genre for compilation.
 * Also deletes it.
 * @param id
 */
void unstage_genre(std::string id);

enum struct ObjectType {
    NOT_FOUND,
    COMP_DEF,
    ARCHETYPE,
    GENRE,
    
    ENUM_SIZE /*Number of valid enum values*/
};

/**
 * @brief Returns what type of object is pointed to by this id. Returns 
 * NOT_FOUND enum value if the object was not found
 * @param id
 * @return The type of the object pointed to by this id.
 */
ObjectType get_staged_type(std::string id);

} // namespace Compiler
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_COMPILER_HPP
