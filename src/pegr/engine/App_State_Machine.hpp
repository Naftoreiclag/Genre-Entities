#ifndef PEGR_ENGINE_APPSTATEMACHINE_HPP
#define PEGR_ENGINE_APPSTATEMACHINE_HPP

#include <memory>
#include <vector>

#include "pegr/engine/App_State.hpp"

namespace pegr {
namespace Engine {

class App_State_Machine {
public:
    ~App_State_Machine();

    void push_state(std::unique_ptr<App_State>&& state);
    
    std::unique_ptr<App_State> pop_state();
    std::unique_ptr<App_State> swap_state(std::unique_ptr<App_State>&& state);
    
    App_State* get_active();
    
    void clear_all();

private:
    std::vector<std::unique_ptr<App_State> > m_state_pushdown;
};

} // namespace Engine
} // namespace pegr

#endif // PEGR_ENGINE_APPSTATEMACHINE_HPP
