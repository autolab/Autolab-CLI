cmake_minimum_required(VERSION 2.8)

project(autolab-cli)

# The version number
set(VERSION_MAJOR 0)
set(VERSION_MINOR 5)

# command line options
option(release "build release version (no debug output)" OFF)

if(NOT release)
  # build debug
  set(CMAKE_BUILD_TYPE Debug)
  set(PRINT_DEBUG TRUE)
else(NOT release)
  # build release
  set(CMAKE_BUILD_TYPE Release)
  set(PRINT_DEBUG FALSE)
endif(NOT release)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# configure a header file to pass some the version number to the source code
configure_file(
  "${PROJECT_SOURCE_DIR}/src/build_config.h.in"
  "${PROJECT_BINARY_DIR}/build_config.h"
)

add_subdirectory(src)
add_subdirectory(lib)