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

#include "pegr/algs/Algs.hpp"
#include "pegr/debug/Debug_Macros.hpp"
#include "pegr/except/Except.hpp"
#include "pegr/gensys/Entity_Collection.hpp"
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

const std::uint64_t ENT_HEADER_FLAGS = 0;
const std::uint64_t ENT_HEADER_SIZE = ENT_HEADER_FLAGS + 8;

const std::uint64_t ENT_FLAG_SPAWNED =           1 << 0;
const std::uint64_t ENT_FLAG_KILLED =            1 << 1;
const std::uint64_t ENT_FLAG_LUA_OWNED =         1 << 2;
const std::uint64_t ENT_FLAGS_DEFAULT =          0;

Entity_Collection n_ent_collection;

Entity_Collection& get_entities() {
    return n_ent_collection;
}

const char* prim_to_dbg_string(Prim::Type ty) {
    switch (ty) {
        case Prim::Type::I32: return "i32";
        case Prim::Type::I64: return "i64";
        case Prim::Type::F32: return "f32";
        case Prim::Type::F64: return "f64";
        case Prim::Type::FUNC: return "func";
        case Prim::Type::STR: return "str";
        case Prim::Type::NULLPTR: return "nullptr";
        default: return "unknown";
    }
}

std::string to_string_comp(Runtime::Comp* comp) {
    std::stringstream sss;
    sss << "<Component @"
        << comp
        << ">";
    return sss.str();
}
std::string to_string_arche(Runtime::Arche* arche) {
    std::stringstream sss;
    sss << "<Archetype @"
        << arche
        << ">";
    return sss.str();
}
std::string to_string_genre(Runtime::Genre* genre) {
    std::stringstream sss;
    sss << "<Genre @"
        << genre
        << ">";
    return sss.str();
}
std::string to_string_entity(Runtime::Entity_Handle ent) {
    std::stringstream sss;
    sss << "<Entity #"
        << bottom_52(ent.get_id());
    if (ent.does_exist()) {
        sss << " thru Arche @"
            << ent->get_arche();
    } else {
        sss << " (deleted)";
    }
    sss << ">";
    return sss.str();
}
std::string to_string_cview(Runtime::Cview cview) {
    std::stringstream sss;
    sss << "<Entity #"
        << bottom_52(cview.m_ent.get_id());
    if (cview.m_ent.does_exist()) {
        sss << " thru Comp @"
            << cview.m_comp;
    } else {
        sss << " (deleted)";
    }
    sss << ">";
    return sss.str();
}
std::string to_string_genview(Runtime::Genview genview) {
    std::stringstream sss;
    sss << "<Entity #"
        << bottom_52(genview.m_ent.get_id());
    if (genview.m_ent.does_exist()) {
        sss << " thru G-Pattern @"
            << genview.m_pattern;
    } else {
        sss << " (deleted)";
    }
    sss << ">";
    return sss.str();
}

void throw_mismatch_error(const char* expected, const char* got) {
    std::stringstream sss;
    sss << "Expected "
        << expected
        << ", got "
        << got;
    throw Except::Runtime(sss.str());
}

void verify_equal_type(Prim::Type expected, Prim::Type got) {
    if (got != expected) {
        throw_mismatch_error(
                prim_to_dbg_string(expected),
                prim_to_dbg_string(got));
    }
}

Member_Ptr::Member_Ptr(Prim::Type typ, void* ptr)
: m_type(typ)
, m_ptr(ptr) {}

Member_Ptr::Member_Ptr()
: m_type(Prim::Type::NULLPTR)
, m_ptr(nullptr) {}

Prim::Type Member_Ptr::get_type() const {
    return m_type;
}

void Member_Ptr::set_value_i32(std::int32_t val) const {
    //Logger::log()->info("Set i32 %v", val);
    verify_equal_type(Prim::Type::I32, m_type);
    *(static_cast<std::int32_t*>(m_ptr)) = val;
}
void Member_Ptr::set_value_i64(std::int64_t val) const {
    //Logger::log()->info("Set i64 %v", val);
    verify_equal_type(Prim::Type::I64, m_type);
    *(static_cast<std::int64_t*>(m_ptr)) = val;
}
void Member_Ptr::set_value_f32(float val) const {
    //Logger::log()->info("Set f32 %v", val);
    verify_equal_type(Prim::Type::F32, m_type);
    *(static_cast<float*>(m_ptr)) = val;
}
void Member_Ptr::set_value_f64(double val) const {
    //Logger::log()->info("Set f64 %v", val);
    verify_equal_type(Prim::Type::F64, m_type);
    *(static_cast<double*>(m_ptr)) = val;
}
void Member_Ptr::set_value_str(const std::string& val) const {
    //Logger::log()->info("Set str %v", val);
    verify_equal_type(Prim::Type::STR, m_type);
    *(static_cast<std::string*>(m_ptr)) = val;
}
void Member_Ptr::set_value_func(Script::Regref val) const {
    //Logger::log()->info("Set func %v", val);
    verify_equal_type(Prim::Type::FUNC, m_type);
    throw Except::Runtime("Cannot assign to static value");
}

