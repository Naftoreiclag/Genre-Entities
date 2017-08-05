#ifndef PEGR_ENGINE_ENGINE_HPP
#define PEGR_ENGINE_ENGINE_HPP

#include <memory>
#include <cstdint>

#include "pegr/engine/App_State.hpp"

namespace pegr {
namespace Engine {

extern const uint16_t INIT_FLAG_LOGGER;
extern const uint16_t INIT_FLAG_SCRIPT;
extern const uint16_t INIT_FLAG_GENSYS;
extern const uint16_t INIT_FLAG_SCHED;
extern const uint16_t INIT_FLAG_WINPUT;
extern const uint16_t INIT_FLAG_ALL;
extern const uint16_t INIT_FLAG_NONE;

bool logger_used();
bool script_used();
bool gensys_used();
bool sched_used();
bool winput_used();

void initialize(uint16_t flags = INIT_FLAG_ALL);
void push_state(std::unique_ptr<App_State>&& state);
std::unique_ptr<App_State> pop_state();
void run();
void cleanup();

void quit();

} // namespace Engine
} // namespace pegr

#endif // PEGR_ENGINE_ENGINE_HPP
