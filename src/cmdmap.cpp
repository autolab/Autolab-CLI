#include "cmdmap.h"
#include "logger.h"

std::string CommandMap::get_usage(std::string command) {
  // Check if command exists?
  try {
    return info_map[command].usage;
  }
  catch(int e) {
    Logger::info << "get_usage(" << command << ") throws error" << e;
    exit(1);
  }
}

int CommandMap::exec_command(cmdargs &cmd, std::string command) {
  try {
    return info_map[command].helper_fn(cmd);
  }
  catch(int e) {
    Logger::info << "exec_command(" << command << ") throws error" << e;
    exit(1);
  }
}

int CommandMap::add_command(std::string cmd_name, std::string cmd_usage, int (* helper_fn)(cmdargs cmd)) {
  // Add the command to the vector of commands
  commands.push_back(cmd_name);

  // Construct the command_info struct for this command
  command_info cmd_info;
  cmd_info.usage = cmd_usage;
  cmd_info.helper_fn = helper_fn;
  info_map[cmd_name] = cmd_info;

  // Why am I returning an int again?
  return 0;
}
