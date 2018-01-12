#include <iostream>

#include "autolab_client.h"
#include "client_helpers.h"
#include "config_manager.h"
#include "file_utils.h"
#include "logger.h"

using namespace std;

const string client_id = "c021c9b12dc597b5b42d783ee285a2bc9a8afcce4a60db5b4b97a1cda551f48d";
const string client_secret = "5cce1a3f5968308defe8a15d4cb1e47250fc88976ebf9c084155fc86da8050f1";
const string redirect_uri = "http://localhost:3000/device_flow_auth_cb";

void print_download_help() {
  cout << "Usage: autolab download [asmt-name-hint]" << endl;
}

void save_tokens(AutolabClient &ac) {
  string at = ac.get_access_token();
  string rt = ac.get_refresh_token();

  Logger::debug << "Saving tokens to arcache: " << Logger::endl <<
                   "access_token: " << at << Logger::endl <<
                   "refresh_token: " << rt << Logger::endl;

  store_tokens(at, rt);
}

int perform_device_flow(AutolabClient &ac) {
  Logger::info << "Initiating authorization..." << Logger::endl << Logger::endl;
  string user_code, verification_uri;
  ac.device_flow_init(user_code, verification_uri);
  Logger::info << "Please visit " << verification_uri << " and enter the code: " << user_code << Logger::endl;
  Logger::info << "Waiting for user authorization ..." << Logger::endl;

  int res = ac.device_flow_authorize(300); // wait for 5 minutes max
  if (res == 1) {
    Logger::info << "User denied authorization." << Logger::endl;
    return 1;
  } else if (res < 0) {
    Logger::info << "Error occurred during authorization." << Logger::endl;
    return -1;
  }

  // res == 0
  Logger::info << "Received authorization!" << Logger::endl;

  save_tokens(ac);  
  return 0;
}

/* if course name not provided:
 * find course_name from environment
 * if not found, return error and display
 * if found
 * if course name provided
 * get all asmts
 * loop through and find the one that includes the asmt_hint
 * if not found, return error and display
 * if found:
 *   create directory using asmt name
 *   create and write .autolab-asmt in it
 *   download handout and writeup into directory
 */
int download_asmt(int argc, char *argv[]) {
  if (argc < 3) {
    print_download_help();
    return 0;
  }

  // set up logger
  Logger::fatal.set_prefix("Cannot download assessment");

  // parse course and assessment name
  string raw_input(argv[2]);
  string::size_type split_pos = raw_input.find(":");
  if (split_pos == string::npos) {
    // the entire string is the assessment name.
    // attempt to load course name from config if exists,
    // otherwise report error.
    Logger::fatal << "Please specify course name." << Logger::endl;
    return 0;
  }
  string course_name = raw_input.substr(0, split_pos);
  string asmt_name = raw_input.substr(split_pos + 1, string::npos);

  Logger::info << "Querying assessment '" << asmt_name << "' of course '" << 
    course_name << "' ..." << Logger::endl;

  // setup AutolabClient
  AutolabClient ac = AutolabClient(client_id, client_secret, redirect_uri);

  string at, rt;
  load_tokens(at, rt);
  ac.set_tokens(at, rt);

  // look for course
  if (!find_course(ac, course_name)) {
    Logger::fatal << "Failed to find the specified course" << Logger::endl;
    return 0;
  }

  // find assessment
  if (!find_assessment(ac, course_name, asmt_name)) {
    Logger::fatal << "Failed to find the specified assessment" << Logger::endl;
    return 0;
  }

  // setup directory
  bool dir_exists = dir_find(get_curr_dir(), asmt_name.c_str(), true);
  if (dir_exists) {
    Logger::fatal << "Directory named '" << asmt_name << "' already exists. Please delete or rename before proceeding." << Logger::endl;
    return 0;
  }

  std::string new_dir(get_curr_dir());
  new_dir.append("/" + asmt_name);
  Logger::info << "Creating directory " << new_dir << Logger::endl;

  bool created = create_dir(new_dir.c_str());
  err_assert(created);

  // download files into directory
  ac.download_handout(new_dir, course_name, asmt_name);

  Logger::debug << "success" << Logger::endl;
}

int user_setup(int argc, char *argv[]) {
  bool force_setup = false;

  string at, rt;
  bool user_exists = load_tokens(at, rt);
  AutolabClient ac = AutolabClient(client_id, client_secret, redirect_uri);
  ac.set_tokens(at, rt);

  if (!force_setup) {
    // perform a check if not a forced setup
    if (user_exists) {
      bool token_valid = true;
      rapidjson::Document user_info;
      try {
        ac.get_user_info(user_info);
      } catch (AutolabClient::InvalidTokenException &e) {
        token_valid = false;
      }
      if (token_valid && user_info.HasMember("first_name")) {
        Logger::info << "User '" << user_info["first_name"].GetString() << "' is currently set up on this client." << Logger::endl <<
          "To force reset of user info, use the '-f' option." << Logger::endl;
        return 0;
      }
    }
  }

  // user non-existant, or existing user's credentials no longer work
  int result = perform_device_flow(ac);
  if (result == 0) {
    Logger::info << Logger::endl << "User setup complete." << Logger::endl;
    return 0;
  }
  Logger::info << Logger::endl << "User setup failed." << Logger::endl;
  return -1;
}

void print_help() {
  cout << "HELP TEXT" << endl;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_help();
    return 0;
  }

  // determine what command it is
  std::string command(argv[1]);
  if ("setup" == command) {
    return user_setup(argc, argv);
  } else if ("download" == command) {
    return download_asmt(argc, argv);
  } else {
    Logger::fatal << "Unrecognized command: " << command << Logger::endl;
  }

  return 0;
}