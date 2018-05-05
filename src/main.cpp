#include <iomanip>
#include <string>

#include "autolab/autolab.h"
#include "autolab/client.h"
#include "logger.h"

#include "app_credentials.h"
#include "build_config.h"
#include "cmd/cmdargs.h"
#include "cmd/cmdimp.h"
#include "cmd/cmdmap.h"

extern Autolab::Client client;

CommandMap command_map;

/* help texts */
void print_help() {
  Logger::info << "usage: autolab [OPTIONS] <command> [command-args] [command-opts]" << Logger::endl
    << Logger::endl
    << "general commands:" << Logger::endl;

  // First we print the general-use commands
  for (auto const &i : command_map.info_map) {
    command_info ci = i.second;
    if(ci.instructor_command == false) {
      Logger::info << ci.usage << Logger::endl;
    }
  }

  // Then we print the instructor-enabled commands
  Logger::info << Logger::endl
    << "instructor commands:" << Logger::endl;

  for (auto const &i : command_map.info_map) {
    command_info ci = i.second;
    if(ci.instructor_command == true) {
      Logger::info << ci.usage << Logger::endl;
    }
  }

  // Then we print general help
  Logger::info << Logger::endl
    << "options:" << Logger::endl
    << "  -h,--help      Show this help message" << Logger::endl
    << "  -v,--version   Show the version number of this build" << Logger::endl
    << Logger::endl
    << "run 'autolab <command> -h' to view usage instructions for each command." << Logger::endl;
}

void print_version() {
  Logger::info << "autolab-cli " << VERSION_MAJOR
    << "." << VERSION_MINOR
    << "." << VERSION_PATCH;
  if (BUILD_VARIANT.length() > 0) {
    Logger::info << " (" << BUILD_VARIANT << ")";
  }
  Logger::info << Logger::endl
    << "Target server: " << server_domain << Logger::endl;
}

/* must manually init client */
int user_setup(cmdargs &cmd) {
  cmd.setup_help("autolab setup",
      "Initiate user setup for the current user.");
  bool option_force = cmd.new_flag_option("-f", "--force",
      "Force user setup, removing the current user");
  cmd.setup_done();

  if (!option_force) {
    bool user_exists = init_autolab_client();

    if (user_exists) {
      // perform a check if not a forced setup
      bool token_valid = true;
      Autolab::User user_info;
      try {
        client.get_user_info(user_info);
      } catch (Autolab::InvalidTokenException &e) {
        token_valid = false;
      }
      if (token_valid) {
        Logger::info << "User '" << user_info.first_name
          << "' is currently set up on this client." << Logger::endl
          << "To force reset of user info, use the '-f' option." << Logger::endl;
        return 0;
      }
    }
  }

  // Request that user will always comply with academic integrity standards
  Logger::info << Logger::endl << "I affirm that, by using this product, I have "
    "complied and always will comply with my courses' academic integrity policies "
    "as defined by the respective syllabi [Y/n]." << Logger::endl;

  char response = getchar();

  if(response != 'Y' && response != 'y') {
    Logger::info << Logger::endl << "User setup failed -- user must agree to "
    "comply with academic integrity policy." << Logger::endl;
    return -1;
  }
  // Success, user has agreed to comply

  // user non-existant, or existing user's credentials no longer work, or forced
  int result = perform_device_flow(client);
  if (result == 0) {
    Logger::info << Logger::endl << "User setup complete." << Logger::endl;
    return 0;
  }
  Logger::info << Logger::endl << "User setup failed." << Logger::endl;
  return -1;
}

int main(int argc, char *argv[]) {
  command_map = init_autolab_command_map();

  cmdargs cmd;
  if (!parse_cmdargs(cmd, argc, argv)) {
    Logger::fatal << "Invalid command line arguments." << Logger::endl
      << "For detailed usage, run with '-h'." << Logger::endl;
    return 0;
  }

  if (cmd.nargs() == 1) {
    // not a command
    if (cmd.has_option("-v", "--version")) {
      print_version();
      return 0;
    }

    print_help();
    return 0;
  }

  // determine what command it is
  std::string command(argv[1]);

  try {
    if ("setup" == command) {
      return user_setup(cmd);
    } else {
      if (!init_autolab_client()) {
        Logger::fatal << "No user set up on this client yet." << Logger::endl
          << Logger::endl
          << "Please run 'autolab setup' to setup your Autolab account." << Logger::endl;
        return 0;
      }

      try {
        command_map.exec_command(cmd, command);
      } catch (Autolab::InvalidTokenException &e) {
        Logger::fatal << "Authorization invalid or expired." << Logger::endl
          << Logger::endl
          << "Please re-authorize this client by running 'autolab-setup'" << Logger::endl;
        return 0;
      }
    }
  } catch (Autolab::HttpException &e) {
    Logger::fatal << e.what() << Logger::endl;
    return -1;
  } catch (Autolab::InvalidResponseException &e) {
    Logger::fatal << Logger::endl
      << "Received invalid response from API server: " << Logger::endl
      << e.what() << Logger::endl;
    return 0;
  } catch (Autolab::ErrorResponseException &e) {
    Logger::fatal << e.what() << Logger::endl;
    return 0;
  }

  return 0;
}
