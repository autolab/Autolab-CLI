#ifndef AUTOLAB_LOGGER_H_
#define AUTOLAB_LOGGER_H_

#include <iostream>
#include <string>

#include "build_config.h"

void err_assert(bool value);
void err_assert(bool value, const char *msg);

namespace Logger {

  class line_ending_symbol {};
  
  struct fatal_logger {
    fatal_logger() : prefix_used(false) {}
    void set_prefix(std::string new_prefix) {
      prefix = std::string(new_prefix);
    }
    template<class T>
    fatal_logger &operator<<(T val) {
      if (!prefix_used) {
        prefix_used = true;
        std::cout << "fatal: " << prefix << std::endl;
      }
      std::cout << val;
      return *this;
    }
  private:
    std::string prefix;
    bool prefix_used;
  };
  struct info_logger {
    template<class T>
    info_logger &operator<<(T val) {
      std::cout << val;
      return *this;
    }
  };
  struct debug_logger {
    template<class T>
    debug_logger &operator<<(T val) {
    #ifdef PRINT_DEBUG
      std::cout << val;
    #endif
      return *this;
    }
  };

  extern line_ending_symbol endl;
  extern fatal_logger fatal;
  extern info_logger info;
  extern debug_logger debug;
  
  // define specializations of operator<< for line_ending_symbols
  template<>
  fatal_logger &fatal_logger::operator<<(line_ending_symbol end);

  template<>
  info_logger &info_logger::operator<<(line_ending_symbol end);

  template<>
  debug_logger &debug_logger::operator<<(line_ending_symbol end);
}

#endif /* AUTOLAB_LOGGER_H_ */