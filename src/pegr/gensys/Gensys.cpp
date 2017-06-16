#include "pegr/gensys/Gensys.hpp"

#include <cassert>

#include "pegr/gensys/GensysLuaInterface.hpp"

namespace pegr {
namespace Gensys {

GlobalState m_global_state = GlobalState::UNINITIALIZED;

GlobalState get_global_state() {
    return m_global_state;
}

void initialize() {
    assert(m_global_state == GlobalState::UNINITIALIZED);
    LI::initialize();
    m_global_state = GlobalState::MUTABLE;
}

void compile() {
    assert(m_global_state == GlobalState::MUTABLE);
    LI::compile();
    m_global_state = GlobalState::EXECUTABLE;
}

} // namespace Gensys
} // namespace pegr
