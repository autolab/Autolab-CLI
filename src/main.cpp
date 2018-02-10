#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "app_credentials.h"
#include "build_config.h"
#include "cmdargs.h"
#include "context_manager.h"
#include "file_utils.h"

#include "autolab/autolab.h"
#include "autolab/raw_client.h"
#include "logger.h"

typedef Autolab::RawClient AutolabClient;

/* globals */
AutolabClient ac = AutolabClient(client_id, client_secret, redirect_uri, store_tokens);

bool init_autolab_client(AutolabClient &ac) {
  std::string at, rt;
  if (!load_tokens(at, rt)) return false;
  ac.set_tokens(at, rt);
  return true;
}

/* help texts */
void print_help() {
  Logger::info << "usage: autolab [OPTIONS] <command> [command-args] [command-opts]" << Logger::endl
    << Logger::endl
    << "commands:" << Logger::endl
    << "  courses             List all courses" << Logger::endl
    << "  assessments/asmts   List all assessments of a course" << Logger::endl
    << "  problems            List all problems in an assessment" << Logger::endl
    << "  download            Download files needed for an assessment" << Logger::endl
    << "  submit              Submit a file to an assessment" << Logger::endl
    << "  scores              Show scores got on an assessment" << Logger::endl
    << "  setup               Setup the user of the client" << Logger::endl
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

void print_download_help() {
  Logger::info << "usage: autolab download <course_name>:<assessment_name> [OPTIONS]" << Logger::endl
    << Logger::endl
    << "options:" << Logger::endl
    << "  -h,--help   Show this help message" << Logger::endl
    << Logger::endl
    << "Create a directory for working on the specified assessment. The writeup and the" << Logger::endl
    << "handout are downloaded into the directory if they are files. The assessment" << Logger::endl
    << "directory is also setup with a local config so that running" << Logger::endl
    << "'autolab submit <filename>' works without the need to specify the names of the" << Logger::endl
    << "course and assessment." << Logger::endl;
}

void print_submit_help() {
  Logger::info << "usage: autolab submit [<course_name>:<assessment_name>] <filename> [OPTIONS]" << Logger::endl
    << Logger::endl
    << "options:" << Logger::endl
    << "  -h,--help   Show this help message" << Logger::endl
    << "  -f,--force  Force use the specified course:assessment pair, overriding the" << Logger::endl
    << "              local config" << Logger::endl
    << Logger::endl
    << "Submit a file to an assessment. The course and assessment names are not needed" << Logger::endl
    << "if the current directory or its parent directory (up to " << DEFAULT_RECUR_LEVEL << " levels) includes" << Logger::endl
    << "an assessment config file. The operation fails if the specified names and the" << Logger::endl
    << "config file do not match, unless the '-f' option is used, in which case the" << Logger::endl
    << "assessment config file is ignored." << Logger::endl;
}

void print_courses_help() {
  Logger::info << "usage: autolab courses [OPTIONS]" << Logger::endl
    << Logger::endl
    << "options:" << Logger::endl
    << "  -h,--help   Show this help message" << Logger::endl
    << Logger::endl
    << "List all current courses of the user." << Logger::endl;
}

void print_assessments_help() {
  Logger::info << "usage: autolab assessments <course_name> [OPTIONS]" << Logger::endl
    << Logger::endl
    << "options:" << Logger::endl
    << "  -h,--help   Show this help message" << Logger::endl
    << Logger::endl
    << "List all available assessments of a course." << Logger::endl;
}

void print_problems_help() {
  Logger::info << "usage: autolab problems [<course_name>:<assessment_name>] [OPTIONS]" << Logger::endl
    << Logger::endl
    << "options:" << Logger::endl
    << "  -h,--help   Show this help message" << Logger::endl
    << Logger::endl
    << "List all problems of an assessment. Course and assessment names are optional if" << Logger::endl
    << "inside an autolab assessment directory." << Logger::endl;
}

void print_scores_help() {
  Logger::info << "usage: autolab scores [<course_name>:<assessment_name>] [OPTIONS]" << Logger::endl
    << Logger::endl
    << "options:" << Logger::endl
    << "  -h,--help   Show this help message" << Logger::endl
    << "  -a,--all    Show scores from all submission. Default shows only the latest" << Logger::endl
    << Logger::endl
    << "Show all scores the user got for an assessment. Course and assessment names are" << Logger::endl
    << "optional if inside an autolab assessment directory." << Logger::endl;
}

void print_feedback_help() {
  Logger::info << "usage: autolab feedback [<course_name>:<assessment_name>] [OPTIONS]" << Logger::endl
    << Logger::endl
    << "options:" << Logger::endl
    << "  -v,--version <version_num>    Get feedback for this particular version" << Logger::endl
    << "  -p,--problem <problem_name>   Get feedback for this problem" << Logger::endl
    << "  -h,--help                     Show this help message" << Logger::endl
    << Logger::endl
    << "Gets feedback for a problem of an assessment. If version number is not given, " << Logger::endl
    << "the latest version will be used. If problem_name is not given, the first" << Logger::endl
    << "problem will be used. Course and assessment names are optional if inside" << Logger::endl
    << "an autolab assessment directory." << Logger::endl;
}

void print_setup_help() {
  Logger::info << "usage: autolab assessments <course_name> [OPTIONS]" << Logger::endl
    << Logger::endl
    << "options:" << Logger::endl
    << "  -h,--help   Show this help message" << Logger::endl
    << "  -f,--force  Force user setup, removing the current user" << Logger::endl
    << Logger::endl
    << "Initiate user setup for the current user." << Logger::endl;
}

void print_not_in_asmt_dir_error() {
  Logger::fatal << "Not inside an autolab assessment directory: .autolab-asmt not found" << Logger::endl
    << Logger::endl
    << "Please change directory or specify the course and assessment names" << Logger::endl;
}

/* helpers */
int perform_device_flow(AutolabClient &ac) {
  Logger::info << "Initiating authorization..." << Logger::endl << Logger::endl;
  std::string user_code, verification_uri;
  ac.device_flow_init(user_code, verification_uri);
  Logger::info << "Please visit " << verification_uri << " and enter the code: " << user_code << Logger::endl;
  Logger::info << Logger::endl << "Waiting for user authorization ..." << Logger::endl;

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

  return 0;
}

/* exit if failed to parse */
void parse_course_and_asmt(std::string raw_input, std::string &course, std::string &asmt) {
  std::string::size_type split_pos = raw_input.find(":");
  if (split_pos == std::string::npos) {
    Logger::fatal << "Failed to parse course name and assessment name: " << raw_input << Logger::endl;
    exit(0);
  }
  course = raw_input.substr(0, split_pos);
  asmt = raw_input.substr(split_pos + 1, std::string::npos);
}

/* if the supplied names are empty, it assigns them the values from the autolab asmt file.
 * if the context file doesn't exist, it reports an error and exits.
 * If the user does specify names and they don't match, it reports an error and exits.
 */
void check_names_with_asmt_file(std::string &course_name, std::string &asmt_name) {
  bool user_specified_names = (course_name.length() > 0) || (asmt_name.length() > 0);
  std::string course_name_config, asmt_name_config;
  // attempt to load names from asmt-file
  bool found_asmt_file = read_asmt_file(course_name_config, asmt_name_config);
  if (!found_asmt_file && !user_specified_names) {
    print_not_in_asmt_dir_error();
    exit(0);
  }

  if (found_asmt_file && user_specified_names) {
    if ((course_name != course_name_config) || (asmt_name != asmt_name_config)) {
      Logger::fatal << "The provided names and the configured names for this autolab assessment directory do not match:" << Logger::endl
        << "Provided names:   " << course_name  << ":" << asmt_name << Logger::endl
        << "Configured names: " << course_name_config << ":" << asmt_name_config << Logger::endl << Logger::endl
        << "Please resolve this conflict, or use the '-f' option to force the use of the provided names." << Logger::endl;
      exit(0);
    }
  }

  if (found_asmt_file && !user_specified_names) {
    course_name = course_name_config;
    asmt_name = asmt_name_config;
  }

  // if !found_asmt_file && user_specified_names, we don't need to do anything
}

struct namepair {
  std::string name;
  std::string display_name;
  namepair(std::string n, std::string dn) :
    name(n), display_name(dn) {}
};
bool namepair_comparator(const namepair &a, const namepair &b) {
  if (a.name == b.name) return a.display_name < b.display_name;
  return a.name < b.name;
}

/* commands */

/* download assessment into a new directory
 */
int download_asmt(cmdargs &cmd) {
  if (cmd.nargs() < 3 || cmd.has_option("-h", "--help")) {
    print_download_help();
    return 0;
  }

  // set up logger
  Logger::fatal.set_prefix("Cannot download assessment");

  // parse course and assessment name
  std::string course_name, asmt_name;
  parse_course_and_asmt(cmd.args[2], course_name, asmt_name);

  Logger::info << "Querying assessment '" << asmt_name << "' of course '" << 
    course_name << "' ..." << Logger::endl;

  // make sure assessment exists
  rapidjson::Document asmt;
  ac.get_assessment_details(asmt, course_name, asmt_name);
  check_error_and_exit(asmt);

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
  rapidjson::Document response;
  ac.download_handout(response, new_dir, course_name, asmt_name);
  check_error_and_exit(response);
  if (response.IsObject()) {
    if (response.HasMember("url"))
      Logger::info << "Handout URL: " << response["url"].GetString() << Logger::endl;
    else
      Logger::info << "Assessment has no handout" << Logger::endl;
  } else {
    Logger::info << "Handout downloaded into assessment directory" << Logger::endl;
  }

  ac.download_writeup(response, new_dir, course_name, asmt_name);
  check_error_and_exit(response);
  if (response.IsObject()) {
    if (response.HasMember("url"))
      Logger::info << "Writeup URL: " << response["url"].GetString() << Logger::endl;
    else
      Logger::info << "Assessment has no writeup" << Logger::endl;
  } else {
    Logger::info << "Writeup downloaded into assessment directory" << Logger::endl;
  }

  // write assessment file
  write_asmt_file(new_dir, course_name, asmt_name);

  return 0;
}

/* two ways of calling:
 *   1. autolab submit <filename>                  (must have autolab-asmt file)
 *   2. autolab submit <course>:<asmt> <filename>  (from anywhere)
 */
int submit_asmt(cmdargs &cmd) {
  if (cmd.nargs() < 3 || cmd.has_option("-h", "--help")) {
    print_submit_help();
    return 0;
  }

  bool option_force = cmd.has_option("-f", "--force");

  std::string course_name, asmt_name, filename;

  // set up logger
  Logger::fatal.set_prefix("Cannot submit assessment");

  if (cmd.nargs() >= 4) {
    // user provided course and assessment name with filename
    parse_course_and_asmt(cmd.args[2], course_name, asmt_name);
    filename = cmd.args[3];
  } else {
    // user only provided filename
    if (option_force) {
      Logger::fatal << "The '-f' option can only be used when the course and assessment names are also specified." << Logger::endl;
      return 0;
    }
    filename = cmd.args[2];
  }

  if (!option_force) {
    // only check local version if non-force
    check_names_with_asmt_file(course_name, asmt_name);
  }

  if (!file_exists(filename.c_str())) {
    Logger::fatal << "File not found: " << filename << Logger::endl;
    return 0;
  }

  Logger::info << "Submitting to " << course_name << ":" << asmt_name << " ...";
  if (option_force) {
    Logger::info << " (force)" << Logger::endl;
  } else {
    Logger::info << Logger::endl;
  }

  // conflicts resolved, use course_name and asmt_name from now on
  rapidjson::Document response;
  ac.submit_assessment(response, course_name, asmt_name, filename);
  check_error_and_exit(response);

  if (response.IsObject()) {
    Logger::info << "Successfully submitted to Autolab (version " << response["version"].GetInt() << ")" << Logger::endl;
  }
  return 0;
}

int show_courses(cmdargs &cmd) {
  if (cmd.has_option("-h", "--help")) {
    print_courses_help();
    return 0;
  }

  // set up logger
  Logger::fatal.set_prefix("Cannot get courses");

  rapidjson::Document courses;
  ac.get_courses(courses);
  check_error_and_exit(courses);

  Logger::debug << "Found " << courses.Size() << " current courses." << Logger::endl;

  std::vector<namepair> course_list;
  for (auto &c : courses.GetArray()) {
    std::string course_name = c.GetObject()["name"].GetString();
    std::string course_display_name = c.GetObject()["display_name"].GetString();
    course_list.emplace_back(course_name, course_display_name);
  }
  std::sort(course_list.begin(), course_list.end(), namepair_comparator);

  for (auto &c : course_list) {
    Logger::info << c.name << " (" << c.display_name << ")" << Logger::endl;
  }

  return 0;
}

int show_assessments(cmdargs &cmd) {
  if (cmd.nargs() < 3 || cmd.has_option("-h", "--help")) {
    print_assessments_help();
    return 0;
  }

  // set up logger
  Logger::fatal.set_prefix("Cannot get assessments");

  std::string course_name(cmd.args[2]);

  rapidjson::Document asmts;
  ac.get_assessments(asmts, course_name);
  check_error_and_exit(asmts);

  Logger::debug << "Found " << asmts.Size() << " assessments." << Logger::endl;

  std::vector<namepair> asmt_list;
  for (auto &a : asmts.GetArray()) {
    std::string asmt_name = a.GetObject()["name"].GetString();
    std::string asmt_display_name = a.GetObject()["display_name"].GetString();
    asmt_list.emplace_back(asmt_name, asmt_display_name);
  }
  std::sort(asmt_list.begin(), asmt_list.end(), namepair_comparator);

  for (auto &c : asmt_list) {
    Logger::info << c.name << " (" << c.display_name << ")" << Logger::endl;
  }

  return 0;
}

int show_problems(cmdargs &cmd) {
  if (cmd.has_option("-h", "--help")) {
    print_problems_help();
    return 0;
  }

  // set up logger
  Logger::fatal.set_prefix("Cannot get problems");
  
  std::string course_name, asmt_name;
  // user-specified names take precedence
  if (cmd.nargs() >= 3) {
    parse_course_and_asmt(cmd.args[2], course_name, asmt_name);
  } else {
    if (!read_asmt_file(course_name, asmt_name)) {
      print_not_in_asmt_dir_error();
      exit(0);
    }
  }

  rapidjson::Document problems;
  ac.get_problems(problems, course_name, asmt_name);
  check_error_and_exit(problems);

  Logger::debug << "Found " << problems.Size() << " problems." << Logger::endl;

  for (auto &p : problems.GetArray()) {
    std::string problem_name = p.GetObject()["name"].GetString();
    Logger::info << problem_name;
    if (p.GetObject()["max_score"].IsFloat()) {
      float max_score = p.GetObject()["max_score"].GetFloat();
      Logger::info << " (" << max_score << ")" << Logger::endl;
    } else {
      Logger::info << Logger::endl;
    }
  }

  return 0;
}

int show_scores(cmdargs &cmd) {
  static const int col1_length = 9;

  if (cmd.has_option("-h", "--help")) {
    print_scores_help();
    return 0;
  }

  // set up logger
  Logger::fatal.set_prefix("Cannot get scores");

  bool option_all = cmd.has_option("-a", "--all");

  std::string course_name, asmt_name;
  // user-specified names take precedence
  if (cmd.nargs() >= 3) {
    parse_course_and_asmt(cmd.args[2], course_name, asmt_name);
  } else {
    if (!read_asmt_file(course_name, asmt_name)) {
      print_not_in_asmt_dir_error();
      exit(0);
    }
  }

  rapidjson::Document problems;
  ac.get_problems(problems, course_name, asmt_name);
  check_error_and_exit(problems);

  Logger::info << "Scores for " << course_name << ":" << asmt_name << Logger::endl
    << "(Only submissions made via this client can be shown)" << Logger::endl
    << Logger::endl;

  // print table header
  std::vector<std::pair<std::string, int>> prob_list;
  Logger::info << "| version | ";
  for (auto &p : problems.GetArray()) {
    std::string problem_name = p.GetObject()["name"].GetString();
    std::ostringstream column;
    column << problem_name;
    if (p.GetObject()["max_score"].IsFloat()) {
      float max_score = p.GetObject()["max_score"].GetFloat();
      column << " (" << max_score << ")";
    }
    column << " | ";
    Logger::info << column.str();
    prob_list.push_back({problem_name, column.str().length() - 1});
  }
  Logger::info << Logger::endl;

  // print horizontal line
  Logger::info << "+" << std::string(col1_length, '-') << "+";
  for (auto &p : prob_list) {
    Logger::info << std::string(p.second, '-') << "+";
  }
  Logger::info << Logger::endl;

  // get submissions
  rapidjson::Document subs;
  ac.get_submissions(subs, course_name, asmt_name);
  check_error_and_exit(subs);

  Logger::debug << "Found " << subs.Size() << " submissions." << Logger::endl;

  // print actual table
  if (subs.Size() == 0) {
    Logger::info << "[none]" << Logger::endl;
  } else {
    int nprint = option_all ? subs.Size() : 1;
    for (int i = 0; i < nprint; i++) {
      const auto &s = subs.GetArray()[i].GetObject();
      Logger::info << "|" << std::setw(col1_length) << s["version"].GetInt() << "|";

      const auto &score = s["scores"].GetObject();
      for (auto &p : prob_list) {
        Logger::info << std::setw(p.second);
        if (score.HasMember(p.first.c_str()) && score[p.first.c_str()].IsFloat()) {
          Logger::info << score[p.first.c_str()].GetFloat();
        } else {
          Logger::info << "--";
        }
        Logger::info << "|";
      }
      
      Logger::info << Logger::endl;
    }
  }

  return 0;
}

int show_feedback(cmdargs &cmd) {
  if (cmd.has_option("-h", "--help")) {
    print_feedback_help();
    return 0;
  }

  // set up logger
  Logger::fatal.set_prefix("Cannot get feedback");

  bool option_all = cmd.has_option("-a", "--all");
  std::string option_problem = cmd.get_option("-p", "--problem");
  std::string option_version = cmd.get_option("-v", "--version");

  std::string course_name, asmt_name;
  // user-specified names take precedence
  if (cmd.nargs() >= 3) {
    parse_course_and_asmt(cmd.args[2], course_name, asmt_name);
  } else {
    if (!read_asmt_file(course_name, asmt_name)) {
      print_not_in_asmt_dir_error();
      exit(0);
    }
  }

  // determine version number
  int version = -1;
  if (option_version.length() == 0) {
    // use latest version
    rapidjson::Document subs;
    ac.get_submissions(subs, course_name, asmt_name);
    check_error_and_exit(subs);

    if (subs.Size() == 0) {
      Logger::fatal << "No submissions available for this assessment." << Logger::endl;
      return 0;
    }

    version = subs.GetArray()[0].GetObject()["version"].GetInt();
  } else {
    version = std::stoi(option_version);
  }

  // determine problem name
  if (option_problem.length() == 0) {
    // use first problem
    rapidjson::Document problems;
    ac.get_problems(problems, course_name, asmt_name);
    check_error_and_exit(problems);

    if (problems.Size() == 0) {
      Logger::fatal << "This assessment has no problems." << Logger::endl;
      return 0;
    }

    option_problem = problems.GetArray()[0].GetObject()["name"].GetString();
  }
  Logger::debug << "Using problem name: " << option_problem << Logger::endl;

  rapidjson::Document feedback;
  ac.get_feedback(feedback, course_name, asmt_name, version, option_problem);
  check_error_and_exit(feedback);

  if (!feedback.HasMember("feedback")) {
    Logger::fatal << "Unexpected response. Expected 'feedback' key." << Logger::endl;
    return 0;
  }

  Logger::info << feedback["feedback"].GetString() << Logger::endl;
  return 0;  
}

/* must manually init ac */
int user_setup(cmdargs &cmd) {
  if (cmd.has_option("-h", "--help")) {
    print_setup_help();
    return 0;
  }

  bool option_force = cmd.has_option("-f", "--force");

  bool user_exists = init_autolab_client(ac);

  if (user_exists && !option_force) {
    // perform a check if not a forced setup
    bool token_valid = true;
    rapidjson::Document user_info;
    try {
      ac.get_user_info(user_info);
    } catch (Autolab::InvalidTokenException &e) {
      token_valid = false;
    }
    if (token_valid && user_info.HasMember("first_name")) {
      Logger::info << "User '" << user_info["first_name"].GetString() << "' is currently set up on this client." << Logger::endl <<
        "To force reset of user info, use the '-f' option." << Logger::endl;
      return 0;
    }
  }

  // user non-existant, or existing user's credentials no longer work, or forced
  int result = perform_device_flow(ac);
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
    Logger::fatal << "Invalid command line arguments. All options must come after" << Logger::endl
      << "all positional arguments (e.g. commands). For detailed usage, run with '-h'." << Logger::endl;
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
  if ("setup" == command) {
    return user_setup(cmd);
  } else {
    Logger::fatal.set_prefix("Cannot start autolab client");

    if (!init_autolab_client(ac)) {
      Logger::fatal << "No user set up on this client yet." << Logger::endl
        << Logger::endl
        << "Please run 'autolab setup' to setup your Autolab account." << Logger::endl;
      return 0;
    }
    try {
      if ("download" == command) {
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

  return 0;
}