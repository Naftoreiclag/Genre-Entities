#include "pegr/engine/Engine.hpp"

using namespace pegr;

int main() {
    Engine::initialize();
    Engine::run();
    Engine::cleanup();
    return 0;
}
