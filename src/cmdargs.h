#ifndef AUTOLAB_CMDARGS_H_
#define AUTOLAB_CMDARGS_H_

#include <string>
#include <utility> // pair
#include <vector>

struct cmdargs {
  // matches option name/key to option value
  // if the option doesn't have a value, it will be an empty string
  typedef std::pair<std::string, std::string> opt_pair;

  std::vector<std::string> args; // positional arguments
  std::vector<opt_pair> opts; // options

  bool has_option(std::string name, std::string alt_name = "") {
    for (auto &opt : opts) {
      if (name == opt.first || alt_name == opt.first) return true;
    }
    return false;
  }

  std::string get_option(std::string name, std::string alt_name = "") {
    for (auto &opt : opts) {
      if (name == opt.first || alt_name == opt.first) return opt.second;
    }
    return std::string();
  }

  int nargs() {
    return args.size();
  }
};

bool parse_cmdargs(cmdargs &cmd, int argc, char *argv[]);

#endif /* AUTOLAB_CMDARGS_H_ */