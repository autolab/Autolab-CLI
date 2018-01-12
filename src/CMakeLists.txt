set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(autolab main.cpp file_utils.cpp config_manager.cpp autolab_client.cpp client_helpers.cpp logger.cpp)

find_library(CURL_LIB curl)
target_link_libraries(autolab ${CURL_LIB})

include_directories(.)