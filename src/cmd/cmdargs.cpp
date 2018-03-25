#include "cmdargs.h"

#include <cstdlib> // exit

#include <iomanip>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <utility> // pair
#include <vector>

#include "logger.h"
#include "../pretty_print/pretty_print.h"

// error output
void error_pos_after_opt(std::string error_arg) {
  Logger::fatal << "Invalid command line argument '" << error_arg << "'." << Logger::endl
      << "Note that all options must come after all positional arguments (e.g. commands)."
      << Logger::endl << "For detailed usage, run with '-h'." << Logger::endl;
  exit(-1);
}

void error_opt_missing_arg(std::string opt_name) {
  Logger::fatal << "Required parameter for '" << opt_name << "' missing." << Logger::endl
      << "For detailed usage, run with '-h'." << Logger::endl;
  exit(-1);
}

// helpers
std::string combine_opt_names(std::string name, std::string alt_name,
    std::string arg_name = "") {
  std::string combined = name;
  if (alt_name.length() > 0) {
    combined += "," + alt_name;
  }
  if (arg_name.length() > 0) {
    combined += " <" + arg_name + ">";
  }
  return combined;
}

// interface
bool cmdargs::has_option(std::string name, std::string alt_name) {
  std::string arg;
  return get_option(arg, name, alt_name);
}

bool cmdargs::get_option(std::string &arg, std::string name, std::string alt_name) {
  for (auto &opt : opts) {
    if (name == opt.first || alt_name == opt.first) {
      arg =  opt.second;
      return true;
    }
  }
  return false;
}

void cmdargs::setup_help(std::string name, std::string help) {
  cmd_name = name;
  help_text = help;
  // every command comes with a default help option
  // can be overriden using new_option or new_flag_option if needed
  new_flag_option("-h","--help","Show this help message");
}

void cmdargs::setup_done() {
  // count required args
  int required_args = count_words(cmd_name);
  for (auto &arg : arg_help) {
    if (arg.second) required_args++;
  }

  // check for help
  if (has_option("-h","--help") || required_args > nargs()) {
    print_help();
    exit(0);
  }
}

// set a new positional argument
// should be called in order of the positional arguments
void cmdargs::new_arg(std::string name, bool is_required) {
  arg_help.push_back(std::make_pair(name, is_required));
}

// set a new option that requires an argument
std::string cmdargs::new_option(std::string name, std::string alt_name,
    std::string arg_name, std::string description) {
  opt_help[combine_opt_names(name, alt_name, arg_name)] = description;
  std::string argument;
  bool exists = get_option(argument, name, alt_name);
  if (exists && argument == "") {
    error_opt_missing_arg(name); // exits
  }
  return argument;
}

// set a new option that does not require an argument
bool cmdargs::new_flag_option(std::string name, std::string alt_name,
    std::string description) {
  opt_help[combine_opt_names(name, alt_name)] = description;
  std::string argument;
  bool exists = get_option(argument, name, alt_name);
  if (exists && argument != "") {
    error_pos_after_opt(argument); // exits
  }
  return exists;
}

void cmdargs::print_help() {
  bool has_options = opt_help.size() > 0;
  bool has_help_text = help_text.length() > 0;
  // print usage
  Logger::info << "usage: ";
  std::ostringstream usage;

  usage << cmd_name << " ";
  for (auto &arg : arg_help) {
    bool is_required = arg.second;
    if (!is_required) usage << "[";
    usage << "<" << arg.first << ">";
    if (!is_required) usage << "]";
    usage << " ";
  }
  if (has_options) usage << "[OPTIONS]";
  Logger::info << wrap_text_with_indent(7 /* length of "usage: " */, usage.str());

  if (has_options) {
    Logger::info << Logger::endl << "options:" << Logger::endl;

    int longest_key = 0;
    for (auto it = opt_help.begin(); it != opt_help.end(); it++) {
      int curr_length = it->first.length();
      if (curr_length > longest_key) {
        longest_key = curr_length;
      }
    }

    longest_key += 2; // add 2 spaces

    for (auto it = opt_help.begin(); it != opt_help.end(); it++) {
      Logger::info << "  " << std::setw(longest_key) << std::left << it->first
        << wrap_text_with_indent(2 + longest_key, it->second);
    }

    if (has_help_text) {
      Logger::info << Logger::endl;
    }
  }

  if (has_help_text) {
    Logger::info << wrap_text_with_indent(0, help_text);
  }
}

/* parse command line args into a cmdargs struct.
 *
 * positional args must all come before options, otherwise returns false
 */
bool parse_cmdargs(cmdargs &cmd, int argc, char *argv[]) {
  bool args_done = false;
  char *curr = nullptr;
  char *next = nullptr;

  for (int i = 0; i < argc; i++) {
    curr = argv[i];

    if (curr[0] == '-') {
      args_done = true;

      if (curr[1] != '-' && curr[2] != '\0') {
        // this is a combination of short options, separate them
        for (char *this_opt = curr + 1; *this_opt != '\0'; this_opt++) {
          cmd.opts.emplace_back(std::string("-") + *this_opt, "");
        }
      } else {
        // this is a single short option or a single long option,
        // try looking for an argument
        cmd.opts.emplace_back(curr, "");
        // check if next arg is a value
        if (i < argc - 1) {
          next = argv[i+1];
          if (next[0] != '-') {
            cmd.opts.back().second = std::string(next);
            i++;
          }
        }
      }
    } else {
      // non-opt arg
      if (args_done) error_pos_after_opt(curr); // exits

      cmd.args.emplace_back(curr);
    }
  }

  return true;
}
