#ifndef CMDARGS_H
#define CMDARGS_H

#include <vector>
#include <string>

struct cmdargs {
  std::vector<std::string> args; // positional arguments
  std::vector<std::string> opts; // options

  bool has_option(std::string name, std::string alt_name = "") {
    for (auto &opt : opts) {
      if (name == opt || alt_name == opt) return true;
    }
    return false;
  }
  int nargs() {
    return args.size();
  }
};

bool parse_cmdargs(cmdargs &cmd, int argc, char *argv[]);

#endif /* CMDARGS_H */