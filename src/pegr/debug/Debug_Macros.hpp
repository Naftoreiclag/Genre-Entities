#ifndef PEGR_DEBUG_DEBUGMACROS_HPP
#define PEGR_DEBUG_DEBUGMACROS_HPP

#ifndef NDEBUG

#include "pegr/Debug/Debug_Assert_Lua_Balance.hpp"

#define assert_balance_msg(msg, ...) \
        pegr::Debug::LuaBalanceGuard \
        _pegr_debug_balance_assert_obj__LINE__ \
        ({__VA_ARGS__}, msg)
#define assert_balance(...) \
        pegr::Debug::LuaBalanceGuard \
        _pegr_debug_balance_assert_obj__LINE__ \
        ({__VA_ARGS__}, __FILE__, __LINE__)

#else

#define assert_balance_msg(...) ((void*) 0)
#define assert_balance(...) ((void*) 0)
    
#endif


#endif // PEGR_DEBUG_DEBUGMACROS_HPP
