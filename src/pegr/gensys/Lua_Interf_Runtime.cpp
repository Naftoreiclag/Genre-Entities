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

// MTI = metatable id
const char* MTI_COMPONENT = "pegr.Component";
const char* MTI_ARCHETYPE = "pegr.Archetype";
const char* MTI_GENRE = "pegr.Genre";
const char* MTI_ENTITY = "pegr.Entity";
const char* MTI_COMP_VIEW = "pegr.Component_View";


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

Runtime::Arche** argcheck_archetype(lua_State* l, int idx) {
    void* lua_mem = luaL_checkudata(l, 1, MTI_ARCHETYPE);
    return static_cast<Runtime::Arche**>(lua_mem);
}
Runtime::Entity_Handle* argcheck_entity(lua_State* l, int idx) {
    void* lua_mem = luaL_checkudata(l, 1, MTI_ENTITY);
    return static_cast<Runtime::Entity_Handle*>(lua_mem);
}

void initialize_userdata_metatables(lua_State* l) {
    assert_balance(0);
    
    int success = luaL_newmetatable(l, MTI_COMPONENT);
    Script::Pop_Guard popg(1);
    assert(success && "Component metatable id already taken!");
    {
        const luaL_Reg metatable[] = {
            // TODO
            
            // End of the list
            {nullptr, nullptr}
        };
        luaL_register(l, nullptr, metatable);
    }
    popg.pop(1);
    
    success = luaL_newmetatable(l, MTI_ARCHETYPE);
    popg.on_push(1);
    assert(success && "Archetype metatable id already taken!");
    {
        const luaL_Reg metatable[] = {
            {"__tostring", li_arche_mt_tostring},
            // No __gc, since this userdata is POD pointer
            
            // End of the list
            {nullptr, nullptr}
        };
        luaL_register(l, nullptr, metatable);
    }
    popg.pop(1);

    success = luaL_newmetatable(l, MTI_ENTITY);
    popg.on_push(1);
    assert(success && "Entity metatable id already taken!");
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
    
    success = luaL_newmetatable(l, MTI_COMP_VIEW);
    popg.on_push(1);
    assert(success && "Component view metatable id already taken!");
    {
        const luaL_Reg metatable[] = {
            {"__gc", li_cview_mt_gc},
            {"__index", li_cview_mt_index},
            {"__tostring", li_cview_mt_tostring},
            
            // End of the list
            {nullptr, nullptr}
        };
        luaL_register(l, nullptr, metatable);
    }
    popg.pop(1);
}

void push_arche_pointer(lua_State* l, Runtime::Arche* ptr) {
    void* lua_mem = lua_newuserdata(l, sizeof(Runtime::Arche*));
    luaL_getmetatable(l, MTI_ARCHETYPE);
    lua_setmetatable(l, -2);
    Runtime::Arche*& lud_arche = *static_cast<Runtime::Arche**>(lua_mem);
    lud_arche = ptr;
}

void push_entity_handle(lua_State* l, Runtime::Entity_Handle ent) {
    void* lua_mem = lua_newuserdata(l, sizeof(Runtime::Entity_Handle));
    luaL_getmetatable(l, MTI_ENTITY);
    lua_setmetatable(l, -2);
    *(new (lua_mem) Runtime::Entity_Handle) = ent;
}

void push_cview(lua_State* l, Cview cview) {
    void* lua_mem = lua_newuserdata(l, sizeof(Cview));
    luaL_getmetatable(l, MTI_COMP_VIEW);
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
    // TODO
    return 0;
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
