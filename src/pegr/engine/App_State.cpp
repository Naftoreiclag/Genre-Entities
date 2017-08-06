#include "pegr/engine/App_State.hpp"

namespace pegr {
namespace Engine {

App_State::App_State(const char* dbg_name)
: m_dbg_name(dbg_name) {}
App_State::~App_State() {}

void App_State::initialize() {}

void App_State::pause(App_State* to) {}
void App_State::unpause(App_State* from) {}

void App_State::on_frame() {}
void App_State::on_window_resize(int32_t width, int32_t height) {}

void App_State::cleanup() {}

const char* App_State::get_dbg_name() {
    return m_dbg_name;
}
    
} // namespace pegr
} // namespace Engine

