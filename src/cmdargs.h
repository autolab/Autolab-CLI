#ifndef AUTOLAB_CMDARGS_H_
#define AUTOLAB_CMDARGS_H_

#include <map>
#include <string>
#include <utility> // pair
#include <vector>

class cmdargs {
private:
  std::string cmd_name, help_text; // help texts
  std::vector<std::pair<std::string, bool>> arg_help; // help info for args
  std::map<std::string, std::string> opt_help; // help info for options

public:
  // matches option name/key to option value
  // if the option doesn't have a value, it will be an empty string
  typedef std::pair<std::string, std::string> opt_pair;

  std::vector<std::string> args; // parsed positional args
  std::vector<opt_pair> opts; // parsed options
  
  bool has_option(std::string name, std::string alt_name = "");
  std::string get_option(std::string name, std::string alt_name = "");

  void setup_help(std::string name, std::string help);
  void setup_done();
  void new_arg(std::string name, bool is_required);
  std::string new_option(std::string name, std::string alt_name,
    std::string description);
  bool new_flag_option(std::string name, std::string alt_name,
    std::string description);

  int nargs() { return args.size(); }

  void print_help();
};

bool parse_cmdargs(cmdargs &cmd, int argc, char *argv[]);

#endif /* AUTOLAB_CMDARGS_H_ */