std::int32_t Member_Ptr::get_value_i32() const {
    //Logger::log()->info("Get i32");
    verify_equal_type(Prim::Type::I32, m_type);
    return *(static_cast<std::int32_t*>(m_ptr));
}
std::int64_t Member_Ptr::get_value_i64() const {
    //Logger::log()->info("Get i64");
    verify_equal_type(Prim::Type::I64, m_type);
    return *(static_cast<std::int64_t*>(m_ptr));
}
float Member_Ptr::get_value_f32() const {
    //Logger::log()->info("Get f32");
    verify_equal_type(Prim::Type::F32, m_type);
    return *(static_cast<float*>(m_ptr));
}
double Member_Ptr::get_value_f64() const {
    //Logger::log()->info("Get f64");
    verify_equal_type(Prim::Type::F64, m_type);
    return *(static_cast<double*>(m_ptr));
}
const std::string& Member_Ptr::get_value_str() const {
    //Logger::log()->info("Get str");
    verify_equal_type(Prim::Type::STR, m_type);
    return *(static_cast<std::string*>(m_ptr));
}
Script::Regref Member_Ptr::get_value_func() const {
    //Logger::log()->info("Get func");
    verify_equal_type(Prim::Type::FUNC, m_type);
    return *(static_cast<Script::Regref*>(m_ptr));
}

void Member_Ptr::set_value_any_number(double val) const {
    //Logger::log()->info("Set any num %v", val);
    switch (m_type) {
        case Prim::Type::I32: {
            *(static_cast<std::int32_t*>(m_ptr)) = val;
            break;
        }
        case Prim::Type::I64: {
            *(static_cast<std::int64_t*>(m_ptr)) = val;
            break;
        }
        case Prim::Type::F32: {
            *(static_cast<float*>(m_ptr)) = val;
            break;
        }
        case Prim::Type::F64: {
            *(static_cast<double*>(m_ptr)) = val;
            break;
        }
        default: {
            throw_mismatch_error("(any number)", prim_to_dbg_string(m_type));
        }
    }
}
double Member_Ptr::get_value_any_number() const {
    //Logger::log()->info("Get any num");
    switch (m_type) {
        case Prim::Type::I32: {
            return *(static_cast<std::int32_t*>(m_ptr));
        }
        case Prim::Type::I64: {
            return *(static_cast<std::int64_t*>(m_ptr));
        }
        case Prim::Type::F32: {
            return *(static_cast<float*>(m_ptr));
        }
        case Prim::Type::F64: {
            return *(static_cast<double*>(m_ptr));
        }
        default: {
            throw_mismatch_error("(any number)", prim_to_dbg_string(m_type));
        }
    }
}

bool Member_Ptr::is_nullptr() const {
    assert((m_type == Prim::Type::NULLPTR) == (m_ptr == nullptr));
    return m_type == Prim::Type::NULLPTR;
}

bool Arche::matches(Entity* ent_unsafe) {
    return ent_unsafe->get_arche() == this;
}

Arche* Arche::match(Entity* ent_unsafe) {
    if (ent_unsafe->get_arche() == this) {
        return this;
    }
    return nullptr;
}

bool Cview::is_nullptr() const {
    return !m_ent.does_exist();
}

Member_Ptr Cview::get_member_ptr(const Symbol& member_symb) const {
    //Logger::log()->info("Access %v thru cview", member_symb);
    Entity* ent_ptr = m_ent.get_volatile_entity_ptr();
    if (!ent_ptr) {
        return Member_Ptr();
    }
    // Find where the member is stored within the component
    auto prim_entry = m_comp->m_member_offsets.find(member_symb);
    if (prim_entry == m_comp->m_member_offsets.end()) {
        return Member_Ptr();
    }
    Member_Key member_key(m_cached_aggidx, prim_entry->second);
    //Logger::log()->info("Aggidx %v %v %v", 
    //      member_key.m_aggidx.m_func_idx, 
    //      member_key.m_aggidx.m_pod_idx, 
    //      member_key.m_aggidx.m_string_idx);
    return ent_ptr->get_member(member_key);
}

