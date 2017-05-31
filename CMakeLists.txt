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

##### GENREENT CMAKE CONFIG #####

### Basic CMake project and build information ###

# CMake minimum version
# 3.7.2 is the first version that FindVulkan is supported
cmake_minimum_required(VERSION 3.7.2)

# Project name/version
project(GENREENT VERSION 0.1.0)
message(STATUS "Genre Entity System version ${GENREENT_VERSION}")

# Where to find modules
list(APPEND CMAKE_MODULE_PATH "${GENREENT_SOURCE_DIR}/cmake")

### Variables starting with "PGLOCAL_" are not CMake built-ins

# Where the .cpp files reside
set(PGLOCAL_SOURCE_DIR "${GENREENT_SOURCE_DIR}/src")

# Where the header files reside
set(PGLOCAL_INCLUDE_DIRS ${PGLOCAL_SOURCE_DIR})

# Name of target executable
set(PGLOCAL_ENGINE_TARGET "GenreEntitySystem")

### User options ###

### Build target configuration ###

# Load the sources list from a file as PGLOCAL_SOURCES_LIST
include("EngineSrcList")

set(PGLOCAL_ALL_REQUIRED_READY TRUE)
message(STATUS "Package status: ")

## Header-Only / Compiled Third-party ##

# JsonCpp #
message(STATUS "JsonCpp ==============")
find_package(JsonCpp)
if(JSONCPP_FOUND)
    message(STATUS "\tInclude Dirs: " ${JSONCPP_INCLUDE_DIR})
    message(STATUS "\tSource Files: " ${JSONCPP_SOURCE_FILES})
    list(APPEND PGLOCAL_INCLUDE_DIRS ${JSONCPP_INCLUDE_DIR})
    list(APPEND PGLOCAL_SOURCES_LIST ${JSONCPP_SOURCE_FILES})
else()
    message("\tNOT FOUND")
    set(PGLOCAL_ALL_REQUIRED_READY FALSE)
endif()

# Add a new build target
add_executable(${PGLOCAL_ENGINE_TARGET} ${PGLOCAL_SOURCES_LIST})

# Add required features
set_property(TARGET ${PGLOCAL_ENGINE_TARGET} PROPERTY CXX_STANDARD 11)

## Linked Third-party ##

# Lua #
message(STATUS "Lua 5.1 ==============")
find_package(LuaJIT)
if(LUAJIT_FOUND)
    message(STATUS "\tInclude Dirs: " ${LUAJIT_INCLUDE_DIR})
    message(STATUS "\tLibraries: " ${LUAJIT_LIBRARY})
    list(APPEND PGLOCAL_INCLUDE_DIRS ${LUAJIT_INCLUDE_DIR})
    target_link_libraries(${PGLOCAL_ENGINE_TARGET} ${LUAJIT_LIBRARY})
else()
    message("\tNOT FOUND")
    set(PGLOCAL_ALL_REQUIRED_READY FALSE)
endif()

# Helpful information
if(PGLOCAL_ALL_REQUIRED_READY)
    message(STATUS "All packages found and are compatible")
else()
    message("One or more packages are missing, build will fail!")
endif()

# Setup include directories
include_directories(${PGLOCAL_INCLUDE_DIRS})
