#include "pegr/gensys/Gensys.hpp"

#include <map>
#include <cassert>
#include <sstream>

#include "pegr/logger/Logger.hpp"
#include "pegr/gensys/GensysLuaInterface.hpp"

namespace pegr {
namespace Gensys {

std::map<std::string, Interm::Comp_Def*> m_staged_components;
std::map<std::string, Interm::Arche*> m_staged_archetypes;

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
    
    for (auto const& pair : m_staged_archetypes) {
        delete pair.second;
    }
    m_staged_archetypes.clear();
    
    for (auto const& pair : m_staged_components) {
        delete pair.second;
    }
    m_staged_components.clear();
    
    m_global_state = GlobalState::UNINITIALIZED;
}

void stage_component(const char* id, Interm::Comp_Def* comp_def) {
    std::string id_str(id);
    auto iter = m_staged_components.find(id_str);
    if (iter != m_staged_components.end()) {
        Logger::log()->warn("Overwriting staged component: %v", id);
        delete iter->second;
    }
    m_staged_components[id_str] = comp_def;
}
Interm::Comp_Def* get_staged_component(const char* id) {
    std::string id_str(id);
    auto iter = m_staged_components.find(id_str);
    if (iter == m_staged_components.end()) {
        Logger::log()->warn("Could not find staged component: %v", id);
        return nullptr;
    }
    return iter->second;
}
void stage_archetype(const char* id, Interm::Arche* arche) {
    std::string id_str(id);
    auto iter = m_staged_archetypes.find(id_str);
    if (iter != m_staged_archetypes.end()) {
        Logger::log()->warn("Overwriting staged archetype: %v", id);
        delete iter->second;
    }
    m_staged_archetypes[id_str] = arche;
}
Interm::Arche* get_staged_archetype(const char* id) {
    std::string id_str(id);
    auto iter = m_staged_archetypes.find(id_str);
    if (iter == m_staged_archetypes.end()) {
        Logger::log()->warn("Could not find staged archetype: %v", id);
        return nullptr;
    }
    return iter->second;
}

} // namespace Gensys
} // namespace pegr
