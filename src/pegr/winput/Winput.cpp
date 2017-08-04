#include "pegr/winput/Winput.hpp"

#include <stdexcept>

#include <SDL.h>

#include "pegr/engine/Engine.hpp"

namespace pegr {
namespace Winput {

SDL_Window* n_window;

void initialize() {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("Could not initalize SDL video");
    }
    // Create the window
    n_window = SDL_CreateWindow("hello world", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        640, 480, 
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN);
    if(!n_window) {
        throw std::runtime_error("Could not create SDL window");
    }
    SDL_ShowWindow(n_window);
}
void pollEvents() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT: {
                Engine::quit();
                break;
            }
            default: break;
        }
    }
}
void cleanup() {
    SDL_DestroyWindow(n_window);
    SDL_Quit();
}

} // namespace Winput
} // namespace pegr
