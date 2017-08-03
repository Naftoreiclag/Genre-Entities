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
# - BGFX_FOUND
# - BGFX_INCLUDE_DIR
# - BGFX_LIBRARIES

set(BGFX_FOUND FALSE)
find_path(BGFX_INCLUDE_DIR NAMES "bgfx/bgfx.h")
find_library(BGFX_LIBRARY_BGFX NAMES "bgfx")
find_library(BGFX_LIBRARY_BIMG NAMES "bimg")
find_library(BGFX_LIBRARY_BX NAMES "bx")


if(BGFX_INCLUDE_DIR 
        AND BGFX_LIBRARY_BX 
        AND BGFX_LIBRARY_BIMG 
        AND BGFX_LIBRARY_BGFX)
    set(BGFX_FOUND TRUE)
    list(APPEND BGFX_LIBRARIES ${BGFX_LIBRARY_BGFX})
    list(APPEND BGFX_LIBRARIES ${BGFX_LIBRARY_BIMG})
    list(APPEND BGFX_LIBRARIES ${BGFX_LIBRARY_BX})
    if(WIN32)
        find_library(PSAPI Psapi)
        list(APPEND BGFX_LIBRARIES ${PSAPI})
    endif()
endif()

mark_as_advanced(
    BGFX_INCLUDE_DIR
    BGFX_LIBRARY_BGFX
    BGFX_LIBRARY_BIMG
    BGFX_LIBRARY_BX
)

