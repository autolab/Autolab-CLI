#include <string>
#include <vector>

#include "cmdargs.h"

/*
  The struct that will be stored as values in the info_map

  Contains:
    - A string to print in the usage statement
    - A helper function to be called when the subcommand is specified
    - A boolean that specifies if the command is intended for instructors only
*/
typedef struct command_info {
  std::string usage;
  int (* helper_fn) (cmdargs &cmd);
  bool instructor_command;
} command_info;

typedef std::map<std::string, command_info> command_info_map;
typedef std::map<std::string, std::string> command_alias_map;

/*
  An object that will keep track of all valid commands and information
  associated with them.

  Variables:
    - A dictionary of commands to command_info structs
    - A dictionary of command aliases to the default command names

  Functions:
    - exec_command: (string, cmdargs)s -> int
      - takes a subcommand and executes it
      - "executes it" by running the helper function specified in its
        respective command_info struct
*/
class CommandMap {
  public:
  command_info_map info_map;
  command_alias_map aliases;

  int exec_command(cmdargs &cmd, std::string command);
};

/*
  Takes no arguments and spits out the standard command map that the Autolab
  CLI uses for its operations.
*/
CommandMap init_autolab_command_map();
