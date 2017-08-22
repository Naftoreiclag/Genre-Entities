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

#ifndef PEGR_SCRIPT_LUAINTERFUTIL_HPP
#define PEGR_SCRIPT_LUAINTERFUTIL_HPP

#include <functional>

#include "pegr/script/Script.hpp"
#include "pegr/resource/Oid.hpp"

namespace pegr {
namespace Script {
namespace Util {
/**
 * @brief Used in Lua interface functions that stage user-provided constructor
 * tables to some global processing registry (another Lua table) that uses
 * resource IDs as keys.
 * 
 * Arg 1 is the id
 * Arg 2 is the table to add
 * 
 * @param l the Lua state
 * @param registry Registry reference to the table to store the key
 * @param post_copy (Optional, see overload below) performs some kind of extra
 * processing to the table before adding it to the stack
 */
void generic_li_add_to_res_table(lua_State* l, Regref registry, 
        std::function<void(lua_State*)> post_copy);
/**
 * @brief Overload for the function that does not accept a post_copy.
 * 
 * @param l the Lua state
 * @param registry Registry reference to the table to store the key
 */
void generic_li_add_to_res_table(lua_State* l, Regref registry);

/**
 * @brief Retrieves a table previously enqueued into a global processing table,
 * using a resource ID as a key.
 * 
 * Arg 1 is the key
 * 
 * @param l the Lua state
 * @param table
 */
void generic_li_edit_from_res_table(lua_State* l, Regref table);

} // namespace Util
} // namespace Script
} // namespace pegr

#endif // PEGR_SCRIPT_LUAINTERFUTIL_HPP
