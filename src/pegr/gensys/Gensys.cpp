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
    LI::translate_working();
    m_global_state = GlobalState::EXECUTABLE;
}

void cleanup() {
    assert(m_global_state != GlobalState::UNINITIALIZED);
    LI::cleanup();
    m_global_state = GlobalState::UNINITIALIZED;
}

void stage_component(const char* id, Interm::Comp_Def* comp_def) {
    
}
Interm::Comp_Def* get_staged_component(const char* id) {
    
}
void stage_archetype(const char* id, Interm::Arche* arche) {
    
}
Interm::Arche* get_staged_archetype(const char* id) {
    
}

} // namespace Gensys
} // namespace pegr
