#include "pegr/gensys/Gensys.hpp"

#include "pegr/gensys/Compiler.hpp"
#include "pegr/gensys/Util.hpp"
#include "pegr/gensys/Runtime.hpp"

namespace pegr {
namespace Gensys {

GlobalState m_global_state = GlobalState::UNINITIALIZED;

GlobalState get_global_state() {
    return m_global_state;
}

void initialize() {
    assert(m_global_state == GlobalState::UNINITIALIZED);
    Runtime::initialize();
    Compiler::initialize();
    m_global_state = GlobalState::MUTABLE;
}
void compile() {
    assert(m_global_state == GlobalState::MUTABLE);
    Compiler::compile();
    m_global_state = GlobalState::EXECUTABLE;
}
void cleanup() {
    assert(m_global_state != GlobalState::UNINITIALIZED);
    Compiler::cleanup();
    Runtime::cleanup();
    m_global_state = GlobalState::UNINITIALIZED;
}
    
} // namespace Gensys
} // namespace pegr

