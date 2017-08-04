#ifndef PEGR_ENGINE_ENGINE_HPP
#define PEGR_ENGINE_ENGINE_HPP

#include <memory>

#include "pegr/engine/App_State.hpp"

namespace pegr {
namespace Engine {

void initialize();
void push_state(std::unique_ptr<App_State>&& state);
std::unique_ptr<App_State> pop_state();
void run();
void cleanup();

void quit();

} // namespace Engine
} // namespace pegr

#endif // PEGR_ENGINE_ENGINE_HPP
