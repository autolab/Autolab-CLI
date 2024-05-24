/*
 * Includes classes and methods that can be used to write output.
 *
 * Three output strategies are included:
 *   - Logger::info
 *       Used for general output to stdout. All info intended for the user
 *       should go through it. This logger also supports colors.
 *   - Logger::fatal
 *       Used for reporting fatal errors to stderr. An optional prefix can be
 *       set so that the first time this logger is written to, the prefix is
 *       printed. The convention is to exit the program after writing to it.
 *   - LogDebug
 *       WARNING: Do not call Logger::debug directly. Always use the provided
 *       LogDebug macro. This will remove any debugging output in the release
 *       builds.
 *       Used for outputting debug info to stdout. If PRINT_DEBUG is not
 *       defined, any output written to this logger will be discarded. Should be
 *       used sparingly for reporting important information that may be useful
 *       while debugging.
 *
 * A Logger::endl is provided to write std::out to the output.
 */

#ifndef AUTOLAB_LOGGER_H_
#define AUTOLAB_LOGGER_H_

#include <iostream>
#include <string>

#include "build_config.h"

#ifdef PRINT_DEBUG
#define LogDebug(m) Logger::debug << m
#else
#define LogDebug(m)
#endif /* PRINT_DEBUG */

namespace Logger {

  struct line_ending_symbol {};

  struct color_symbol {
    int code;
  };
  
  struct fatal_logger {
    fatal_logger() : prefix_used(false) {}
    void set_prefix(std::string new_prefix) {
      prefix = std::string(new_prefix);
    }
    template<class T>
    fatal_logger &operator<<(T val) {
      if (!prefix_used) {
        prefix_used = true;
        std::cerr << "fatal: ";
        if (prefix.length() > 0) {
          std::cerr << prefix << std::endl;
        }
      }
      std::cerr << val;
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
    debug_logger &operator<<([[maybe_unused]] T val) {
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

  // colors
  extern color_symbol NONE;
  extern color_symbol RED;
  extern color_symbol GREEN;
  extern color_symbol YELLOW;
  extern color_symbol BLUE;
  extern color_symbol MAGENTA;
  extern color_symbol CYAN;
  
  // define specializations of operator<< for line_ending_symbols
  template<>
  fatal_logger &fatal_logger::operator<<(line_ending_symbol end);

  template<>
  info_logger &info_logger::operator<<(line_ending_symbol end);
  template<>
  info_logger &info_logger::operator<<(color_symbol color);

  template<>
  debug_logger &debug_logger::operator<<(line_ending_symbol end);
}

#endif /* AUTOLAB_LOGGER_H_ */