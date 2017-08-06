#ifndef PEGR_APP_GAME_HPP
#define PEGR_APP_GAME_HPP

#include <bgfx/bgfx.h>

#include "pegr/engine/App_State.hpp"
#include "pegr/render/Handle_Util.hpp"

namespace pegr {
namespace App {

class Game_State : public Engine::App_State {
public:
    Game_State();
    virtual ~Game_State();
    virtual void initialize() override;
    
    virtual void on_frame() override;
    virtual void on_window_resize(int32_t width, int32_t height) override;
    
private:
    Render::Unique_Program m_shader_prog;
};

} // namespace App
} // namespace pegr

#endif // PEGR_APP_GAME_HPP
