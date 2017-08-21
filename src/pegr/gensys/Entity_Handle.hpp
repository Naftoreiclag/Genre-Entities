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

#ifndef TEMPLATE_USE_HEADER_GENERATOR
#define TEMPLATE_USE_HEADER_GENERATOR

#include <cstdint>

namespace pegr {
namespace Gensys {
namespace Runtime {

class Entity;

/**
 * @class Entity_Handle
 * @brief Since Lua may have holds on Entity instances, rather than pass around
 * pointers, we use 64-bit handles. Note: we used raw pointers to Comp, Arche, 
 * etc. earlier because those objects are guaranteed to exist during the
 * lifetime of a gensys ecosystem. This is not true for Entity, and so we
 * cannot use raw pointers because those can 1) become invalidated and 2)
 * suddenly point to another Entity.
 * 
 * Note that since we will be passing these ids to Lua (and therefore be 
 * converting them into 64-bit floats), the bottom 52 bits are also guaranteed
 * to be unique. (2^53 is the smallest positive value that, when stored in a 
 * 64-bit IEEE 754 double, cannot be incremented by one).
 */
class Entity_Handle {
public:
    Entity_Handle();
    explicit Entity_Handle(uint64_t id);
    
    uint64_t get_id() const;
    
    /**
     * @brief Returns false if any of the following conditions are true:
     *  -   This handle has no entity associated with it at all, such as
     *      immediately after using the default constructor
     *  -   The entity that this handle once referred to was deleted (and
     *      therefore there is absolutely no data that this handle can
     *      return.)
     * This function returns false iff operator-> returns nullptr
     * Note that existence implies that the entity has been despawned, but
     * being despawned does not imply non-existence.
     * 
     * Existence indicates whether or not any data is retrievable from the 
     * entity instance.
     * 
     * @return Whether an entity "exists"
     */
    bool does_exist() const;
    
    /**
     * @brief Rather than expose the get_entity() method, we use an arrow
     * operator. This also allows Entity members to be accessed directly.
     * Users should check does_exist() before using.
     * 
     * Note that it might be beneficial to use get_volatile_entity_ptr(), but
     * doing so can be unsafe. The arrow operator is guaranteed to always return
     * a valid single-use pointer to the entity, provided does_exist() returns
     * true.
     */
    Entity* operator ->() const;
    
    explicit operator bool() const;
    operator uint64_t() const;
    
    /**
     * @brief WARNING! THIS POINTER STAYS VALID ONLY UNDER VERY SPECIFIC 
     * CONDITIONS! Segfaults ahoy!
     * 
     * This is only used when there are many calls to -> that are guaranteed
     * to have the same return value over time. This should happen so long as
     * no other entities are created or destroyed during that time.
     * 
     * On destruction:
     *      It's possible that the soon-to-be-deleted entity swaps places with 
     *      this one, causing the memory address for the entity to change.
     * On creation:
     *      Following vector iterator invalidation rules, if a resize occurs,
     *      then all pointers are invalidated. A resize may occur when adding
     *      an Entity (i.e. during creation)
     * 
     * In general, use this pointer only within the scope of single function,
     * don't store this value in any long-term container, and do not use this
     * value if there is any chance of entity creation or deletion during its
     * usage. For example, don't use this if you are also calling an addon's
     * function, which could create an entity.
     */
    Entity* get_volatile_entity_ptr() const;
    
private:
    uint64_t m_entity_id;
    
    /**
     * @brief Returns the memory address of the enclosed entity. Note that there
     * is no guarantee that the return of this value will be consistent over
     * the lifetime of the handle, and therefore it cannot be cached and must
     * be re-fetched at the beginning of each method call.
     * @return "volatile" memory address or nullptr if the handle points to
     * nothing or the entity has been deleted.
     */
    Entity* get_entity() const;
};

} // namespace Runtime
} // namespace Gensys
} // namespace pegr

#endif // TEMPLATE_USE_HEADER_GENERATOR
