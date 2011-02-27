cmake_minimum_required(VERSION 2.6)
project(Msync)
set(EXECUTABLE_OUTPUT_PATH ${Msync_SOURCE_DIR}/../bin)
set(LIBRARY_OUTPUT_PATH ${Msync_SOURCE_DIR}/../bin)
set(Msync_INCLUDE_DIR ${Msync_SOURCE_DIR}/../include)

if (WIN32 AND NOT UNIX)
add_definitions(/DWINDOWS)
endif()

if(UNIX)
add_definitions(-DUNIX)
endif()

add_subdirectory(../src ../build/temp)
