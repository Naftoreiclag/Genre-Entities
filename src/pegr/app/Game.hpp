#ifndef PEGR_APP_GAME_HPP
#define PEGR_APP_GAME_HPP

#include "pegr/engine/App_State.hpp"

namespace pegr {
namespace App {

class Game_State : public Engine::App_State {
public:
    Game_State();
    virtual ~Game_State();
    virtual void initialize() override;
    virtual void on_frame() override;
};

} // namespace App
} // namespace pegr

#endif // PEGR_APP_GAME_HPP
