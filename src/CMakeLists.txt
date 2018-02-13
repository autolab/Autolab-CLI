if (NOT release)
  set(PRINT_DEBUG TRUE)
endif (NOT release)

# configure a header file to pass some the version number to the source code
configure_file(
  "${PROJECT_SOURCE_DIR}/src/build_config.h.in"
  "${PROJECT_BINARY_DIR}/build_config.h"
)

add_executable(autolab-client 
  main.cpp file_utils.cpp context_manager.cpp cmdargs.cpp pretty_print.cpp)
set_target_properties(autolab-client PROPERTIES OUTPUT_NAME autolab)

target_include_directories(autolab-client
  PRIVATE . "${PROJECT_BINARY_DIR}")

target_link_libraries(autolab-client
  autolab logger)

install (TARGETS autolab-client DESTINATION bin)