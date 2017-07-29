#include "pegr/gensys/Lua_Interf.hpp"

#include <stdexcept>
#include <sstream>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include <map>

#include "pegr/debug/Debug_Macros.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/gensys/Compiler.hpp"
#include "pegr/script/Script_Helper.hpp"
#include "pegr/gensys/Runtime.hpp"
#include "pegr/gensys/Gensys.hpp"

namespace pegr {
namespace Gensys {
namespace LI {

Script::Regref n_comp_metatable = LUA_REFNIL;
Script::Regref n_arche_metatable = LUA_REFNIL;
Script::Regref n_genre_metatable = LUA_REFNIL;
Script::Regref n_entity_metatable = LUA_REFNIL;
Script::Regref n_cview_metatable = LUA_REFNIL;

/**
 * @param num A 64 bit unsigned value
 * @return The lower 52 bits
 */
uint64_t bottom_52(uint64_t num) {
    //             0123456789abcdef
    return num & 0x001FFFFFFFFFFFFF;
}
lua_Number entity_handle_to_lua_number(uint64_t data) {
    // Shave off the bottom 52 bits and cast to number
    return static_cast<lua_Number>(bottom_52(data));
}

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
            const char* tname) {
    void* retval = to_mt_userdata(l, narg, metatable);
    if (!retval) {
        luaL_typerror(l, narg, tname);
    }
    return retval;
}

Runtime::Comp** arg_require_comp(lua_State* l, int idx) {
    void* lua_mem = arg_require_userdata(l, idx, 
            n_comp_metatable, "pegr.Component");
    return static_cast<Runtime::Comp**>(lua_mem);
}
Runtime::Arche** arg_require_arche(lua_State* l, int idx) {
    void* lua_mem = arg_require_userdata(l, idx, 
            n_arche_metatable, "pegr.Archetype");
    return static_cast<Runtime::Arche**>(lua_mem);
}
Runtime::Entity_Handle* arg_require_entity(lua_State* l, int idx) {
    void* lua_mem = arg_require_userdata(l, idx, 
            n_entity_metatable, "pegr.Entity");
    return static_cast<Runtime::Entity_Handle*>(lua_mem);
}
Cview* arg_require_cview(lua_State* l, int idx) {
    void* lua_mem = arg_require_userdata(l, idx, 
            n_cview_metatable, "pegr.Comp_View");
    return static_cast<Cview*>(lua_mem);
}

void initialize_any_udatamt(lua_State* l, Script::Regref& dest, 
        const luaL_Reg* metatable) {
    assert_balance(0);
    lua_newtable(l);
    luaL_register(l, nullptr, metatable);
    lua_pushvalue(l, -1);
    dest = Script::grab_reference();
    lua_pop(l, 1);
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
    initialize_any_udatamt(l, n_comp_metatable, metatable);
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
    initialize_any_udatamt(l, n_arche_metatable, metatable);
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
    initialize_any_udatamt(l, n_genre_metatable, metatable);
}
void initialize_udatamt_entity(lua_State* l) {
    const luaL_Reg metatable[] = {
        {"__gc", li_entity_mt_gc},
        {"__index", li_entity_mt_index},
        {"__tostring", li_entity_mt_tostring},
        
        // End of the list
        {nullptr, nullptr}
    };
    initialize_any_udatamt(l, n_entity_metatable, metatable);
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
    initialize_any_udatamt(l, n_cview_metatable, metatable);
}

void initialize_userdata_metatables(lua_State* l) {
    assert_balance(0);
    
    /* add
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
}

void cleanup_userdata_metatables(lua_State* l) {
    Script::drop_reference(n_comp_metatable);
    Script::drop_reference(n_arche_metatable);
    Script::drop_reference(n_genre_metatable);
    Script::drop_reference(n_entity_metatable);
    Script::drop_reference(n_cview_metatable);
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
    push_any_pointer<Runtime::Comp*>(l, ptr, n_comp_metatable);
}
void push_arche_pointer(lua_State* l, Runtime::Arche* ptr) {
    push_any_pointer<Runtime::Arche*>(l, ptr, n_arche_metatable);
}
void push_genre_pointer(lua_State* l, Runtime::Genre* ptr) {
    push_any_pointer<Runtime::Genre*>(l, ptr, n_genre_metatable);
}
void push_entity_handle(lua_State* l, Runtime::Entity_Handle ent) {
    push_any_value<Runtime::Entity_Handle>(l, ent, n_entity_metatable);
}
void push_cview(lua_State* l, Cview cview) {
    push_any_value<Cview>(l, cview, n_cview_metatable);
}

Cview make_cview(Runtime::Comp* comp, Runtime::Entity* ent_unsafe) {
    Cview retval;
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
        Cview cview;
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

std::string to_string_cview(Cview cview) {
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

void check_write_compatible_prim_type(lua_State* l, 
        Runtime::Prim::Type t, int idx) {
    // Check that the correct type was given
    bool correct = false;
    switch (t) {
        case Runtime::Prim::Type::I32:
        case Runtime::Prim::Type::I64:
        case Runtime::Prim::Type::F32:
        case Runtime::Prim::Type::F64: {
            // (Can't cache the lua type, as this function also conditionally
            // accepts strings as numbers)
            if (lua_isnumber(l, idx)) correct = true;
            break;
        }
        case Runtime::Prim::Type::STR: {
            if (lua_isstring(l, idx)) correct = true;
            break;
        }
        case Runtime::Prim::Type::FUNC: {
            if (lua_isfunction(l, idx)) correct = true;
            break;
        }
    }
    if (!correct) {
        luaL_error(l, "Expected %s, got %s", 
                Runtime::prim_typename(t), lua_typename(l, 3));
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
    const int ARG_GENRE = 1;
    const int ARG_ENT = 2;
    // The first argument is guaranteed to be the right type
    Runtime::Genre* genre = 
            *(static_cast<Runtime::Genre**>(lua_touserdata(l, ARG_GENRE)));
            
    // TODO
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
    Runtime::Entity* ent_unsafe = ent.get_volatile_entity_ptr();
    
    std::size_t keystrlen;
    const char* keystr = luaL_checklstring(l, ARG_MEMBER, &keystrlen);
    if (keystrlen >= 2 && keystr[0] == '_' && keystr[1] == '_') {
        const char* special_key = keystr + 2;
        if (std::strcmp(special_key, "id") == 0) {
            lua_pushnumber(l, entity_handle_to_lua_number(ent.get_id()));
            return 1;
        }
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
        if (ent_unsafe == nullptr) {
            return 0;
        }
        
        const Runtime::Arche* arche = ent_unsafe->get_arche();
        Runtime::Symbol member_str(keystr, keystrlen);
        //Logger::log()->info(member_str);
        auto comp_iter = arche->m_components.find(member_str);
        
        /*for (auto entry : arche->m_components) {
            Logger::log()->info("k: %v, v: %v", entry.first, entry.second);
        }*/
        
        if (comp_iter == arche->m_components.end()) {
            return 0;
        }
        
        make_cache_push_cview(l, ent_unsafe, ARG_MEMBER, comp_iter->second);
        
        // Return the cview
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
    Cview& lhs = *(static_cast<Cview*>(lua_touserdata(l, ARG_LHS)));
    Cview& rhs = *(static_cast<Cview*>(lua_touserdata(l, ARG_RHS)));
    
    Logger::log()->info("compare, %v %v", lhs.m_comp, rhs.m_comp);
    
    lua_pushboolean(l, lhs.m_comp == rhs.m_comp && lhs.m_ent == rhs.m_ent);
    return 1;
}
int li_cview_mt_gc(lua_State* l) {
    // The first argument is guaranteed to be the right type
    Cview& cview = *(static_cast<Cview*>(lua_touserdata(l, 1)));
    cview.Cview::~Cview();
    return 0;
}
int li_cview_mt_index(lua_State* l) {
    Cview& cview = *(static_cast<Cview*>(lua_touserdata(l, 1)));
    
    Runtime::Entity* ent_unsafe = cview.m_ent.get_volatile_entity_ptr();
    if (!ent_unsafe) {
        return 0;
    }
    
    std::size_t keystrlen;
    const char* keystr = luaL_checklstring(l, 2, &keystrlen);
    
    if (!keystr) {
        return 0;
    }
    
    // Member symbol argument
    Runtime::Symbol member_symb(keystr, keystrlen);
    
    // Find where the member is stored within the component
    auto offset_entry = cview.m_comp->m_member_offsets.find(member_symb);
    if (offset_entry == cview.m_comp->m_member_offsets.end()) {
        return 0;
    }
    Runtime::Prim member_signature = offset_entry->second;
    
    /* Depending on the member's type, where we read the data and how we
     * intepret it changes. For POD types, the data comes from the chunk. Other
     * types are stored in other arrays. Note that the member signature uses
     * a union to store the different offsets, and so there is only one defined
     * way to read the data.
     */
    switch(member_signature.m_type) {
        case Runtime::Prim::Type::I32:
        case Runtime::Prim::Type::I64:
        case Runtime::Prim::Type::F32:
        case Runtime::Prim::Type::F64: {
            std::size_t pod_offset = Runtime::ENT_HEADER_SIZE
                                    + cview.m_cached_aggidx.m_pod_idx 
                                    + member_signature.m_refer.m_byte_offset;
            switch(member_signature.m_type) {
                case Runtime::Prim::Type::I32: {
                    int32_t val = ent_unsafe->get_chunk()
                            .get_value<int32_t>(pod_offset);
                    lua_pushnumber(l, val);
                    return 1;
                }
                case Runtime::Prim::Type::I64: {
                    int64_t val = ent_unsafe->get_chunk()
                            .get_value<int64_t>(pod_offset);
                    lua_pushnumber(l, val);
                    return 1;
                }
                case Runtime::Prim::Type::F32: {
                    float val = ent_unsafe->get_chunk()
                            .get_value<float>(pod_offset);
                    lua_pushnumber(l, val);
                    return 1;
                }
                case Runtime::Prim::Type::F64: {
                    double val = ent_unsafe->get_chunk()
                            .get_value<double>(pod_offset);
                    lua_pushnumber(l, val);
                    return 1;
                }
                default: {
                    assert(0);
                }
            }
        }
        case Runtime::Prim::Type::STR: {
            std::size_t string_idx = cview.m_cached_aggidx.m_string_idx
                                    + member_signature.m_refer.m_index;
            lua_pushstring(l, ent_unsafe->get_string(string_idx).c_str());
            return 1;
        }
        default: {
            assert(false && "TODO");
        }
    }
}
int li_cview_mt_newindex(lua_State* l) {
    Cview& cview = *(static_cast<Cview*>(lua_touserdata(l, 1)));
    
    Runtime::Entity* ent_unsafe = cview.m_ent.get_volatile_entity_ptr();
    if (!ent_unsafe) {
        return 0;
    }
    
    std::size_t keystrlen;
    const char* keystr = luaL_checklstring(l, 2, &keystrlen);
    
    if (!keystr) {
        return 0;
    }
    
    // Member symbol argument
    Runtime::Symbol member_symb(keystr, keystrlen);
    
    // Find where the member is stored within the component
    auto offset_entry = cview.m_comp->m_member_offsets.find(member_symb);
    if (offset_entry == cview.m_comp->m_member_offsets.end()) {
        return 0;
    }
    Runtime::Prim member_signature = offset_entry->second;
    
    // Check that the given Lua value is assignable to the primitive
    check_write_compatible_prim_type(l, member_signature.m_type, 3);
    
    /* Depending on the member's type, where we write the data and how we
     * intepret it changes. For POD types, the data comes from the chunk. Other
     * types are stored in other arrays. Note that the member signature uses
     * a union to store the different offsets, and so there is only one defined
     * way to read the data.
     */
    switch(member_signature.m_type) {
        case Runtime::Prim::Type::I32:
        case Runtime::Prim::Type::I64:
        case Runtime::Prim::Type::F32:
        case Runtime::Prim::Type::F64: {
            
            std::size_t pod_offset = Runtime::ENT_HEADER_SIZE
                                    + cview.m_cached_aggidx.m_pod_idx 
                                    + member_signature.m_refer.m_byte_offset;
            switch(member_signature.m_type) {
                case Runtime::Prim::Type::I32: {
                    assert(lua_isnumber(l, 3));
                    int32_t val = lua_tonumber(l, 3);
                    ent_unsafe->get_chunk()
                            .set_value<int32_t>(pod_offset, val);
                    return 1;
                }
                case Runtime::Prim::Type::I64: {
                    assert(lua_isnumber(l, 3));
                    int64_t val = lua_tonumber(l, 3);
                    ent_unsafe->get_chunk()
                            .set_value<int64_t>(pod_offset, val);
                    return 1;
                }
                case Runtime::Prim::Type::F32: {
                    assert(lua_isnumber(l, 3));
                    float val = lua_tonumber(l, 3);
                    ent_unsafe->get_chunk()
                            .set_value<float>(pod_offset, val);
                    return 1;
                }
                case Runtime::Prim::Type::F64: {
                    assert(lua_isnumber(l, 3));
                    double val = lua_tonumber(l, 3);
                    ent_unsafe->get_chunk()
                            .set_value<double>(pod_offset, val);
                    return 1;
                }
                default: {
                    assert(0);
                    return 0;
                }
            }
        }
        case Runtime::Prim::Type::STR: {
            std::size_t string_idx = cview.m_cached_aggidx.m_string_idx
                                    + member_signature.m_refer.m_index;
            std::size_t string_len;
            const char* string_data = lua_tolstring(l, 3, &string_len);
            ent_unsafe->set_string(string_idx, 
                    std::string(string_data, string_len));
            return 1;
        }
        default: {
            assert(false && "TODO");
            return 0;
        }
    }
}
int li_cview_mt_tostring(lua_State* l) {
    // The first argument is guaranteed to be the right type
    Cview& cview = *(static_cast<Cview*>(lua_touserdata(l, 1)));
    lua_pushstring(l, to_string_cview(cview).c_str());
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
