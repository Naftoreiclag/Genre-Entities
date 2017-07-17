#include "pegr/gensys/Gensys.hpp"

#include <memory>
#include <vector>

namespace pegr {
namespace Gensys {

std::vector<std::unique_ptr<Runtime::Comp> > m_runtime_comps;
std::vector<std::unique_ptr<Runtime::Arche> > m_runtime_arches;
std::vector<std::unique_ptr<Runtime::Genre> > m_runtime_genres;

} // namespace Gensys
} // namespace pegr
