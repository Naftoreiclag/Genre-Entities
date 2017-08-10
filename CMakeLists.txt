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

### Basic CMake project and build information ###

set(PGLOCAL_HUMAN_NAME "Naftoreiclag's Genre Entity System")
set(PGLOCAL_PROJ_NAME "pegr")

# CMake minimum version
# 3.7.2 is the first version that FindVulkan is supported
cmake_minimum_required(VERSION 3.7.2)

# Project name/version
project(GENREENT VERSION 0.1.0)
message(STATUS "${PGLOCAL_HUMAN_NAME} version ${GENREENT_VERSION}")

# Where to find modules
list(APPEND CMAKE_MODULE_PATH "${GENREENT_SOURCE_DIR}/cmake")

### Variables starting with "PGLOCAL_" are not CMake built-ins

# Where the .cpp files reside
set(PGLOCAL_SOURCE_DIR "${GENREENT_SOURCE_DIR}/src")

# Where the header files reside
set(PGLOCAL_INCLUDE_DIRS ${PGLOCAL_SOURCE_DIR})

# Name of target executable
set(PGLOCAL_MAIN_TARGET "${PGLOCAL_PROJ_NAME}")
set(PGLOCAL_TEST_TARGET "${PGLOCAL_PROJ_NAME}Test")

### User options ###

### Build target configuration ###

include("MainSrcList")
set(PGLOCAL_MAIN_SOURCES_LIST ${PGLOCAL_SOURCES_LIST})
include("TestSrcList")
set(PGLOCAL_TEST_SOURCES_LIST ${PGLOCAL_SOURCES_LIST})
set(PGLOCAL_SOURCES_LIST "")
include("IncludesList")
list(APPEND PGLOCAL_INCLUDE_DIRS ${PGLOCAL_INCLUDES_LIST})
set(PGLOCAL_INCLUDES_LIST "")

set(PGLOCAL_ALL_REQUIRED_READY TRUE)
message(STATUS "Package status: ")

# Add build targets
add_executable(${PGLOCAL_MAIN_TARGET} ${PGLOCAL_MAIN_SOURCES_LIST})
add_executable(${PGLOCAL_TEST_TARGET} ${PGLOCAL_TEST_SOURCES_LIST})

# Add required features
set_property(TARGET ${PGLOCAL_MAIN_TARGET} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PGLOCAL_TEST_TARGET} PROPERTY CXX_STANDARD 14)

## Linked Third-party ##

# Lua #
message(STATUS "Lua 5.1 ==============")
find_package(LuaJIT)
if(LUAJIT_FOUND)
    message(STATUS "\tInclude Dirs: " ${LUAJIT_INCLUDE_DIRS})
    message(STATUS "\tLibraries: " ${LUAJIT_LIBRARIES})
    list(APPEND PGLOCAL_INCLUDE_DIRS ${LUAJIT_INCLUDE_DIRS})
    target_link_libraries(${PGLOCAL_MAIN_TARGET} ${LUAJIT_LIBRARIES})
    target_link_libraries(${PGLOCAL_TEST_TARGET} ${LUAJIT_LIBRARIES})
else()
    message("\tNOT FOUND")
    set(PGLOCAL_ALL_REQUIRED_READY FALSE)
endif()

# SDL2 #
message(STATUS "SDL2 =================")
find_package(SDL2)
if(SDL2_FOUND)
    message(STATUS "\tInclude Dirs: " ${SDL2_INCLUDE_DIRS})
    message(STATUS "\tLibraries: " ${SDL2_LIBRARIES})
    list(APPEND PGLOCAL_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PGLOCAL_MAIN_TARGET} ${SDL2_LIBRARIES})
    target_link_libraries(${PGLOCAL_TEST_TARGET} ${SDL2_LIBRARIES})
else()
    message("\tNOT FOUND")
    set(PGLOCAL_ALL_REQUIRED_READY FALSE)
endif()

# bgfx #
message(STATUS "bgfx =================")
find_package(bgfx)
if(BGFX_FOUND)
    message(STATUS "\tInclude Dirs: " ${BGFX_INCLUDE_DIRS})
    message(STATUS "\tLibraries: " ${BGFX_LIBRARIES})
    list(APPEND PGLOCAL_INCLUDE_DIRS ${BGFX_INCLUDE_DIRS})
    target_link_libraries(${PGLOCAL_MAIN_TARGET} ${BGFX_LIBRARIES})
    target_link_libraries(${PGLOCAL_TEST_TARGET} ${BGFX_LIBRARIES})
else()
    message("\tNOT FOUND")
    set(PGLOCAL_ALL_REQUIRED_READY FALSE)
endif()

# Boost #
message(STATUS "Boost Filesystem =====")
find_package(Boost 1.46.0 COMPONENTS system filesystem)
if(Boost_FOUND)
    message(STATUS "\tInclude Dirs: " ${Boost_INCLUDE_DIRS})
    message(STATUS "\tLibraries: " ${Boost_LIBRARIES})
    list(APPEND PGLOCAL_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    target_link_libraries(${PGLOCAL_MAIN_TARGET} ${Boost_LIBRARIES})
    target_link_libraries(${PGLOCAL_TEST_TARGET} ${Boost_LIBRARIES})
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
