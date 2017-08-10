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

# Populates:
# - LUAJIT_FOUND
# - LUAJIT_INCLUDE_DIRS
# - LUAJIT_LIBRARIES

set(LUAJIT_FOUND FALSE)
find_path(LUAJIT_INCLUDE_DIRS NAMES "lua.h" "luaxlib.h" "lualib.h" "luajit.h")
find_library(LUAJIT_LIBRARIES NAMES "luaJIT")

if(LUAJIT_INCLUDE_DIRS)
    set(LUAJIT_FOUND TRUE)
endif()

mark_as_advanced(
    LUAJIT_INCLUDE_DIRS
)
