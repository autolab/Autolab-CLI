#include <string>
#include <vector>

#include "cmdargs.h"

/*
  The struct that will be stored as values in the info_map

  Contains:
    - A string to print in the usage statement
    - A helper function to be called when the subcommand is specified
*/
typedef struct command_info {
  std::string usage;
  int (* helper_fn) (cmdargs &cmd);
} command_info;

/*
  An object that will keep track of all valid commands and information
  associated with them.

  Variables:
    - A vector of all valid commands
    - A dictionary of commands to command_info structs

  Functions:
    - get_usage: string -> string
      - takes a command, and returns its usage string
    - exec_command: (string, cmdargs)s -> int
      - takes a subcommand and executes it
      - "executes it" by running the helper function specified in its
        respective command_info struct
    - add_command: (string, string, F: cmdargs -> int) -> int
*/
class CommandMap {
  public:
  std::vector<std::string> commands;
  std::map<std::string, command_info *> info_map;

  std::string get_usage(std::string command);
  int exec_command(cmdargs &cmd, std::string command);
};

/*
  Takes no arguments and spits out the standard command map that the Autolab
  CLI uses for its operations.
*/
CommandMap init_autolab_command_map();
