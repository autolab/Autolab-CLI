#include "logger.h"

#include <iostream>

/* Logger-related */

namespace Logger {

  line_ending_symbol endl;
  fatal_logger fatal;
  info_logger info;
  debug_logger debug;

  template<>
  fatal_logger &fatal_logger::operator<<(line_ending_symbol end) {
    std::cout << std::endl;
    return *this;
  }

  template<>
  info_logger &info_logger::operator<<(line_ending_symbol end) {
    std::cout << std::endl;
    return *this;
  }

  template<>
  debug_logger &debug_logger::operator<<(line_ending_symbol end) {
  #ifdef PRINT_DEBUG
    std::cout << std::endl;
  #endif
    return *this;
  }

}