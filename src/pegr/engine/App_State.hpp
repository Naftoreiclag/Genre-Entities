#ifndef PEGR_ENGINE_APPSTATE_HPP
#define PEGR_ENGINE_APPSTATE_HPP

namespace pegr {
namespace Engine {

class App_State {
public:
    App_State(const char* dbg_name);
    virtual ~App_State();
    
    virtual void initialize();
    virtual void pause(App_State* to);
    virtual void unpause(App_State* from);
    virtual void on_frame();
    virtual void cleanup();
    
    const char* get_dbg_name();
    
private:
    const char* m_dbg_name;
};

} // namespace pegr
} // namespace Engine

#endif // PEGR_ENGINE_APPSTATE_HPP
