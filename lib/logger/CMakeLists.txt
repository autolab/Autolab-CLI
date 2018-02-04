add_library(logger
  logger.cpp)

target_include_directories(logger
  PUBLIC . "${PROJECT_BINARY_DIR}")