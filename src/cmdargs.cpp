#include "cmdargs.h"

/* parse command line args into a cmdargs struct.
 * 
 * positional args must all come before options, otherwise returns false
 */
bool parse_cmdargs(cmdargs &cmd, int argc, char *argv[]) {
  bool args_done = false;

  for (int i = 0; i < argc; i++) {
    char *curr = argv[i];

    if (curr[0] == '-') {
      args_done = true;
      cmd.opts.emplace_back(curr);
    } else {
      // non-opt arg
      if (args_done) return false;
      cmd.args.emplace_back(curr);
    }
  }

  return true;
}