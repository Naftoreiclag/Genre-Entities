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
# - SDL2_FOUND
# - SDL2_INCLUDE_DIR
# - SDL2_LIBRARIES

set(SDL2_FOUND FALSE)
find_path(SDL2_INCLUDE_DIR NAMES "SDL.h")
find_library(SDL2_LIBRARY_SDL2MAIN NAMES "SDL2main")
find_library(SDL2_LIBRARY_SDL2 NAMES "SDL2")

if(SDL2_INCLUDE_DIR
        AND SDL2_LIBRARY_SDL2MAIN
        AND SDL2_LIBRARY_SDL2)
    set(SDL2_FOUND TRUE)
    list(APPEND SDL2_LIBRARIES ${SDL2_LIBRARY_SDL2MAIN})
    list(APPEND SDL2_LIBRARIES ${SDL2_LIBRARY_SDL2})
endif()

mark_as_advanced(
    SDL2_INCLUDE_DIR
    SDL2_LIBRARY_SDL2MAIN
    SDL2_LIBRARY_SDL2
)
