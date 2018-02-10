add_library(autolab
  json_helpers.cpp client.cpp raw_client.cpp)

target_include_directories(autolab
  PUBLIC "${PROJECT_SOURCE_DIR}/include"
  PRIVATE .)

find_library(CURL_LIB curl)
target_link_libraries(autolab
  ${CURL_LIB} logger)