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

const char* MTI_COMPONENT = "pegr.Component";
const char* MTI_ARCHETYPE = "pegr.Archetype";
const char* MTI_GENRE = "pegr.Genre";
const char* MTI_ENTITY = "pegr.Entity";

int archetype_mt_tostring(lua_State* l) {
    lua_pushstring(l, "Archetype");
    return 1;
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
    Runtime::Arche** egg = static_cast<Runtime::Arche**>(lua_mem);
    
    
    *egg = arche;
    
    luaL_getmetatable(l, MTI_ARCHETYPE);
    lua_setmetatable(l, -2);
    
    return 1;
}

int new_entity(lua_State* l) {
    if (Gensys::get_global_state() != GlobalState::EXECUTABLE) {
        luaL_error(l, "new_entity is only available during execution");
    }
}

} // namespace LI
} // namespace Gensys
} // namespace pegr
