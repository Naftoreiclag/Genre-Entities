#ifndef PEGR_GENSYS_GENSYS_HPP
#define PEGR_GENSYS_GENSYS_HPP

#include "pegr/gensys/GensysIntermediate.hpp"

namespace pegr {
namespace Gensys {

enum struct GlobalState {
    // Stage before initialize() is called for the first time
    UNINITIALIZED,
    
    // Stage where new elements can be added into the system
    MUTABLE,
    
    // Stage after "compiling," entities can be created and processed now
    EXECUTABLE,
    
    ENUM_SIZE /*Number of valid enum values*/
};

GlobalState get_global_state();

/**
 * @brief Initializes the gensys. Sets up workspaces for adding new components,
 * genres, archetypes, etc. Initially in editing (working) mode.
 */
void initialize();

/**
 * @brief Transitions to executable mode, turning all of the staged intermediate
 * elements into their post-process types
 */
void compile();

void cleanup();

/**
 * @brief Stages an intermediate component definition for compilation. 
 * This also hands off deletion responsibility to Gensys
 * @param id
 * @param comp_def the component definition
 */
void stage_component(std::string id, Interm::Comp_Def* comp_def);

/**
 * @brief Returns a currently staged component from the id. If the component
 * cannot be found, nullptr is returned.
 * @param id
 * @return nullptr or staged Comp_Def
 */
Interm::Comp_Def* get_staged_component(std::string id);

/**
 * @brief Stages an intermediate archetype for compilation
 * @param id
 * @param arche the archetype
 */
void stage_archetype(std::string id, Interm::Arche* arche);

/**
 * @brief Returns a currently staged archetype from the id. If the archetype
 * cannot be found, nullptr is returned.
 * @param id
 * @return nullptr or staged Arche
 */
Interm::Arche* get_staged_archetype(std::string id);

/**
 * @brief Stages an intermediate genre for compilation
 * @param id
 * @param genre
 */
void stage_genre(std::string id, Interm::Genre* genre);

/**
 * @brief Returns a currently staged genre from the id. If the genre
 * cannot be found, nullptr is returned.
 * @param id
 * @return nullptr or staged Genre
 */
Interm::Genre* get_staged_genre(std::string id);

} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_GENSYS_HPP
