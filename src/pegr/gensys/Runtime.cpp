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

#include "pegr/gensys/Runtime.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

#include "pegr/debug/Debug_Macros.hpp"
#include "pegr/gensys/Util.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/script/Script_Util.hpp"

namespace pegr {
namespace Gensys {
namespace Runtime {

std::map<std::string, std::unique_ptr<Runtime::Comp> > n_runtime_comps;
std::map<std::string, std::unique_ptr<Runtime::Arche> > n_runtime_arches;
std::map<std::string, std::unique_ptr<Runtime::Genre> > n_runtime_genres;
std::vector<Script::Unique_Regref> n_held_lua_values;

const uint64_t ENT_HEADER_FLAGS = 0;
const uint64_t ENT_HEADER_SIZE = ENT_HEADER_FLAGS + 8;

const uint64_t ENT_FLAG_SPAWNED =           1 << 0;
const uint64_t ENT_FLAG_KILLED =            1 << 1;
const uint64_t ENT_FLAG_LUA_OWNED =         1 << 2;
const uint64_t ENT_FLAGS_DEFAULT =          0;

uint64_t n_next_handle = 0;
std::vector<Entity> n_entities;
std::unordered_map<uint64_t, std::size_t> n_handle_to_index;

Member_Key::Member_Key(Arche::Aggindex aggidx, Prim prim)
: m_aggidx(aggidx)
, m_prim(prim) {}

Entity_Handle::Entity_Handle(uint64_t id)
: m_entity_id(id) {}

Entity_Handle::Entity_Handle()
: m_entity_id(-1) {}

uint64_t Entity_Handle::get_id() const {
    return m_entity_id;
}
bool Entity_Handle::does_exist() const {
    return m_entity_id != -1
            && n_handle_to_index.find(*this) != n_handle_to_index.end();
}
Entity* Entity_Handle::operator ->() const {
    return get_entity();
}

Entity_Handle::operator bool() const {
    return does_exist();
}
Entity_Handle::operator uint64_t() const {
    return get_id();
}
Entity* Entity_Handle::get_volatile_entity_ptr() const {
    return get_entity();
}

Entity* Entity_Handle::get_entity() const {
    if (m_entity_id == -1) return nullptr;
    auto iter = n_handle_to_index.find(*this);
    if (iter == n_handle_to_index.end()) {
        return nullptr;
    }
    std::size_t idx = iter->second;
    //Logger::log()->info("get %v %v", idx, n_entities.size());
    assert(idx >= 0 && idx < n_entities.size());
    return &(n_entities[idx]);
}

Entity::Entity(Arche* arche)
: m_arche(arche)
, m_handle(reserve_new_handle()) {
    
    m_chunk.reset(Pod::new_pod_chunk(
            ENT_HEADER_SIZE + m_arche->m_default_chunk.get().get_size()));
    
    Pod::copy_pod_chunk(
            m_arche->m_default_chunk.get(), 0, 
            m_chunk.get(), ENT_HEADER_SIZE,
            m_arche->m_default_chunk.get().get_size());
    
    m_chunk.get().set_value<uint64_t>(ENT_HEADER_FLAGS, ENT_FLAGS_DEFAULT);
    m_strings = m_arche->m_default_strings;
    
    assert(get_flags() == ENT_FLAGS_DEFAULT);
    assert(!has_been_spawned());
}

Entity::Entity()
: m_arche(nullptr) {}

Entity_Handle Entity::new_entity(Arche* arche) {
    // Record what the entity's index in the vector will be
    std::size_t index = n_entities.size();
    
    // Emplace-back a new entity
    n_entities.emplace_back(arche);
    
    // If there is a failure for some reason
    if (n_entities.size() != index + 1) {
        Logger::log()->warn("Unable to create new entity!");
        
        // Return a null handle
        return Entity_Handle();
    }
    
    // Get the entity we just created by reference
    Entity& ent = n_entities.back();
    
    // Map the entity handle to that index in the vector
    Entity_Handle hand = ent.get_handle();
    n_handle_to_index[hand] = index;
    
    // Return handle to newly created entity
    return hand;
}

void Entity::delete_entity(Entity_Handle handle_a) {
    /* Delete the entity given by the handle (entity "A") by swapping it with
     * the last entity in the vector (entity "B"), then popping off the last
     * entity (A). Must also update B's index (set B's index to A's old index).
     * 
     * The special case where A and B are the same entity is handled implicitly.
     */
    
    assert(n_entities.size() > 0);
    
    // Find the pair in the handle-to-index map
    auto map_entry_a = n_handle_to_index.find(handle_a);
    assert(map_entry_a != n_handle_to_index.end());
    
    // Get the index of the entity in the entity vector
    std::size_t index_a = map_entry_a->second;
    
    // Get the index of the last entity
    std::size_t index_b = n_entities.size() - 1;
    
    // Get the handle of the last entity
    Entity_Handle handle_b = n_entities[index_b].get_handle();
    
    // Find the pair in the handle-to-index map
    auto map_entry_b = n_handle_to_index.find(handle_b);
    assert(map_entry_b != n_handle_to_index.end());
    
    // Set the index
    map_entry_b->second = index_a;
    
    // Swap this entity with the back and remove
    std::iter_swap(n_entities.begin() + index_a, n_entities.begin() + index_b);
    n_entities.pop_back();
    assert(n_entities.size() == index_b);
    assert(n_entities.size() == 0 || 
            n_handle_to_index[n_entities[index_a].get_handle()] == index_a);
    
    // Remove the corresponding entry from the handle-to-index map
    n_handle_to_index.erase(map_entry_a);
    assert(n_handle_to_index.find(handle_a) == n_handle_to_index.end());
    
    //Logger::log()->info("delete swapped #%v with #%v", index_a, index_b);
}

Arche* Entity::get_arche() const {
    return m_arche;
}
Pod::Chunk_Ptr Entity::get_chunk() const {
    return m_chunk.get();
}
Entity_Handle Entity::get_handle() const {
    return m_handle;
}

Script::Regref Entity::get_table() {
    if (m_generic_table.is_nil()) {
        assert_balance(0);
        lua_State* l = Script::get_lua_state();
        lua_newtable(l);
        m_generic_table.reset(Script::grab_reference());
    }
    return m_generic_table.get();
}

Script::Regref Entity::get_weak_table() {
    if (m_generic_weak_table.is_nil()) {
        assert_balance(0);
        Script::Util::push_new_weak_table("v"); // +1
        m_generic_weak_table.reset(Script::grab_reference()); // -1
    }
    return m_generic_weak_table.get();
}

void Entity::free_table() {
    m_generic_table.reset();
}

void Entity::free_weak_table() {
    m_generic_weak_table.reset();
}

std::string Entity::get_string(std::size_t idx) const {
    assert(idx >= 0 && idx < m_strings.size());
    return m_strings[idx];
}
Script::Regref Entity::get_func(std::size_t idx) const {
    assert(idx >= 0 && idx < m_arche->m_static_funcs.size());
    return m_arche->m_static_funcs[idx];
}
uint64_t Entity::get_flags() const {
    return m_chunk.get().get_value<uint64_t>(ENT_HEADER_FLAGS);
}

bool Entity::has_been_spawned() const {
    return (get_flags() & ENT_FLAG_SPAWNED) == ENT_FLAG_SPAWNED;
}
bool Entity::is_alive() const {
    uint64_t flags = get_flags();
    // Spawned and not killed
    
    return (flags & (ENT_FLAG_SPAWNED | ENT_FLAG_KILLED)) == ENT_FLAG_SPAWNED;
}
bool Entity::has_been_killed() const {
    return (get_flags() & ENT_FLAG_KILLED) == ENT_FLAG_KILLED;
}
bool Entity::can_be_spawned() const {
    return !has_been_spawned();
}
bool Entity::is_lua_owned() const {
    return (get_flags() & ENT_FLAG_LUA_OWNED) == ENT_FLAG_LUA_OWNED;
}

void Entity::set_flags(uint64_t arg_flags, bool set) {
    uint64_t flags = m_chunk.get().get_value<uint64_t>(ENT_HEADER_FLAGS);
    if (set) {
        flags |= arg_flags;
        assert((flags & arg_flags) == arg_flags);
    } else {
        flags &= ~arg_flags;
        assert((flags & arg_flags) == 0);
    }
    m_chunk.get().set_value<uint64_t>(ENT_HEADER_FLAGS, flags);
}
    
void Entity::set_flag_spawned(bool flag) {
    set_flags(ENT_FLAG_SPAWNED, flag);
    assert(has_been_spawned() == flag);
}
void Entity::set_flag_lua_owned(bool flag) {
    set_flags(ENT_FLAG_LUA_OWNED, flag);
    assert(is_lua_owned() == flag);
}
void Entity::set_flag_killed(bool flag) {
    set_flags(ENT_FLAG_KILLED, flag);
    assert(has_been_killed() == flag);
}
void Entity::set_string(std::size_t idx, std::string str) {
    assert(idx >= 0 && idx < m_strings.size());
    m_strings[idx] = str;
}
void* Entity::get_member(const Member_Key& member_key) {
    /* Depending on the member's type, where we read the data and how we
     * intepret it changes. For POD types, the data comes from the chunk. Other
     * types are stored in other arrays. Note that the member signature uses
     * a union to store the different offsets, and so there is only one defined
     * way to read the data.
     */
    const Prim& prim = member_key.m_prim;
    const Arche::Aggindex& aggidx = member_key.m_aggidx;
    switch(prim.m_type) {
        case Runtime::Prim::Type::I32:
        case Runtime::Prim::Type::I64:
        case Runtime::Prim::Type::F32:
        case Runtime::Prim::Type::F64: {
            std::size_t pod_offset = Runtime::ENT_HEADER_SIZE
                                    + aggidx.m_pod_idx 
                                    + prim.m_refer.m_byte_offset;
            switch(prim.m_type) {
                case Runtime::Prim::Type::I32: {
                    return m_chunk.get().get_aligned<int32_t>(pod_offset);
                }
                case Runtime::Prim::Type::I64: {
                    return m_chunk.get().get_aligned<int64_t>(pod_offset);
                }
                case Runtime::Prim::Type::F32: {
                    return m_chunk.get().get_aligned<float>(pod_offset);
                }
                case Runtime::Prim::Type::F64: {
                    return m_chunk.get().get_aligned<double>(pod_offset);
                }
                default: {
                    assert(false && "Unhandled pod prim type!");
                }
            }
        }
        case Runtime::Prim::Type::STR: {
            std::size_t string_idx = aggidx.m_string_idx
                                    + prim.m_refer.m_index;
            return &(m_strings[string_idx]);
        }
        case Runtime::Prim::Type::FUNC: {
            std::size_t func_idx = aggidx.m_func_idx
                                    + prim.m_refer.m_index;
            return &(m_arche->m_static_funcs[func_idx]);
        }
        default: {
            assert(false && "Unhandled prim type!");
        }
    }
}

void* Cview::get_member_ptr(Symbol member_symb) const {
    Entity* ent_ptr = m_ent.get_volatile_entity_ptr();
    if (!ent_ptr) {
        return nullptr;
    }
    // Find where the member is stored within the component
    auto prim_entry = m_comp->m_member_offsets.find(member_symb);
    if (prim_entry == m_comp->m_member_offsets.end()) {
        return nullptr;
    }
    Member_Key member_key(m_cached_aggidx, prim_entry->second);
    return ent_ptr->get_member(member_key);
}

bool Cview::operator ==(const Cview& rhs) const {
    return m_comp == rhs.m_comp && m_ent == rhs.m_ent;
}

void* Genview::get_member_ptr(Symbol member_symb) const {
    Entity* ent_ptr = m_ent.get_volatile_entity_ptr();
    if (!ent_ptr) {
        return nullptr;
    }
    // Extract the aggidx and primitive
    auto alias_entry = m_pattern->m_aliases.find(member_symb);
    if (alias_entry == m_pattern->m_aliases.end()) {
        return nullptr;
    }
    const Runtime::Genre::Pattern::Alias& member_alias = alias_entry->second;
    Runtime::Arche* arche = ent_ptr->get_arche();
    assert(arche->m_comp_offsets.find(member_alias.m_comp) !=
            arche->m_comp_offsets.end());
    Member_Key member_key(
            arche->m_comp_offsets.at(member_alias.m_comp),
            member_alias.m_prim_copy);
    return ent_ptr->get_member(member_key);
}

bool Genview::operator ==(const Genview& rhs) const {
    return m_pattern == rhs.m_pattern && m_ent == rhs.m_ent;
}

void initialize() {
}
void cleanup() {
    n_entities.clear();
    // Very important: otherwise entity handles may wrongly report existence.
    n_handle_to_index.clear();
    
    n_next_handle = 0;
    n_runtime_comps.clear();
    n_runtime_arches.clear();
    n_runtime_genres.clear();
    n_held_lua_values.clear();
}

Entity_Handle reserve_new_handle() {
    return Entity_Handle(n_next_handle++);
}

Runtime::Comp* find_component(std::string id_str) {
    return Util::find_something(n_runtime_comps, id_str, 
            "Could not find component: %v");
}
Runtime::Arche* find_archetype(std::string id_str) {
    return Util::find_something(n_runtime_arches, id_str, 
            "Could not find archetype: %v");
}
Runtime::Genre* find_genre(std::string id_str) {
    return Util::find_something(n_runtime_genres, id_str, 
            "Could not find genre: %v");
}

const char* prim_typename(Runtime::Prim::Type t) {
    switch (t) {
        case Prim::Type::F32: return "f32";
        case Prim::Type::F64: return "f64";
        case Prim::Type::I32: return "i32";
        case Prim::Type::I64: return "i64";
        case Prim::Type::STR: return "str";
        case Prim::Type::FUNC: return "func";
        default: assert(0); return "unknown_type";
    }
}

} // namespace Runtime
} // namespace Gensys
} // namespace pegr
