add_library(autolab
  json_helpers.cpp utility.cpp client.cpp raw_client.cpp)

add_dependencies(autolab rapidjson-download)

target_include_directories(autolab
  PUBLIC "${PROJECT_SOURCE_DIR}/include" ${RAPIDJSON_INCLUDE_DIR}
  PRIVATE .)

find_library(CURL_LIB curl)
target_link_libraries(autolab
  ${CURL_LIB} logger)
