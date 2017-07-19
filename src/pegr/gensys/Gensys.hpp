#ifndef PEGR_GENSYS_GENSYS_HPP
#define PEGR_GENSYS_GENSYS_HPP

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

void initialize();
void compile();
void cleanup();
    
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_GENSYS_HPP
