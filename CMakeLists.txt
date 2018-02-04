cmake_minimum_required(VERSION 2.8)

project(autolab-cli)

# command line options
option(release "build release version (no debug output)" OFF)

# The version number
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)

# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src)
add_subdirectory(lib)