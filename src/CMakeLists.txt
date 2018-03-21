add_executable(autolab-client
  main.cpp file_utils.cpp context_manager.cpp cmdargs.cpp pretty_print.cpp
  pseudocrypto.cpp cmdmap.cpp)
set_target_properties(autolab-client PROPERTIES OUTPUT_NAME autolab)

target_include_directories(autolab-client
  PRIVATE . "${PROJECT_BINARY_DIR}")

target_link_libraries(autolab-client
  autolab logger crypto)

install (TARGETS autolab-client DESTINATION bin)
