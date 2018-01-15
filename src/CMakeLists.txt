if (NOT release)
  set(PRINT_DEBUG TRUE)
endif (NOT release)

# configure a header file to pass some the version number to the source code
configure_file(
  "${PROJECT_SOURCE_DIR}/src/build_config.h.in"
  "${PROJECT_BINARY_DIR}/build_config.h"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(autolab main.cpp file_utils.cpp context_manager.cpp autolab_client.cpp client_helpers.cpp logger.cpp cmdargs.cpp)

find_library(CURL_LIB curl)
target_link_libraries(autolab ${CURL_LIB})

include_directories(. "${PROJECT_BINARY_DIR}")

install (TARGETS autolab DESTINATION bin)