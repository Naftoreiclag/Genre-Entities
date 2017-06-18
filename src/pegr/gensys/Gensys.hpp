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

/**
 * @brief Stages an intermediate component definition for compilation
 * @param comp_def the component definition
 */
void stage_component(Interm::Comp_Def comp_def);

} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_GENSYS_HPP
