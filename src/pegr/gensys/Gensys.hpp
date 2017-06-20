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
 * @brief Stages an intermediate component definition for compilation
 * @param id
 * @param comp_def the component definition
 */
void stage_component(const char* id, Interm::Comp_Def* comp_def);

/**
 * @brief Returns a currently staged component from the id. If the component
 * cannot be found, nullptr is returned.
 * @param id
 * @return nullptr or staged Comp_Def
 */
Interm::Comp_Def* get_staged_component(const char* id);

/**
 * @brief Stages an intermediate archetype for compilation
 * @param id
 * @param arche the archetype
 */
void stage_archetype(const char* id, Interm::Arche* arche);

/**
 * @brief Returns a currently staged archetype from the id. If the archetype
 * cannot be found, nullptr is returned.
 * @param id
 * @return nullptr or staged Arche
 */
Interm::Arche* get_staged_archetype(const char* id);
 
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_GENSYS_HPP
