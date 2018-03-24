#include <cmath>
#include <ctime>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread> // sleep_for
#include <vector>

#include "autolab/autolab.h"
#include "autolab/client.h"
#include "logger.h"

#include "app_credentials.h"
#include "build_config.h"
#include "cmd/cmdargs.h"
#include "context_manager/context_manager.h"
#include "file/file_utils.h"
#include "pretty_print/pretty_print.h"
#include "cmd/cmdmap.h"
#include "cmd/cmdimp.h"

extern Autolab::Client client;

/* help texts */
void print_help() {
  Logger::info << "usage: autolab [OPTIONS] <command> [command-args] [command-opts]" << Logger::endl
    << Logger::endl
    << "general commands:" << Logger::endl
    << "  courses             List all courses" << Logger::endl
    << "  assessments/asmts   List all assessments of a course" << Logger::endl
    << "  status              Show status of the local assessment" << Logger::endl
    << "  problems            List all problems in an assessment" << Logger::endl
    << "  download            Download files needed for an assessment" << Logger::endl
    << "  submit              Submit a file to an assessment" << Logger::endl
    << "  scores              Show scores got on an assessment" << Logger::endl
    << "  setup               Setup the user of the client" << Logger::endl
    << Logger::endl
    << "instructor commands:" << Logger::endl
    << "  enroll              Manage users affiliated with a course" << Logger::endl
    << Logger::endl
    << "options:" << Logger::endl
    << "  -h,--help      Show this help message" << Logger::endl
    << "  -v,--version   Show the version number of this build" << Logger::endl
    << Logger::endl
    << "run 'autolab <command> -h' to view usage instructions for each command." << Logger::endl;
}

void print_version() {
  Logger::info << "autolab-cli version " << VERSION_MAJOR << "." << VERSION_MINOR << Logger::endl;
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
      Logger::fatal.set_prefix("Cannot start autolab client");

      if (!init_autolab_client()) {
        Logger::fatal << "No user set up on this client yet." << Logger::endl
          << Logger::endl
          << "Please run 'autolab setup' to setup your Autolab account." << Logger::endl;
        return 0;
      }
      try {
        if ("status" == command) {
          return show_status(cmd);
        } else if ("download" == command) {
          return download_asmt(cmd);
        } else if ("submit" == command) {
          return submit_asmt(cmd);
        } else if ("courses" == command) {
          return show_courses(cmd);
        } else if ("assessments" == command ||
                   "asmts" == command) {
          return show_assessments(cmd);
        } else if ("problems" == command) {
          return show_problems(cmd);
        } else if ("scores" == command) {
          return show_scores(cmd);
        } else if ("feedback" == command) {
          return show_feedback(cmd);
        } else if ("enroll" == command) {
          return manage_enrolls(cmd);
        } else {
          Logger::fatal << "Unrecognized command: " << command << Logger::endl;
        }
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
