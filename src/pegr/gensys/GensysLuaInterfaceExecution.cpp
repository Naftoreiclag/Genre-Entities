#include "pegr/gensys/GensysLuaInterface.hpp"

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
#include "pegr/gensys/Gensys.hpp"
#include "pegr/script/ScriptHelper.hpp"
#include "pegr/gensys/GensysRuntime.hpp"

namespace pegr {
namespace Gensys {
namespace LI {

// MTI = metatable id
const char* MTI_COMPONENT = "pegr.Component";
const char* MTI_ARCHETYPE = "pegr.Archetype";
const char* MTI_GENRE = "pegr.Genre";
const char* MTI_AVIEW = "pegr.Archetypical_Entity";


Runtime::Arche** argcheck_archetype(lua_State* l, int idx) {
    void* lua_mem = luaL_checkudata(l, 1, MTI_ARCHETYPE);
    return static_cast<Runtime::Arche**>(lua_mem);
}
Runtime::Aview* argcheck_aview(lua_State* l, int idx) {
    void* lua_mem = luaL_checkudata(l, 1, MTI_AVIEW);
    return static_cast<Runtime::Aview*>(lua_mem);
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
            {"__tostring", archetype_mt_tostring},
            // No __gc, since this userdata is POD pointer
            
            // End of the list
            {nullptr, nullptr}
        };
        luaL_register(l, nullptr, metatable);
    }
    popg.pop(1);

    success = luaL_newmetatable(l, MTI_AVIEW);
    popg.on_push(1);
    assert(success && "Archetypical Entity metatable id already taken!");
    {
        const luaL_Reg metatable[] = {
            {"__tostring", aview_mt_tostring},
            {"__gc", aview_mt_gc},
            
            // End of the list
            {nullptr, nullptr}
        };
        luaL_register(l, nullptr, metatable);
    }
    popg.pop(1);
}

int archetype_mt_tostring(lua_State* l) {
    // The first argument is guaranteed to be the right type
    Runtime::Arche* arche = 
            *(static_cast<Runtime::Arche**>(lua_touserdata(l, 1)));
    std::stringstream sss;
    sss << "Archetype: @"
        << arche;
    lua_pushstring(l, sss.str().c_str());
    return 1;
}

int aview_mt_tostring(lua_State* l) {
    Runtime::Aview& aview = 
            *(static_cast<Runtime::Aview*>(lua_touserdata(l, 1)));
    std::stringstream sss;
    sss << "Entity: @"
        << aview.m_entity.m_chunk
        << " through archetype @"
        << aview.m_entity.m_archetype;
    lua_pushstring(l, sss.str().c_str());
    return 1;
}

int aview_mt_gc(lua_State* l) {
    Runtime::Aview& av = 
            *(static_cast<Runtime::Aview*>(lua_touserdata(l, 1)));
    av.Runtime::Aview::~Aview();
    return 0;
}

int find_archetype(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::EXECUTABLE) {
        luaL_error(l, "find_archetype is only available during execution");
    }
    std::size_t strlen;
    const char* strdata = luaL_checklstring(l, 1, &strlen);
    std::string key(strdata, strlen);
    
    Runtime::Arche* arche = Gensys::find_archetype(key);
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

int new_entity(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::EXECUTABLE) {
        luaL_error(l, "new_entity is only available during execution");
    }
    
    Runtime::Arche* arche = *argcheck_archetype(l, 1);
    
    Runtime::Entity_Ptr ent = Runtime::new_entity(arche);
    assert(ent.can_be_spawned());
    
    void* lua_mem = lua_newuserdata(l, sizeof(Runtime::Aview));
    luaL_getmetatable(l, MTI_AVIEW);
    lua_setmetatable(l, -2);
    Runtime::Aview& lud_aview = *(new (lua_mem) Runtime::Aview);
    
    lud_aview.m_entity = ent;
    Runtime::grab_entity(ent);
    
    return 1;
}

} // namespace LI
} // namespace Gensys
} // namespace pegr