bool Cview::operator ==(const Cview& rhs) const {
    return m_comp == rhs.m_comp && m_ent == rhs.m_ent;
}
Cview::operator bool() const {
    return !is_nullptr();
}

Cview Comp::match(Entity* ent_unsafe) {
    Cview retval;
    // Try find the aggregate index
    auto aggidx_iter = ent_unsafe->get_arche()
            ->m_comp_offsets.find(this);
    if (aggidx_iter == ent_unsafe->get_arche()->m_comp_offsets.end()) {
        // If its not there, then the matching failed
        assert(retval.is_nullptr());
        return retval;
    }
    // Assemble the cview
    retval.m_cached_aggidx = aggidx_iter->second;
    retval.m_ent = ent_unsafe->get_handle();
    retval.m_comp = this;
    assert(!retval.is_nullptr());
    return retval;
}

Member_Key::Member_Key(Arche::Aggindex aggidx, Prim prim)
: m_aggidx(aggidx)
, m_prim(prim) {}

bool Genview::is_nullptr() const {
    return !m_ent.does_exist();
}
Genview::operator bool() const {
    return !is_nullptr();
}

Member_Ptr Genview::get_member_ptr(const Symbol& member_symb) const {
    Entity* ent_ptr = m_ent.get_volatile_entity_ptr();
    if (!ent_ptr) {
        return Member_Ptr();
    }
    // Extract the aggidx and primitive
    auto alias_entry = m_pattern->m_aliases.find(member_symb);
    if (alias_entry == m_pattern->m_aliases.end()) {
        return Member_Ptr();
    }
    const Runtime::Pattern::Alias& member_alias = alias_entry->second;
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

Genview Genre::match(Entity* ent_unsafe) {
    Genview retval;
    Runtime::Arche* arche = ent_unsafe->get_arche();
    // Maybe cache whether or not this archetype matches?
    if (!Algs::is_subset_of_presorted(
            m_sorted_required_intersection, 
            arche->m_sorted_component_array)) {
        // Cannot possibly match
        assert(retval.is_nullptr());
        return retval;
    }
    // Important: iterate using references not copies
    for (Runtime::Pattern& pattern : m_patterns) {
        if (Algs::is_subset_of_presorted(
                pattern.m_sorted_required_comps_specific, 
                arche->m_sorted_component_array)) {
            retval.m_ent = ent_unsafe->get_handle();
            retval.m_pattern = &pattern;
            assert(!retval.is_nullptr());
            return retval;
        }
    }
    // No matches found
    assert(retval.is_nullptr());
    return retval;
}

Entity::Entity(Arche* arche, Entity_Handle handle)
: m_arche(arche)
, m_handle(handle) {
    
    m_chunk.reset(Pod::new_pod_chunk(
            ENT_HEADER_SIZE + m_arche->m_default_chunk.get().get_size()));
    
    Pod::copy_pod_chunk(
            m_arche->m_default_chunk.get(), 0, 
            m_chunk.get(), ENT_HEADER_SIZE,
            m_arche->m_default_chunk.get().get_size());
    
    m_chunk.get().set_value<std::uint64_t>(ENT_HEADER_FLAGS, ENT_FLAGS_DEFAULT);
    m_strings = m_arche->m_default_strings;
    
    assert(get_flags() == ENT_FLAGS_DEFAULT);
    assert(!has_been_spawned());
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
std::uint64_t Entity::get_flags() const {
    return m_chunk.get().get_value<std::uint64_t>(ENT_HEADER_FLAGS);
}

bool Entity::has_been_spawned() const {
    return (get_flags() & ENT_FLAG_SPAWNED) == ENT_FLAG_SPAWNED;
}
bool Entity::is_alive() const {
    std::uint64_t flags = get_flags();
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

void Entity::spawn() {
    assert(can_be_spawned());
    set_flag_spawned(true);
    assert(has_been_spawned());
}

void Entity::kill() {
    assert(has_been_spawned());
    set_flag_killed(true);
    assert(has_been_killed());
}

Member_Ptr Entity::get_member(const Member_Key& member_key) {
    /* Depending on the member's type, where we read the data and how we
     * intepret it changes. For POD types, the data comes from the chunk. Other
     * types are stored in other arrays. Note that the member signature uses
     * a union to store the different offsets, and so there is only one defined
     * way to read the data.
     */
    const Prim& prim = member_key.m_prim;
    const Arche::Aggindex& aggidx = member_key.m_aggidx;
    void* vptr;
    switch(prim.m_type) {
        case Runtime::Prim::Type::I32:
        case Runtime::Prim::Type::I64:
        case Runtime::Prim::Type::F32:
        case Runtime::Prim::Type::F64: {
            std::size_t pod_offset = Runtime::ENT_HEADER_SIZE
                                    + aggidx.m_pod_idx 
                                    + prim.m_refer.m_byte_offset;
            //Logger::log()->info("Access pod");
            assert(pod_offset < m_chunk.get().get_size());
            assert(pod_offset >= 0);
            switch(prim.m_type) {
                case Runtime::Prim::Type::I32: {
                    //Logger::log()->info("i32");
                    vptr = m_chunk.get().get_aligned<std::int32_t>(pod_offset);
                    break;
                }
                case Runtime::Prim::Type::I64: {
                    //Logger::log()->info("i64");
                    vptr = m_chunk.get().get_aligned<std::int64_t>(pod_offset);
                    break;
                }
                case Runtime::Prim::Type::F32: {
                    //Logger::log()->info("f32");
                    vptr = m_chunk.get().get_aligned<float>(pod_offset);
                    break;
                }
                case Runtime::Prim::Type::F64: {
                    //Logger::log()->info("f64");
                    vptr = m_chunk.get().get_aligned<double>(pod_offset);
                    break;
                }
                default: {
                    assert(false && "Should not have got here!");
                }
            }
            break;
        }
        case Runtime::Prim::Type::STR: {
            std::size_t string_idx = aggidx.m_string_idx
                                    + prim.m_refer.m_index;
            //Logger::log()->info("Access str %v", string_idx);
            assert(string_idx < m_strings.size());
            assert(string_idx >= 0);
            vptr = &(m_strings[string_idx]);
            break;
        }
        case Runtime::Prim::Type::FUNC: {
            std::size_t func_idx = aggidx.m_func_idx
                                    + prim.m_refer.m_index;
            //Logger::log()->info("Access func %v", func_idx);
            assert(func_idx < m_arche->m_static_funcs.size());
            assert(func_idx >= 0);
            vptr = &(m_arche->m_static_funcs[func_idx]);
            break;
        }
        default: {
            assert(false && "Unhandled prim type!");
        }
    }
    return Member_Ptr(prim.m_type, vptr);
}

Cview Entity::make_cview(const Symbol& comp_symb) {
    Cview retval;
    // Try find the component in the archetype
    auto comp_iter = m_arche->m_components.find(comp_symb);
    if (comp_iter == m_arche->m_components.end()) {
        // Not found, return error
        assert(retval.is_nullptr());
        return retval;
    }
    // Get the component ptr
    retval.m_comp = comp_iter->second;
    // Get the aggregate index (which should definitely be there)
    auto aggidx_iter = m_arche->m_comp_offsets.find(retval.m_comp);
    assert(aggidx_iter != m_arche->m_comp_offsets.end());
    retval.m_cached_aggidx = aggidx_iter->second;
    
    retval.m_ent = get_handle();
    assert(!retval.is_nullptr());
    return retval;
}

Entity::Entity()
: m_arche(nullptr) {}

void Entity::set_flags(std::uint64_t arg_flags, bool set) {
    std::uint64_t flags = 
            m_chunk.get().get_value<std::uint64_t>(ENT_HEADER_FLAGS);
    if (set) {
        flags |= arg_flags;
        assert((flags & arg_flags) == arg_flags);
    } else {
        flags &= ~arg_flags;
        assert((flags & arg_flags) == 0);
    }
    m_chunk.get().set_value<std::uint64_t>(ENT_HEADER_FLAGS, flags);
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

void initialize() {
}
void cleanup() {
    n_ent_collection.clear();
    n_runtime_comps.clear();
    n_runtime_arches.clear();
    n_runtime_genres.clear();
    n_held_lua_values.clear();
}

std::uint64_t bottom_52(std::uint64_t num) {
    //             0123456789abcdef
    return num & 0x001FFFFFFFFFFFFF;
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

} // namespace Runtime
} // namespace Gensys
} // namespace pegr
