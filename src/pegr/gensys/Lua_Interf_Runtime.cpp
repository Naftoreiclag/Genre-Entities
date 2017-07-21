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


/**
 * @param num A 64 bit unsigned value
 * @return The lower 52 bits
 */
uint64_t bottom_52(uint64_t num) {
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
            {"__tostring", li_archetype_mt_tostring},
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
}

int li_archetype_mt_tostring(lua_State* l) {
    // The first argument is guaranteed to be the right type
    Runtime::Arche* arche = 
            *(static_cast<Runtime::Arche**>(lua_touserdata(l, 1)));
    std::stringstream sss;
    sss << "<Archetype @"
        << arche
        << ">";
    lua_pushstring(l, sss.str().c_str());
    return 1;
}

std::string to_string_entity(Runtime::Entity_Handle ent) {
    std::stringstream sss;
    sss << "<Entity #"
        << bottom_52(ent.get_id());
    if (ent.does_exist()) {
        sss << " arche @"
            << ent->get_arche();
    } else {
        sss << " (deleted)";
    }
    sss << ">";
    return sss.str();
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
    
    if (!ent.does_exist()) {
        luaL_error(l, "Entity %v does not exist!", 
                to_string_entity(ent).c_str());
    }
    
    const char* keystr = luaL_checkstring(l, 2);
    if (std::strcmp(keystr, "__id") == 0) {
        lua_pushnumber(l, entity_handle_to_lua_number(ent.get_id()));
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
    
    void* lua_mem = lua_newuserdata(l, sizeof(Runtime::Arche*));
    luaL_getmetatable(l, MTI_ARCHETYPE);
    lua_setmetatable(l, -2);
    Runtime::Arche*& lud_arche = *static_cast<Runtime::Arche**>(lua_mem);
    
    lud_arche = arche;
    
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
    
    void* lua_mem = lua_newuserdata(l, sizeof(Runtime::Entity_Handle));
    luaL_getmetatable(l, MTI_ENTITY);
    lua_setmetatable(l, -2);
    
    *(new (lua_mem) Runtime::Entity_Handle) = ent;
    
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
