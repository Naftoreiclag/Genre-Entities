#include "pegr/gensys/Lua_Interf.hpp"

#include <stdexcept>
#include <sstream>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include <map>

#include "pegr/debug/DebugMacros.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/gensys/Compiler.hpp"
#include "pegr/script/ScriptHelper.hpp"
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

void* check_user_data(lua_State* l, int narg, Script::Regref metatable,
            const char* tname) {
    void* ptr = lua_touserdata(l, narg);
    if (ptr) {
        if (lua_getmetatable(l, narg)) {
            Script::Pop_Guard pg(1);
            Script::push_reference(metatable);
            pg.on_push(1);
            if (lua_rawequal(l, -1, -2)) {
                return ptr;
            }
        }
    }
    luaL_typerror(l, narg, tname);
    return nullptr;
}

Runtime::Arche** argcheck_archetype(lua_State* l, int idx) {
    void* lua_mem = check_user_data(l, 1, n_arche_metatable, "pegr.Archetype");
    return static_cast<Runtime::Arche**>(lua_mem);
}
Runtime::Entity_Handle* argcheck_entity(lua_State* l, int idx) {
    void* lua_mem = check_user_data(l, 1, n_entity_metatable, "pegr.Entity");
    return static_cast<Runtime::Entity_Handle*>(lua_mem);
}

void initialize_userdata_metatables(lua_State* l) {
    assert_balance(0);
    
    lua_newtable(l);
    Script::Pop_Guard popg(1);
    lua_pushvalue(l, -1);
    n_comp_metatable = Script::grab_reference();
    {
        const luaL_Reg metatable[] = {
            // TODO
            
            // End of the list
            {nullptr, nullptr}
        };
        luaL_register(l, nullptr, metatable);
    }
    popg.pop(1);
    
    lua_newtable(l);
    popg.on_push(1);
    lua_pushvalue(l, -1);
    n_arche_metatable = Script::grab_reference();
    {
        const luaL_Reg metatable[] = {
            {"__tostring", li_arche_mt_tostring},
            // No __equal, since there should only be one global pointer
            // No __gc, since this userdata is POD pointer
            
            // End of the list
            {nullptr, nullptr}
        };
        luaL_register(l, nullptr, metatable);
    }
    popg.pop(1);

    lua_newtable(l);
    popg.on_push(1);
    lua_pushvalue(l, -1);
    n_entity_metatable = Script::grab_reference();
    {
        const luaL_Reg metatable[] = {
            {"__gc", li_entity_mt_gc},
            {"__index", li_entity_mt_index},
            {"__tostring", li_entity_mt_tostring},
            
            // End of the list
            {nullptr, nullptr}
        };
        luaL_register(l, nullptr, metatable);
    }
    popg.pop(1);
    
    lua_newtable(l);
    popg.on_push(1);
    lua_pushvalue(l, -1);
    n_cview_metatable = Script::grab_reference();
    {
        const luaL_Reg metatable[] = {
            {"__gc", li_cview_mt_gc},
            {"__index", li_cview_mt_index},
            {"__newindex", li_cview_mt_newindex},
            {"__tostring", li_cview_mt_tostring},
            
            // End of the list
            {nullptr, nullptr}
        };
        luaL_register(l, nullptr, metatable);
    }
    popg.pop(1);
}

void cleanup_userdata_metatables(lua_State* l) {
    Script::drop_reference(n_comp_metatable);
    Script::drop_reference(n_arche_metatable);
    Script::drop_reference(n_genre_metatable);
    Script::drop_reference(n_entity_metatable);
    Script::drop_reference(n_cview_metatable);
}

