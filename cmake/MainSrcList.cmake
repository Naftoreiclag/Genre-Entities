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

# This file contains a listing of all of the source files used in the engine
# build target. Populates a list called PGLOCAL_SOURCES_LIST

# Preferred method of adding source items is through the Python script in:
# `util/GenerateEngineSrcList.py`

# This function appends the provided string list to PGLOCAL_SOURCES_LIST
set(PGLOCAL_SOURCES_LIST "")
foreach(fname 

"debug/DebugAssertLuaBalance.cpp"
"gensys/Compiler.cpp"
"gensys/Gensys.cpp"
"gensys/Interm_Types.cpp"
"gensys/Lua_Interf_Runtime.cpp"
"gensys/Lua_Interf_Setup.cpp"
"gensys/Pod_Chunk.cpp"
"gensys/Runtime_Types.cpp"
"gensys/Util.cpp"
"logger/Logger.cpp"
"main.cpp"
"scheduler/SchedLuaInterface.cpp"
"script/Script.cpp"
"script/ScriptHelper.cpp"

)
list(APPEND PGLOCAL_SOURCES_LIST "${PGLOCAL_SOURCE_DIR}/pegr/${fname}")
endforeach()
