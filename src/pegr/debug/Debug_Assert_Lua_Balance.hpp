#ifndef PEGR_DEBUG_DEBUGASSERTLUABALANCE_HPP
#define PEGR_DEBUG_DEBUGASSERTLUABALANCE_HPP

#include <vector>

namespace pegr {
namespace Debug {

class LuaBalanceGuard {
public:
    LuaBalanceGuard(std::vector<int> deltas = {0}, 
            const char* msg = nullptr, int line = -1);
    ~LuaBalanceGuard();
private:
    int m_original_size;
    const int m_line;
    std::vector<int> m_deltas;
    const char* m_msg;
};

} // namespace Debug
} // namespace pegr

#endif // PEGR_DEBUG_DEBUGASSERTLUABALANCE_HPP
