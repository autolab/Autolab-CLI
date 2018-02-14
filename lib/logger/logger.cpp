#include "logger.h"

#include <iostream>

/* Logger-related */

namespace Logger {

  line_ending_symbol endl;
  fatal_logger fatal;
  info_logger info;
  debug_logger debug;

  // colors
  color_symbol NONE    = {0};
  color_symbol RED     = {91};
  color_symbol GREEN   = {92};
  color_symbol YELLOW  = {93};
  color_symbol BLUE    = {94};
  color_symbol MAGENTA = {95};
  color_symbol CYAN    = {96};

  template<>
  fatal_logger &fatal_logger::operator<<(line_ending_symbol) {
    std::cerr << std::endl;
    return *this;
  }

  template<>
  info_logger &info_logger::operator<<(line_ending_symbol) {
    std::cout << std::endl;
    return *this;
  }
  template<>
  info_logger &info_logger::operator<<(color_symbol color) {
    std::cout << "\x1b[" << color.code << "m";
    return *this;
  }

  template<>
  debug_logger &debug_logger::operator<<(line_ending_symbol) {
  #ifdef PRINT_DEBUG
    std::cout << std::endl;
  #endif
    return *this;
  }

}