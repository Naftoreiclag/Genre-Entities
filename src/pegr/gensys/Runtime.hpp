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

} // namespace Runtime
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_RUNTIME_HPP
