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

#include "pegr/gensys/Lua_Interf.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "pegr/debug/Debug_Macros.hpp"
#include "pegr/except/Except.hpp"
#include "pegr/gensys/Compiler.hpp"
#include "pegr/gensys/Gensys.hpp"
#include "pegr/gensys/Runtime.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/script/Script_Util.hpp"
#include "pegr/util/Algs.hpp"

namespace pegr {
namespace Gensys {
namespace LI {

Script::Unique_Regref n_comp_metatable;
Script::Unique_Regref n_arche_metatable;
Script::Unique_Regref n_genre_metatable;
Script::Unique_Regref n_entity_metatable;
Script::Unique_Regref n_cview_metatable;
Script::Unique_Regref n_genview_metatable;

lua_Number entity_handle_to_lua_number(uint64_t data) {
    // Shave off the bottom 52 bits and cast to number
    return static_cast<lua_Number>(Runtime::bottom_52(data));
}

/**
 * @brief Attempts to convert the Lua value stored at idx on the stack into a
 * void pointer (userdata), succeeding iff that userdata has the metatable
 * given by the provided registry index. Returns nullptr on failure.
 * @param l The Lua state
 * @param idx The value on the stack to cast into a userdata ptr
 * @param metatable The metatable the userdata ptr must have
 */
void* to_mt_userdata(lua_State* l, int idx, Script::Regref metatable) {
    assert_balance(0);
    void* ptr = lua_touserdata(l, idx);
    if (ptr) {
        if (lua_getmetatable(l, idx)) {
            Script::push_reference(metatable);
            if (lua_rawequal(l, -1, -2)) {
                lua_pop(l, 2);
                return ptr;
            }
            lua_pop(l, 2);
        }
    }
    return nullptr;
}

void* arg_require_userdata(lua_State* l, int narg, Script::Regref metatable,
            const char* dbg_tname) {
    assert(narg > 0);
    void* retval = to_mt_userdata(l, narg, metatable);
    if (!retval) {
        luaL_typerror(l, narg, dbg_tname);
    }
    return retval;
}

Runtime::Comp** arg_require_comp(lua_State* l, int narg) {
    void* lua_mem = arg_require_userdata(l, narg, 
            n_comp_metatable.get(), "pegr.Component");
    return static_cast<Runtime::Comp**>(lua_mem);
}
Runtime::Arche** arg_require_arche(lua_State* l, int narg) {
    void* lua_mem = arg_require_userdata(l, narg, 
            n_arche_metatable.get(), "pegr.Archetype");
    return static_cast<Runtime::Arche**>(lua_mem);
}
Runtime::Entity_Handle* arg_require_entity(lua_State* l, int narg) {
    void* lua_mem = arg_require_userdata(l, narg, 
            n_entity_metatable.get(), "pegr.Entity");
    return static_cast<Runtime::Entity_Handle*>(lua_mem);
}
Runtime::Cview* arg_require_cview(lua_State* l, int narg) {
    void* lua_mem = arg_require_userdata(l, narg, 
            n_cview_metatable.get(), "pegr.Comp_View");
    return static_cast<Runtime::Cview*>(lua_mem);
}
Runtime::Genview* arg_require_genview(lua_State* l, int narg) {
    void* lua_mem = arg_require_userdata(l, narg, 
            n_genview_metatable.get(), "pegr.Genre_View");
    return static_cast<Runtime::Genview*>(lua_mem);
}

Script::Unique_Regref initialize_any_udatamt(lua_State* l, const luaL_Reg* mt) {
    assert_balance(0);
    lua_newtable(l);
    luaL_register(l, nullptr, mt);
    lua_pushvalue(l, -1);
    Script::Unique_Regref dest = Script::grab_unique_reference();
    lua_pop(l, 1);
    return dest;
}

void initialize_udatamt_comp(lua_State* l) {
    const luaL_Reg metatable[] = {
        {"__call", li_comp_mt_call},
        // No __eq, since there should only be one global pointer
        // No __gc, since this userdata is POD pointer
        {"__tostring", li_comp_mt_tostring},
        
        // End of the list
        {nullptr, nullptr}
    };
    n_comp_metatable = initialize_any_udatamt(l, metatable);
}
void initialize_udatamt_arche(lua_State* l) {
    const luaL_Reg metatable[] = {
        {"__call", li_arche_mt_call},
        // No __eq, since there should only be one global pointer
        // No __gc, since this userdata is POD pointer
        {"__tostring", li_arche_mt_tostring},
        
        // End of the list
        {nullptr, nullptr}
    };
    n_arche_metatable = initialize_any_udatamt(l, metatable);
}
void initialize_udatamt_genre(lua_State* l) {
    const luaL_Reg metatable[] = {
        {"__call", li_genre_mt_call},
        // No __eq, since there should only be one global pointer
        // No __gc, since this userdata is POD pointer
        {"__tostring", li_genre_mt_tostring},
        
        // End of the list
        {nullptr, nullptr}
    };
    n_genre_metatable = initialize_any_udatamt(l, metatable);
}
void initialize_udatamt_entity(lua_State* l) {
    const luaL_Reg metatable[] = {
        {"__gc", li_entity_mt_gc},
        {"__index", li_entity_mt_index},
        {"__tostring", li_entity_mt_tostring},
        
        // End of the list
        {nullptr, nullptr}
    };
    n_entity_metatable = initialize_any_udatamt(l, metatable);
}
void initialize_udatamt_cview(lua_State* l) {
    const luaL_Reg metatable[] = {
        {"__eq", li_cview_mt_eq},
        {"__gc", li_cview_mt_gc},
        {"__index", li_cview_mt_index},
        {"__newindex", li_cview_mt_newindex},
        {"__tostring", li_cview_mt_tostring},
        
        // End of the list
        {nullptr, nullptr}
    };
    n_cview_metatable = initialize_any_udatamt(l, metatable);
}
void initialize_udatamt_genview(lua_State* l) {
    const luaL_Reg metatable[] = {
        {"__eq", li_genview_mt_eq},
        {"__gc", li_genview_mt_gc},
        {"__index", li_genview_mt_index},
        {"__newindex", li_genview_mt_newindex},
        {"__tostring", li_genview_mt_tostring},
        
        // End of the list
        {nullptr, nullptr}
    };
    n_genview_metatable = initialize_any_udatamt(l, metatable);
}

void initialize_userdata_metatables(lua_State* l) {
    assert_balance(0);
    
    /* All special metatable members, in alphabetical order:
     * add
     * call
     * concat
     * div
     * eq
     * gc
     * index
     * le
     * len
     * lt
     * mod
     * mode
     * mul
     * newindex
     * pow
     * sub
     * tostring
     * unm
     */
     
    initialize_udatamt_comp(l);
    initialize_udatamt_arche(l);
    initialize_udatamt_genre(l);
    initialize_udatamt_entity(l);
    initialize_udatamt_cview(l);
    initialize_udatamt_genview(l);
}

void cleanup_userdata_metatables(lua_State* l) {
    n_comp_metatable.reset();
    n_arche_metatable.reset();
    n_genre_metatable.reset();
    n_entity_metatable.reset();
    n_cview_metatable.reset();
    n_genview_metatable.reset();
}

template <typename Pointer_T>
void push_any_pointer(lua_State* l, Pointer_T ptr, Script::Regref metatable) {
    assert_balance(1);
    if (ptr->m_lua_userdata.is_nil()) {
        void* lua_mem = lua_newuserdata(l, sizeof(Pointer_T));
        Script::push_reference(metatable);
        lua_setmetatable(l, -2);
        Pointer_T& lud = *static_cast<Pointer_T*>(lua_mem);
        lud = ptr;
        lua_pushvalue(l, -1);
        ptr->m_lua_userdata.reset(Script::grab_reference());
    } else {
        Script::push_reference(ptr->m_lua_userdata.get());
    }
}
template <typename Value_T>
void push_any_value(lua_State* l, Value_T val, Script::Regref metatable) {
    assert_balance(1);
    void* lua_mem = lua_newuserdata(l, sizeof(Value_T));
    Script::push_reference(metatable);
    lua_setmetatable(l, -2);
    *(new (lua_mem) Value_T) = val;
}
void push_comp_pointer(lua_State* l, Runtime::Comp* ptr) {
    push_any_pointer<Runtime::Comp*>(l, ptr, n_comp_metatable.get());
}
void push_arche_pointer(lua_State* l, Runtime::Arche* ptr) {
    push_any_pointer<Runtime::Arche*>(l, ptr, n_arche_metatable.get());
}
void push_genre_pointer(lua_State* l, Runtime::Genre* ptr) {
    push_any_pointer<Runtime::Genre*>(l, ptr, n_genre_metatable.get());
}
void push_entity_handle(lua_State* l, Runtime::Entity_Handle ent) {
    push_any_value<Runtime::Entity_Handle>(l, ent, n_entity_metatable.get());
}
void push_cview(lua_State* l, Runtime::Cview cview) {
    push_any_value<Runtime::Cview>(l, cview, n_cview_metatable.get());
}
void push_genview(lua_State* l, Runtime::Genview genview) {
    push_any_value<Runtime::Genview>(l, genview, n_genview_metatable.get());
}

Runtime::Cview make_cview(Runtime::Comp* comp, Runtime::Entity* ent_unsafe) {
    Runtime::Cview retval;
    retval.m_comp = comp;
    retval.m_ent = ent_unsafe->get_handle();
    /* Get the component offset (where this component's data begins within the
     * aggregate arrays stored inside of every instance of this archetype)
     */
    Runtime::Arche* arche = ent_unsafe->get_arche();
    assert(arche);
    assert(arche->m_comp_offsets.find(comp) 
            != arche->m_comp_offsets.end());
    retval.m_cached_aggidx = arche->m_comp_offsets[comp];
    return retval;
}

/**
 * @brief Retrieves the cached cview from the given entity, caching a new
 * cview beforehand if it is not already present in the cache with the given
 * key.
 * 
 * See overload to defer aggidx lookup until absolutely necessary (like lazy
 * evaluation)
 * 
 * @param l The Lua state
 * @param ent_unsafe The pointer to the entity
 * @param key_idx The index of a value already on the Lua stack that used to
 * retrieve the cview from the cache.
 * @param comp The component pointer to use when creating the cview if it was
 * not found in the entity's cache.
 * @param aggidx The cached aggidx to use in the event that this function needs
 * to construct a new cview
 */
void make_cache_push_cview(lua_State* l, Runtime::Entity* ent_unsafe, 
        int key_idx, Runtime::Comp* comp, Runtime::Arche::Aggindex aggidx) {
    assert_balance(1);
    assert(key_idx > 0);
    
    // Get the entity's cview cache, making it if it does not exist
    Script::push_reference(ent_unsafe->get_weak_table());
    
    // Push the component name
    lua_pushvalue(l, key_idx); // +1
    
    // Swap the component name with the cview or nil
    lua_rawget(l, -2); // -1 +1
    
    // No cached cview
    if (lua_isnil(l, -1)) {
        // Pop nil
        lua_pop(l, 1); // -1
        
        //Logger::log()->info("Creating a cview");
        
        // Make a new cview and push it
        Runtime::Cview cview;
        cview.m_comp = comp;
        cview.m_ent = ent_unsafe->get_handle();
        cview.m_cached_aggidx = aggidx;
        push_cview(l, cview); // +1
        
        // Push the component name again so we can store it in the cache
        lua_pushvalue(l, key_idx); // +1
        
        // Push the cview again
        lua_pushvalue(l, -2); // +1
        
        // Set the table
        lua_rawset(l, -4); // -2
        
        // Note that it is impossible for the cview to be gc'd right now
        // because its presence on the stack is a strong reference.
    }
        
    // Remove the cache
    lua_remove(l, -2); // -1
    
    assert(lua_isuserdata(l, -1));
}

/**
 * @brief Retrieves the cached cview from the given entity, caching a new
 * cview beforehand if it is not already present in the cache with the given
 * key.
 * 
 * This overload defers the lookup in the Archetype's table (a somewhat
 * expensive operation) until absolutely necessary (i.e. not already found
 * in the cache.) If the lookup was already performed, use the other overload
 * for this function, which allows you to pass an already-constructed cview.
 * 
 * @param l The Lua state
 * @param ent_unsafe The pointer to the entity
 * @param key_idx The index of a value already on the Lua stack that used to
 * retrieve the cview from the cache.
 * @param comp The component pointer to use when creating the cview if it was
 * not found in the entity's cache.
 */
void make_cache_push_cview(lua_State* l, Runtime::Entity* ent_unsafe, 
        int key_idx, Runtime::Comp* comp) {
    assert_balance(1);
    assert(key_idx > 0);
    
    // Get the entity's cview cache, making it if it does not exist
    Script::push_reference(ent_unsafe->get_weak_table());
    
    // Push the component name
    lua_pushvalue(l, key_idx); // +1
    
    // Swap the component name with the cview or nil
    lua_rawget(l, -2); // -1 +1
    
    // No cached cview
    if (lua_isnil(l, -1)) {
        // Pop nil
        lua_pop(l, 1); // -1
        
        //Logger::log()->info("Creating a cview");
        
        // Make a new cview and push it
        push_cview(l, make_cview(comp, ent_unsafe)); // +1
        
        // Push the component name again so we can store it in the cache
        lua_pushvalue(l, key_idx); // +1
        
        // Push the cview again
        lua_pushvalue(l, -2); // +1
        
        // Set the table
        lua_rawset(l, -4); // -2
        
        // Note that it is impossible for the cview to be gc'd right now
        // because its presence on the stack is a strong reference.
    }
        
    // Remove the cache
    lua_remove(l, -2); // -1
    
    assert(lua_isuserdata(l, -1));
}

int push_member_of_entity(lua_State* l, const Runtime::Member_Ptr& mem_ptr) {
    switch(mem_ptr.get_type()) {
        case Runtime::Prim::Type::I32:
        case Runtime::Prim::Type::I64:
        case Runtime::Prim::Type::F32:
        case Runtime::Prim::Type::F64: {
            lua_pushnumber(l, mem_ptr.get_value_any_number());
            return 1;
        }
        case Runtime::Prim::Type::STR: {
            lua_pushstring(l, mem_ptr.get_value_str().c_str());
            return 1;
        }
        case Runtime::Prim::Type::FUNC: {
            Script::push_reference(mem_ptr.get_value_func());
            return 1;
        }
        case Runtime::Prim::Type::NULLPTR: {
            return 0;
        }
        default: {
            assert(false && "TODO");
        }
    }
}

inline void arg_require_write_compat(lua_State* l, bool compat,
        int value_idx, Runtime::Prim::Type ty) {
    if (!compat) {
        std::stringstream sss;
        sss << lua_typename(l, value_idx)
            << " cannot be assigned to "
            << Runtime::prim_to_dbg_string(ty);
        luaL_argerror(l, value_idx, sss.str().c_str());
    }
}

int write_to_member_of_entity(lua_State* l, const Runtime::Member_Ptr& mem_ptr,
        int idx) {
    Runtime::Prim::Type ty = mem_ptr.get_type();
    switch(ty) {
        case Runtime::Prim::Type::I32:
        case Runtime::Prim::Type::I64:
        case Runtime::Prim::Type::F32:
        case Runtime::Prim::Type::F64: {
            arg_require_write_compat(l, lua_isnumber(l, idx), idx, ty);
            mem_ptr.set_value_any_number(lua_tonumber(l, idx));
            return 0;
        }
        case Runtime::Prim::Type::STR: {
            arg_require_write_compat(l, lua_isstring(l, idx), idx, ty);
            std::size_t data_strlen;
            const char* data_str = lua_tolstring(l, idx, &data_strlen);
            mem_ptr.set_value_str(std::string(data_str, data_strlen));
            return 0;
        }
        case Runtime::Prim::Type::FUNC: {
            arg_require_write_compat(l, lua_isfunction(l, idx), idx, ty);
            mem_ptr.set_value_func(LUA_REFNIL); // Cannot reassign functions
            // Just you wait, this line will give future me a headache ^
            return 0;
        }
        case Runtime::Prim::Type::NULLPTR: {
            assert(false && "Cannot write to nullptr, check beforehand!");
        }
        default: {
            assert(false && "Unhandled write");
        }
    }
}

int li_comp_mt_call(lua_State* l) {
    const int ARG_COMP = 1;
    const int ARG_ENT = 2;
    
    // The first argument is guaranteed to be the right type
    Runtime::Comp* comp = 
            *(static_cast<Runtime::Comp**>(lua_touserdata(l, ARG_COMP)));
    
    Runtime::Entity_Handle ent_h = *arg_require_entity(l, ARG_ENT);
    
    Runtime::Entity* ent_unsafe = ent_h.get_volatile_entity_ptr();
    Runtime::Arche* arche = ent_unsafe->get_arche();
    
    auto agg_iter = arche->m_comp_offsets.find(comp);
    
    // Impossible to match
    if (agg_iter == arche->m_comp_offsets.end()) {
        return 0;
    }
    
    assert_balance(1);
    
    // (PIL 28.5 encourages using lightuserdata as keys)
    lua_pushlightuserdata(l, comp);
    int key_idx = lua_gettop(l);
    make_cache_push_cview(l, ent_unsafe, key_idx, comp, agg_iter->second);
    lua_remove(l, key_idx);
    
    return 1;
}
int li_comp_mt_tostring(lua_State* l) {
    const int ARG_COMP = 1;
    // The first argument is guaranteed to be the right type
    Runtime::Comp* comp = 
            *(static_cast<Runtime::Comp**>(lua_touserdata(l, ARG_COMP)));
    lua_pushstring(l, to_string_comp(comp).c_str());
    return 1;
}

int li_arche_mt_call(lua_State* l) {
    const int ARG_ARCHE = 1;
    const int ARG_ENT = 2;
    // The first argument is guaranteed to be the right type
    Runtime::Arche* arche = 
            *(static_cast<Runtime::Arche**>(lua_touserdata(l, ARG_ARCHE)));
    Runtime::Entity_Handle ent_h = *arg_require_entity(l, ARG_ENT);
    if (ent_h->get_arche() == arche) {
        lua_pushvalue(l, ARG_ENT);
        return 1;
    }
    return 0;
}
int li_arche_mt_tostring(lua_State* l) {
    const int ARG_ARCHE = 1;
    // The first argument is guaranteed to be the right type
    Runtime::Arche* arche = 
            *(static_cast<Runtime::Arche**>(lua_touserdata(l, ARG_ARCHE)));
    lua_pushstring(l, to_string_arche(arche).c_str());
    return 1;
}

int li_genre_mt_call(lua_State* l) {
    assert_balance(0, 1);
    const int ARG_GENRE = 1;
    const int ARG_ENT = 2;
    // The first argument is guaranteed to be the right type
    Runtime::Genre* genre = 
            *(static_cast<Runtime::Genre**>(lua_touserdata(l, ARG_GENRE)));
    Runtime::Entity_Handle ent_h = *arg_require_entity(l, ARG_ENT);
    
    Runtime::Arche* arche = ent_h->get_arche();
    // Maybe cache whether or not this archetype matches?
    if (!Util::is_subset_of_presorted(
            genre->m_sorted_required_intersection, 
            arche->m_sorted_component_array)) {
        //Logger::log()->info("no match");
        // Cannot possibly match
        return 0;
    }
    
    // Important: iterate using references not copies
    for (Runtime::Pattern& pattern : genre->m_patterns) {
        if (Util::is_subset_of_presorted(
                pattern.m_sorted_required_comps_specific, 
                arche->m_sorted_component_array)) {
            // TODO: caching?
            Runtime::Genview genview;
            genview.m_ent = ent_h;
            genview.m_pattern = &pattern;
            
            //Logger::log()->info("found");
            push_genview(l, genview);
            return 1;
        }
        /*
        for (Runtime::Comp* c : pattern.m_sorted_required_comps_specific) {
            Logger::log()->info("require: %v", c);
        }
        for (Runtime::Comp* c : arche->m_sorted_component_array) {
            Logger::log()->info("have: %v", c);
        }
        */
    }
    //Logger::log()->info("no pattern");
    
    // No matches found
    return 0;
}
int li_genre_mt_tostring(lua_State* l) {
    const int ARG_GENRE = 1;
    // The first argument is guaranteed to be the right type
    Runtime::Genre* genre = 
            *(static_cast<Runtime::Genre**>(lua_touserdata(l, ARG_GENRE)));
    lua_pushstring(l, to_string_genre(genre).c_str());
    return 1;
}

int li_entity_mt_gc(lua_State* l) {
    // The first argument is guaranteed to be the right type
    Runtime::Entity_Handle& ent = 
            *(static_cast<Runtime::Entity_Handle*>(lua_touserdata(l, 1)));
    
    /* If the entity exists and can still be spawned, then that means that this
     * entity handle is unique (the last remaining one, since there could not
     * have been any copies made of it)
     */
    if (ent.does_exist() && ent->is_lua_owned()) {
        assert(!ent->has_been_spawned());
        Runtime::Entity::delete_entity(ent);
    }
    
    ent.Runtime::Entity_Handle::~Entity_Handle();
    return 0;
}
int li_entity_mt_index(lua_State* l) {
    assert_balance(0, 1);
    
    const int ARG_ENT = 1;
    const int ARG_MEMBER = 2;
    
    /* 1: The entity userdata
     * 2: Index: string
     */
    // The first argument is guaranteed to be the right type
    Runtime::Entity_Handle ent = 
            *(static_cast<Runtime::Entity_Handle*>(lua_touserdata(l, ARG_ENT)));
    
    std::size_t keystrlen;
    const char* keystr = luaL_checklstring(l, ARG_MEMBER, &keystrlen);
    if (keystrlen >= 2 && keystr[0] == '_' && keystr[1] == '_') {
        const char* special_key = keystr + 2;
        if (std::strcmp(special_key, "id") == 0) {
            lua_pushnumber(l, entity_handle_to_lua_number(ent.get_id()));
            return 1;
        }
        Runtime::Entity* ent_unsafe = ent.get_volatile_entity_ptr();
        if (std::strcmp(special_key, "exists") == 0) {
            lua_pushboolean(l, ent_unsafe != nullptr);
            return 1;
        }
        else if (ent_unsafe != nullptr) {
            if (std::strcmp(special_key, "arche") == 0) {
                push_arche_pointer(l, ent_unsafe->get_arche());
                return 1;
            } else if (std::strcmp(special_key, "killed") == 0) {
                lua_pushboolean(l, ent_unsafe->has_been_killed());
                return 1;
            } else if (std::strcmp(special_key, "alive") == 0) {
                lua_pushboolean(l, ent_unsafe->is_alive());
                return 1;
            } else if (std::strcmp(special_key, "spawned") == 0) {
                lua_pushboolean(l, ent_unsafe->has_been_spawned());
                return 1;
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    } else {
        Runtime::Entity* ent_unsafe = ent.get_volatile_entity_ptr();
        if (ent_unsafe == nullptr) {
            return 0;
        }
        
        const Runtime::Arche* arche = ent_unsafe->get_arche();
        Runtime::Symbol member_str(keystr, keystrlen);
        auto comp_iter = arche->m_components.find(member_str);
        if (comp_iter == arche->m_components.end()) {
            return 0;
        }
        
        make_cache_push_cview(l, ent_unsafe, ARG_MEMBER, comp_iter->second);
        
        return 1;
    }
    return 0;
}
int li_entity_mt_tostring(lua_State* l) {
    // The first argument is guaranteed to be the right type
    Runtime::Entity_Handle ent = 
            *(static_cast<Runtime::Entity_Handle*>(lua_touserdata(l, 1)));
    lua_pushstring(l, to_string_entity(ent).c_str());
    return 1;
}

int li_cview_mt_eq(lua_State* l) {
    const int ARG_LHS = 1;
    const int ARG_RHS = 2;
    
    // Both guaranteed: metatables are equal and both are userdata
    Runtime::Cview& lhs = 
            *(static_cast<Runtime::Cview*>(lua_touserdata(l, ARG_LHS)));
    Runtime::Cview& rhs = 
            *(static_cast<Runtime::Cview*>(lua_touserdata(l, ARG_RHS)));
    
    lua_pushboolean(l, lhs == rhs);
    return 1;
}
int li_cview_mt_gc(lua_State* l) {
    // The first argument is guaranteed to be the right type
    Runtime::Cview& cview = 
            *(static_cast<Runtime::Cview*>(lua_touserdata(l, 1)));
    cview.Runtime::Cview::~Cview();
    return 0;
}
int li_cview_mt_index(lua_State* l) {
    const int ARG_CVIEW = 1;
    const int ARG_MEMBER = 2;
    Runtime::Cview& cview = 
            *(static_cast<Runtime::Cview*>(lua_touserdata(l, ARG_CVIEW)));
    std::size_t keystrlen;
    const char* keystr = luaL_checklstring(l, ARG_MEMBER, &keystrlen);
    Runtime::Member_Ptr mem_ptr = 
            cview.get_member_ptr(Runtime::Symbol(keystr, keystrlen));
    return push_member_of_entity(l, mem_ptr);
}
int li_cview_mt_newindex(lua_State* l) {
    const int ARG_CVIEW = 1;
    const int ARG_MEMBER = 2;
    const int ARG_ASSIGN = 3;
    Runtime::Cview& cview = 
            *(static_cast<Runtime::Cview*>(lua_touserdata(l, ARG_CVIEW)));
    std::size_t keystrlen;
    const char* keystr = luaL_checklstring(l, ARG_MEMBER, &keystrlen);
    Runtime::Member_Ptr mem_ptr = 
            cview.get_member_ptr(Runtime::Symbol(keystr, keystrlen));
    if (mem_ptr.is_nullptr()) {
        std::stringstream sss;
        sss << "Tried to write to nonexistent component member \""
            << keystr
            << '"';
        luaL_error(l, sss.str().c_str());
    }
    try {
        return write_to_member_of_entity(l, mem_ptr, ARG_ASSIGN);
    } catch (Except::Runtime& e) {
        std::stringstream sss;
        sss << "Failed to write to component member \""
            << keystr
            << ": "
            << e.what();
        luaL_error(l, sss.str().c_str());
    }
}
int li_cview_mt_tostring(lua_State* l) {
    // The first argument is guaranteed to be the right type
    Runtime::Cview& cview = 
            *(static_cast<Runtime::Cview*>(lua_touserdata(l, 1)));
    lua_pushstring(l, to_string_cview(cview).c_str());
    return 1;
}

int li_genview_mt_eq(lua_State* l) {
    const int ARG_LHS = 1;
    const int ARG_RHS = 2;
    
    // Both guaranteed: metatables are equal and both are userdata
    Runtime::Genview& lhs = 
            *(static_cast<Runtime::Genview*>(lua_touserdata(l, ARG_LHS)));
    Runtime::Genview& rhs = 
            *(static_cast<Runtime::Genview*>(lua_touserdata(l, ARG_RHS)));
    
    lua_pushboolean(l, lhs == rhs);
    return 1;
}
int li_genview_mt_gc(lua_State* l) {
    // The first argument is guaranteed to be the right type
    Runtime::Genview& genview = 
            *(static_cast<Runtime::Genview*>(lua_touserdata(l, 1)));
    genview.Runtime::Genview::~Genview();
    return 0;
}
int li_genview_mt_index(lua_State* l) {
    const int ARG_GENVIEW = 1;
    const int ARG_MEMBER = 2;
    Runtime::Genview& genview = 
            *(static_cast<Runtime::Genview*>(lua_touserdata(l, ARG_GENVIEW)));
    std::size_t keystrlen;
    const char* keystr = luaL_checklstring(l, ARG_MEMBER, &keystrlen);
    Runtime::Member_Ptr mem_ptr = 
            genview.get_member_ptr(Runtime::Symbol(keystr, keystrlen));
    return push_member_of_entity(l, mem_ptr);
}
int li_genview_mt_newindex(lua_State* l) {
    const int ARG_GENVIEW = 1;
    const int ARG_MEMBER = 2;
    const int ARG_ASSIGN = 3;
    Runtime::Genview& genview = 
            *(static_cast<Runtime::Genview*>(lua_touserdata(l, ARG_GENVIEW)));
    std::size_t keystrlen;
    const char* keystr = luaL_checklstring(l, ARG_MEMBER, &keystrlen);
    Runtime::Member_Ptr mem_ptr = 
            genview.get_member_ptr(Runtime::Symbol(keystr, keystrlen));
    if (mem_ptr.is_nullptr()) {
        std::stringstream sss;
        sss << "Tried to write to nonexistent genre member \""
            << keystr
            << '"';
        luaL_error(l, sss.str().c_str());
    }
    try {
        return write_to_member_of_entity(l, mem_ptr, ARG_ASSIGN);
    } catch (Except::Runtime& e) {
        std::stringstream sss;
        sss << "Failed to write to genre member \""
            << keystr
            << ": "
            << e.what();
        luaL_error(l, sss.str().c_str());
    }
}
int li_genview_mt_tostring(lua_State* l) {
    // The first argument is guaranteed to be the right type
    Runtime::Genview& genview = 
            *(static_cast<Runtime::Genview*>(lua_touserdata(l, 1)));
    lua_pushstring(l, to_string_genview(genview).c_str());
    return 1;
}

int li_find_comp(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::EXECUTABLE) {
        luaL_error(l, "find_comp is only available during execution");
    }
    std::size_t strlen;
    const char* strdata = luaL_checklstring(l, 1, &strlen);
    std::string key(strdata, strlen);
    
    Runtime::Comp* comp = Runtime::find_component(key);
    if (!comp) {
        return 0;
    }
    
    push_comp_pointer(l, comp);
    
    return 1;
}
int li_find_archetype(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::EXECUTABLE) {
        luaL_error(l, "find_archetype is only available during execution");
    }
    std::size_t strlen;
    const char* strdata = luaL_checklstring(l, 1, &strlen);
    std::string key(strdata, strlen);
    
    Runtime::Arche* arche = Runtime::find_archetype(key);
    if (!arche) {
        return 0;
    }
    
    push_arche_pointer(l, arche);
    
    return 1;
}
int li_find_genre(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::EXECUTABLE) {
        luaL_error(l, "find_genre is only available during execution");
    }
    std::size_t strlen;
    const char* strdata = luaL_checklstring(l, 1, &strlen);
    std::string key(strdata, strlen);
    
    Runtime::Genre* genre = Runtime::find_genre(key);
    if (!genre) {
        return 0;
    }
    
    push_genre_pointer(l, genre);
    
    return 1;
}
int li_new_entity(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::EXECUTABLE) {
        luaL_error(l, "new_entity is only available during execution");
    }
    
    Runtime::Arche* arche = *arg_require_arche(l, 1);
    
    Runtime::Entity_Handle ent = Runtime::Entity::new_entity(arche);
    ent->set_flag_lua_owned(true);
    assert(ent->is_lua_owned());
    assert(ent->can_be_spawned());
    
    push_entity_handle(l, ent);
    
    return 1;
}
int li_delete_entity(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::EXECUTABLE) {
        luaL_error(l, "delete_entity is only available during execution");
    }
    
    Runtime::Entity_Handle ent = *arg_require_entity(l, 1);
    
    if (!(ent.does_exist() && ent->is_lua_owned())) {
        lua_pushboolean(l , false);
        return 1;
    }

    assert(!ent->has_been_spawned());
    Runtime::Entity::delete_entity(ent);
    
    lua_pushboolean(l , true);
    return 1;
}


} // namespace LI
} // namespace Gensys
} // namespace pegr