void push_arche_pointer(lua_State* l, Runtime::Arche* ptr) {
    if (ptr->m_lua_userdata.is_nil()) {
        void* lua_mem = lua_newuserdata(l, sizeof(Runtime::Arche*));
        Script::push_reference(n_arche_metatable);
        lua_setmetatable(l, -2);
        Runtime::Arche*& lud_arche = *static_cast<Runtime::Arche**>(lua_mem);
        lud_arche = ptr;
        
        lua_pushvalue(l, -1);
        ptr->m_lua_userdata.reset(Script::grab_reference());
    } else {
        Script::push_reference(ptr->m_lua_userdata.get());
    }
}

void push_entity_handle(lua_State* l, Runtime::Entity_Handle ent) {
    void* lua_mem = lua_newuserdata(l, sizeof(Runtime::Entity_Handle));
    Script::push_reference(n_entity_metatable);
    lua_setmetatable(l, -2);
    *(new (lua_mem) Runtime::Entity_Handle) = ent;
}

void push_cview(lua_State* l, Cview cview) {
    void* lua_mem = lua_newuserdata(l, sizeof(Cview));
    Script::push_reference(n_cview_metatable);
    lua_setmetatable(l, -2);
    *(new (lua_mem) Cview) = cview;
}

std::string to_string_arche(Runtime::Arche* arche) {
    std::stringstream sss;
    sss << "<Archetype @"
        << arche
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

int li_arche_mt_tostring(lua_State* l) {
    // The first argument is guaranteed to be the right type
    Runtime::Arche* arche = 
            *(static_cast<Runtime::Arche**>(lua_touserdata(l, 1)));
    lua_pushstring(l, to_string_arche(arche).c_str());
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
    /* 1: The entity userdata
     * 2: Index: string
     */
    // The first argument is guaranteed to be the right type
    Runtime::Entity_Handle ent = 
            *(static_cast<Runtime::Entity_Handle*>(lua_touserdata(l, 1)));
    
    std::size_t keystrlen;
    const char* keystr = luaL_checklstring(l, 2, &keystrlen);
    if (keystrlen >= 2 && keystr[0] == '_' && keystr[1] == '_') {
        const char* special_key = keystr + 2;
        if (std::strcmp(special_key, "id") == 0) {
            lua_pushnumber(l, entity_handle_to_lua_number(ent.get_id()));
            return 1;
        }
        if (std::strcmp(special_key, "exists") == 0) {
            lua_pushboolean(l, ent.does_exist());
            return 1;
        }
        else if (ent.does_exist()) {
            if (std::strcmp(special_key, "arche") == 0) {
                push_arche_pointer(l, ent->get_arche());
                return 1;
            } else if (std::strcmp(special_key, "killed") == 0) {
                lua_pushboolean(l, ent->has_been_killed());
                return 1;
            } else if (std::strcmp(special_key, "alive") == 0) {
                lua_pushboolean(l, ent->is_alive());
                return 1;
            } else if (std::strcmp(special_key, "spawned") == 0) {
                lua_pushboolean(l, ent->has_been_spawned());
                return 1;
            }
        }
    } else {
        if (!ent.does_exist()) {
            return 0;
        }
        
        const Runtime::Arche* arche = ent->get_arche();
        Runtime::Symbol member_str(keystr, keystrlen);
        //Logger::log()->info(member_str);
        auto comp_iter = arche->m_components.find(member_str);
        
        /*for (auto entry : arche->m_components) {
            Logger::log()->info("k: %v, v: %v", entry.first, entry.second);
        }*/
        
        if (comp_iter == arche->m_components.end()) {
            return 0;
        }
        
        Cview cview;
        cview.m_comp = comp_iter->second;
        cview.m_ent = ent;
        //Logger::log()->info("%v", cview.m_comp);
        
        push_cview(l, cview);
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

int li_cview_mt_gc(lua_State* l) {
    // The first argument is guaranteed to be the right type
    Cview& cview = *(static_cast<Cview*>(lua_touserdata(l, 1)));
    cview.Cview::~Cview();
    return 0;
}
int li_cview_mt_index(lua_State* l) {
    Cview& cview = *(static_cast<Cview*>(lua_touserdata(l, 1)));
    
    if (!cview.m_ent.does_exist()) {
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
    
    /* Get the component offset (where this component's data begins within the
     * aggregate arrays stored inside of every instance of this archetype)
     */
    Runtime::Arche* arche = cview.m_ent->get_arche();
    assert(arche);
    assert(arche->m_comp_offsets.find(cview.m_comp) 
            != arche->m_comp_offsets.end());
    Runtime::Arche::Aggindex component_offset = 
            arche->m_comp_offsets[cview.m_comp];
    
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
                                    + component_offset.m_pod_idx 
                                    + member_signature.m_refer.m_byte_offset;
            switch(member_signature.m_type) {
                case Runtime::Prim::Type::I32: {
                    int32_t val = cview.m_ent->get_chunk()
                            .get_value<int32_t>(pod_offset);
                    lua_pushnumber(l, val);
                    return 1;
                }
                case Runtime::Prim::Type::I64: {
                    int64_t val = cview.m_ent->get_chunk()
                            .get_value<int64_t>(pod_offset);
                    lua_pushnumber(l, val);
                    return 1;
                }
                case Runtime::Prim::Type::F32: {
                    float val = cview.m_ent->get_chunk()
                            .get_value<float>(pod_offset);
                    lua_pushnumber(l, val);
                    return 1;
                }
                case Runtime::Prim::Type::F64: {
                    double val = cview.m_ent->get_chunk()
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
            std::size_t string_idx = component_offset.m_string_idx
                                    + member_signature.m_refer.m_index;
            lua_pushstring(l, cview.m_ent->get_string(string_idx).c_str());
            return 1;
        }
        default: {
            assert(false && "TODO");
        }
    }
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

int li_cview_mt_newindex(lua_State* l) {
    Cview& cview = *(static_cast<Cview*>(lua_touserdata(l, 1)));
    
    if (!cview.m_ent.does_exist()) {
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
    
    /* Get the component offset (where this component's data begins within the
     * aggregate arrays stored inside of every instance of this archetype)
     */
    Runtime::Arche* arche = cview.m_ent->get_arche();
    assert(arche);
    assert(arche->m_comp_offsets.find(cview.m_comp) 
            != arche->m_comp_offsets.end());
    Runtime::Arche::Aggindex component_offset = 
            arche->m_comp_offsets[cview.m_comp];
    
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
                                    + component_offset.m_pod_idx 
                                    + member_signature.m_refer.m_byte_offset;
            switch(member_signature.m_type) {
                case Runtime::Prim::Type::I32: {
                    assert(lua_isnumber(l, 3));
                    int32_t val = lua_tonumber(l, 3);
                    cview.m_ent->get_chunk()
                            .set_value<int32_t>(pod_offset, val);
                    return 1;
                }
                case Runtime::Prim::Type::I64: {
                    assert(lua_isnumber(l, 3));
                    int64_t val = lua_tonumber(l, 3);
                    cview.m_ent->get_chunk()
                            .set_value<int64_t>(pod_offset, val);
                    return 1;
                }
                case Runtime::Prim::Type::F32: {
                    assert(lua_isnumber(l, 3));
                    float val = lua_tonumber(l, 3);
                    cview.m_ent->get_chunk()
                            .set_value<float>(pod_offset, val);
                    return 1;
                }
                case Runtime::Prim::Type::F64: {
                    assert(lua_isnumber(l, 3));
                    double val = lua_tonumber(l, 3);
                    cview.m_ent->get_chunk()
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
            std::size_t string_idx = component_offset.m_string_idx
                                    + member_signature.m_refer.m_index;
            std::size_t string_len;
            const char* string_data = lua_tolstring(l, 3, &string_len);
            cview.m_ent->set_string(string_idx, 
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
    
    Runtime::Arche* arche = *argcheck_archetype(l, 1);
    
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
    
    Runtime::Entity_Handle ent = *argcheck_entity(l, 1);
    
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
