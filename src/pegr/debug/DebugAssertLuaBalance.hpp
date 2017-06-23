#ifndef PEGR_DEBUG_DEBUGASSERTLUABALANCE_HPP
#define PEGR_DEBUG_DEBUGASSERTLUABALANCE_HPP

namespace pegr {
namespace Debug {

class LuaBalanceGuard {
public:
    LuaBalanceGuard(int delta = 0, const char* msg = nullptr, int line = -1);
    ~LuaBalanceGuard();
private:
    int m_original_size;
    const int m_line;
    const int m_delta;
    const char* m_msg;
};

} // namespace Debug
} // namespace pegr

#endif // PEGR_DEBUG_DEBUGASSERTLUABALANCE_HPP
