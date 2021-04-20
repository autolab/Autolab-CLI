cmake_minimum_required(VERSION 2.8)

project(autolab-cli)

# The version number
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)
set(variant "" CACHE STRING "build variant")

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
message(STATUS "Build variant: ${variant}")

# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# configure a header file to pass the version number to the source code
configure_file(
  "${PROJECT_SOURCE_DIR}/src/build_config.h.in"
  "${PROJECT_BINARY_DIR}/build_config.h"
)

# setup external project dependencies
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

ExternalProject_Add(rapidjson
  PREFIX thirdparty
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG v1.1.0
  STEP_TARGETS download
  CONFIGURE_COMMAND true
  BUILD_COMMAND true
  INSTALL_COMMAND true
  TEST_COMMAND true
  EXCLUDE_FROM_ALL TRUE)
ExternalProject_Get_Property(rapidjson SOURCE_DIR)
set(RAPIDJSON_INCLUDE_DIR "${SOURCE_DIR}/include")

# go into subdirectories
add_subdirectory(src)
add_subdirectory(lib)
