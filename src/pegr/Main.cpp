#include "pegr/engine/Engine.hpp"
#include "pegr/app/Game.hpp"

using namespace pegr;

int main() {
    Engine::initialize(Engine::INIT_FLAG_ALL);
    Engine::push_state(std::make_unique<App::Game_State>());
    Engine::run();
    Engine::cleanup();
    return 0;
}
