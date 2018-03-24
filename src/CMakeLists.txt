add_executable(autolab-client
  main.cpp file/file_utils.cpp context_manager/context_manager.cpp
  cmd/cmdargs.cpp pretty_print/pretty_print.cpp
  crypto/pseudocrypto.cpp cmd/cmdmap.cpp cmd/cmdimp.cpp)
set_target_properties(autolab-client PROPERTIES OUTPUT_NAME autolab)

target_include_directories(autolab-client
  PRIVATE . "${PROJECT_BINARY_DIR}")

target_link_libraries(autolab-client
  autolab logger crypto)

install (TARGETS autolab-client DESTINATION bin)
