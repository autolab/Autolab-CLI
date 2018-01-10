#include <iostream>

#include "autolab_client.h"
#include "config_manager.h"
#include "file_utils.h"

using namespace std;

void print_download_help() {
  cout << "Usage: autolab download [asmt-name-hint]" << endl;
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

  string client_id = "c021c9b12dc597b5b42d783ee285a2bc9a8afcce4a60db5b4b97a1cda551f48d";
  string client_secret = "5cce1a3f5968308defe8a15d4cb1e47250fc88976ebf9c084155fc86da8050f1";
  string redirect_uri = "http://localhost:3000/device_flow_auth_cb";

  AutolabClient ac = AutolabClient(client_id, client_secret, redirect_uri);

  string user_code = ac.device_flow_init();
  cout << "user code: " << user_code << endl;

  int res = ac.device_flow_authorize(60);
  cout << "authorize res: " << res << endl;

  string at = ac.get_access_token();
  string rt = ac.get_refresh_token();

  cout << "access_token: " << at << endl;
  cout << "refresh_token: " << rt << endl;

  store_tokens(at, rt);

  cout << "store success" << endl;
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
    cout << "Perform setup!" << endl;
  } else if ("download" == command) {
    cout << "Performing download" << endl;
    return download_asmt(argc, argv);
  } else {
    cout << "Unrecognized command: " << command << endl;
  }

  return 0;
}