#include "cmdargs.h"

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

      cmd.opts.emplace_back(curr, "");
      // check if next arg is a value
      if (i < argc - 1) {
        next = argv[i+1];
        if (next[0] != '-') {
          cmd.opts.back().second = std::string(next);
          i++;
        }
      }
    } else {
      // non-opt arg
      if (args_done) return false;

      cmd.args.emplace_back(curr);
    }
  }

  return true;
}