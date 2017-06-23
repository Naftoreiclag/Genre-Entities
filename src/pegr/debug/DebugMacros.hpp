#ifndef PEGR_DEBUG_DEBUGMACROS_HPP
#define PEGR_DEBUG_DEBUGMACROS_HPP

#ifndef NDEBUG

#include "pegr/Debug/DebugAssertLuaBalance.hpp"

#define assert_balance_msg(delta, msg) \
        pegr::Debug::LuaBalanceGuard _pegr_debug_balance_assert_obj(delta, msg)
#define assert_balance(delta) \
        pegr::Debug::LuaBalanceGuard _pegr_debug_balance_assert_obj(delta, __FILE__, __LINE__)

#else

#define assert_balance(delta) ((void*) 0)
    
#endif


#endif // PEGR_DEBUG_DEBUGMACROS_HPP
