#ifndef PEGR_DEBUG_DEBUGMACROS_HPP
#define PEGR_DEBUG_DEBUGMACROS_HPP

#ifndef NDEBUG

#include "pegr/Debug/DebugAssertLuaBalance.hpp"

#define assert_balance(delta, msg) \
        pegr::Debug::LuaBalanceGuard _pegr_debug_balance_assert_obj(delta, msg)

#else

#define assert_balance(delta) ((void*) 0)
    
#endif


#endif // PEGR_DEBUG_DEBUGMACROS_HPP
