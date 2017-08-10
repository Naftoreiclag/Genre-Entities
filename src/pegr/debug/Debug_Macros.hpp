/*
 *  Copyright 2017 James Fong
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

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
