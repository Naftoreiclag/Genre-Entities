#   Copyright 2017 James Fong
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

# This file contains a listing of all of the source files used in the
# build target. Populates a list called PGLOCAL_SOURCES_LIST

# Preferred method of adding source items is through the Python script in:
# `util/Generate*SrcList.py`

# This function appends the provided string list to PGLOCAL_SOURCES_LIST
set(PGLOCAL_SOURCES_LIST "")
foreach(fname 

"../thirdparty/bgfx_examples/bexam_cubes/cubes.cpp"
"../thirdparty/easyloggingpp/easylogging++.cc"
"../thirdparty/jsoncpp/jsoncpp.cpp"
"Main.cpp"
"app/Game.cpp"
"debug/Debug_Assert_Lua_Balance.cpp"
"engine/App_State.cpp"
"engine/App_State_Machine.cpp"
"engine/Engine.cpp"
"except/Except.cpp"
"gensys/Compiler.cpp"
"gensys/Entity_Collection.cpp"
"gensys/Entity_Handle.cpp"
"gensys/Entity_Tick_Event.cpp"
"gensys/Gensys.cpp"
"gensys/Interm_Types.cpp"
"gensys/Lua_Interf_Runtime.cpp"
"gensys/Lua_Interf_Setup.cpp"
"gensys/Pod_Chunk.cpp"
"gensys/Runtime.cpp"
"gensys/Util.cpp"
"logger/Logger.cpp"
"render/Shaders.cpp"
"resource/Json_Util.cpp"
"resource/Oid.cpp"
"resource/Resources.cpp"
"scheduler/Lua_Interf.cpp"
"scheduler/Sched.cpp"
"script/Lua_Interf_Util.cpp"
"script/Script.cpp"
"script/Script_Resource.cpp"
"script/Script_Util.cpp"
"text/Text.cpp"
"winput/Enum_Utils.cpp"
"winput/Winput.cpp"

)
list(APPEND PGLOCAL_SOURCES_LIST 
        "${PGLOCAL_SOURCE_DIR}/${PGLOCAL_PROJ_NAME}/${fname}")
endforeach()
