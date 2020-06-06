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

#include "../app_credentials.h"
#include "../cache/cache.h"
#include "../context_manager/context_manager.h"
#include "../file/file_utils.h"
#include "../pretty_print/pretty_print.h"

#include "cmdargs.h"
#include "cmdimp.h"
#include "cmdmap.h"

Autolab::Client client(server_domain, client_id, client_secret, redirect_uri, store_tokens);

bool init_autolab_client() {
  std::string at, rt;
  if (!load_tokens(at, rt)) return false;
  client.set_tokens(at, rt);
  return true;
}

void print_not_in_asmt_dir_error() {
  Logger::fatal << "Not inside an autolab assessment directory: .autolab-asmt not found" << Logger::endl
    << Logger::endl
    << "Please change directory or specify the course and assessment names" << Logger::endl;
}

/* helpers */
int perform_device_flow(Autolab::Client &client) {
  Logger::info << "Initiating authorization..." << Logger::endl << Logger::endl;
  std::string user_code, verification_uri;
  client.device_flow_init(user_code, verification_uri);
  Logger::info << "Please visit "
    << Logger::CYAN << verification_uri << Logger::NONE << " and enter the code: "
    << Logger::CYAN << user_code << Logger::NONE << Logger::endl;
  Logger::info << Logger::endl << "Waiting for user authorization ..." << Logger::endl;

  int res = client.device_flow_authorize(300); // wait for 5 minutes max
  switch (res) {
    case 1:
      Logger::info << Logger::RED << "User denied authorization." << Logger::NONE << Logger::endl;
      return 1;
    case -2:
      Logger::info << Logger::RED << "Timed out while waiting for user action. Please try again."
        << Logger::NONE << Logger::endl;
      return 1;
  }

  // res == 0
  Logger::info << Logger::GREEN << "Received authorization!" << Logger::NONE << Logger::endl;

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

/* table creators */

// create a submissions scores table, returns the number of data rows (not
// incl. the header row).
int create_scores_table(
    std::vector<std::vector<std::string>> &table,
    std::vector<Autolab::Problem> &problems,
    std::vector<Autolab::Submission> &subs,
    std::size_t max_num_subs) {

  // prepare table header
  std::vector<std::string> header;
  header.push_back("version");
  for (auto &p : problems) {
    std::string column(p.name);
    if (!std::isnan(p.max_score)) {
      column += " (" + double_to_string(p.max_score, 1) + ")";
    }
    header.push_back(column);
  }
  table.push_back(header);

  // prepare table body
  int nprint = std::min(subs.size(), max_num_subs);
  for (int i = 0; i < nprint; i++) {
    std::vector<std::string> row;
    Autolab::Submission &s = subs[i];
    row.push_back(std::to_string(s.version));

    auto &scores_map = s.scores;
    for (auto &p : problems) {
      auto score = scores_map.find(p.name); // find by problem name
      if (score != scores_map.end() && !std::isnan(score->second)) {
        row.push_back(double_to_string(score->second, 1));
      } else {
        row.push_back("--");
      }
    }

    table.push_back(row);
  }

  return nprint;
}

/* commands */

int show_status(cmdargs &cmd) {
  cmd.setup_help("autolab status",
      "Show the context of the current directory. If inside an assessment "
      "directory, the details of the assessment will be shown.");
  cmd.setup_done();

  std::string course_name, asmt_name;
  bool in_asmt_dir = read_asmt_file(course_name, asmt_name);
  if (!in_asmt_dir) {
    Logger::info << "Not currently in any assessment directory" << Logger::endl
      << Logger::endl
      << "Failed to find an assessment config file in the current directory or any" << Logger::endl
      << "of its parent directories (up to " << DEFAULT_RECUR_LEVEL << " levels)." << Logger::endl;
    return 0;
  }

  Logger::info << "Assessment Config: " << course_name << ":" << asmt_name
    << Logger::endl << Logger::endl;

  // get details
  Autolab::DetailedAssessment dasmt;
  client.get_assessment_details(dasmt, course_name, asmt_name);

  Logger::info << dasmt.asmt.display_name << Logger::endl
    << "Due: " << std::ctime(&dasmt.asmt.due_at) // ctime ends string with '\n'
    << "Max submissions: ";

  if (dasmt.max_submissions < 0) {
    Logger::info << "Infinite" << Logger::endl;
  } else {
    Logger::info << dasmt.max_submissions << Logger::endl;
  }

  Logger::info << "Max grace days: ";
  if (dasmt.max_grace_days < 0) {
    Logger::info << "As many as you have left" << Logger::endl;
  } else {
    Logger::info << dasmt.max_grace_days << Logger::endl;
  }

  return 0;
}

/* download assessment into a new directory
 */
int download_asmt(cmdargs &cmd) {
  cmd.setup_help("autolab download",
      "Create a directory for working on the specified assessment. The writeup "
      "and the handout are downloaded into the directory if they are files. "
      "The assessment directory is also setup with a local config so that "
      "running certain commands inside it works without the need to specify "
      "the names of the course and assessment.");
  cmd.new_arg("course_name:assessment_name", true);
  cmd.setup_done();

  // parse course and assessment name
  std::string course_name, asmt_name;
  parse_course_and_asmt(cmd.args[2], course_name, asmt_name);

  Logger::info << "Querying assessment '" << asmt_name << "' of course '" <<
    course_name << "' ..." << Logger::endl;

  // make sure assessment exists
  Autolab::DetailedAssessment dasmt;
  client.get_assessment_details(dasmt, course_name, asmt_name);

  // setup directory
  bool dir_exists = dir_find(get_curr_dir(), asmt_name.c_str(), true);
  if (dir_exists) {
    Logger::fatal << "Directory named '" << asmt_name << "' already exists. "
      << "Please delete or rename before proceeding." << Logger::endl;
    return 0;
  }

  std::string new_dir(get_curr_dir());
  new_dir.append("/" + asmt_name);
  Logger::info << "Creating directory " << new_dir << Logger::endl;

  create_dir(new_dir.c_str());

  // download files into directory
  Autolab::Attachment handout, writeup;
  client.download_handout(handout, new_dir, course_name, asmt_name);
  switch (handout.format) {
    case Autolab::AttachmentFormat::none:
      Logger::info << "Assessment has no handout" << Logger::endl;
      break;
    case Autolab::AttachmentFormat::url:
      Logger::info << "Handout URL: " << handout.url << Logger::endl;
      break;
    case Autolab::AttachmentFormat::file:
      Logger::info << "Handout downloaded into assessment directory" << Logger::endl;
      break;
  }

  client.download_writeup(writeup, new_dir, course_name, asmt_name);
  switch (writeup.format) {
    case Autolab::AttachmentFormat::none:
      Logger::info << "Assessment has no writeup" << Logger::endl;
      break;
    case Autolab::AttachmentFormat::url:
      Logger::info << "Writeup URL: " << writeup.url << Logger::endl;
      break;
    case Autolab::AttachmentFormat::file:
      Logger::info << "Writeup downloaded into assessment directory" << Logger::endl;
      break;
  }

  // write assessment file
  write_asmt_file(new_dir, course_name, asmt_name);

  // additional info
  Logger::info << Logger::endl << "Due: " << std::ctime(&dasmt.asmt.due_at);

  return 0;
}

/* two ways of calling:
 *   1. autolab submit <filename>                  (must have autolab-asmt file)
 *   2. autolab submit <course>:<asmt> <filename>  (from anywhere)
 */
int submit_asmt(cmdargs &cmd) {
  cmd.setup_help("autolab submit",
      "Submit a file to an assessment. The course and assessment names are not "
      "needed if the current directory or its ancestor directories include an "
      "assessment config file. The operation fails if the specified names and "
      "the config file do not match, unless the '-f' option is used, in which "
      "case the assessment config file is ignored.");
  cmd.new_arg("course_name:assessment_name", false);
  cmd.new_arg("filename", true);
  bool option_force = cmd.new_flag_option("-f","--force", "Force use the "
    "specified course:assessment pair, overriding the local config");
  bool option_wait = cmd.new_flag_option("-w","--wait", "Wait until the "
    "autograder is finished, then display the scores for this submission");
  cmd.setup_done();

  std::string course_name, asmt_name, filename;

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
  int version = client.submit_assessment(course_name, asmt_name, filename);

  Logger::info << Logger::GREEN << "Successfully submitted to Autolab (version " << version << ")" << Logger::NONE << Logger::endl;

  if (option_wait) {
    Logger::info << Logger::endl
      << "Waiting for scores to be ready ..." << Logger::endl;
    // wait for at least some scores to be available.
    // Don't wait for all scores because the autograder may not assign scores
    // to all problems
    bool scores_ready = false;
    std::chrono::seconds timeout(300); // 5 minutes
    std::chrono::seconds wait_per_trial(5);
    auto t_now = std::chrono::steady_clock::now();
    auto t_end = t_now + timeout;
    int target_sub_idx = -1;
    std::vector<Autolab::Submission> subs;
    while (t_now < t_end && !scores_ready) {
      subs.clear();
      client.get_submissions(subs, course_name, asmt_name);

      target_sub_idx = -1;
      for (std::size_t i = 0; i < subs.size(); i++) {
        if (subs[i].version == version) {
          // this is our version
          target_sub_idx = i;
          break;
        }
      }
      if (target_sub_idx < 0) {
        Logger::fatal << "Failed to get scores for this current submission."
          << Logger::endl;
        return -1;
      }

      for (auto &score : subs[target_sub_idx].scores) {
        if (!std::isnan(score.second)) {
          scores_ready = true;
          break;
        }
      }
      if (scores_ready) break;

      std::this_thread::sleep_for(wait_per_trial);
      t_now = std::chrono::steady_clock::now();
    }

    if (scores_ready) {
      // found scores
      std::vector<Autolab::Submission> one_sub = { subs[target_sub_idx] };
      // get problem names
      std::vector<Autolab::Problem> problems;
      client.get_problems(problems, course_name, asmt_name);
      // draw the table
      std::vector<std::vector<std::string>> sub_table;
      create_scores_table(sub_table, problems, one_sub, 1);
      Logger::info << format_table(sub_table);
    } else {
      // time out
      Logger::info << "Timed out while waiting for scores to be ready."
        << Logger::endl
        << "You can still use 'autolab scores' to view your scores anytime."
        << Logger::endl;
      return 0;
    }
  }

  return 0;
}

int show_courses(cmdargs &cmd) {
  cmd.setup_help("autolab courses",
      "List all current courses of the user.");
  cmd.setup_done();

  // hidden option --use-cache
  if (cmd.has_option("-u", "--use-cache")) {
    print_course_cache_entry();
    return 0;
  }

  std::vector<Autolab::Course> courses;
  client.get_courses(courses);
  LogDebug("Found " << courses.size() << " current courses." << Logger::endl);

  std::string course_name_config, asmt_name_config;
  read_asmt_file(course_name_config, asmt_name_config);
  std::string course_name_config_lower = to_lowercase(course_name_config);

  for (auto &c : courses) {
    bool is_curr_asmt = (course_name_config_lower == to_lowercase(c.name));
    if (is_curr_asmt) {
      Logger::info << "* " << Logger::GREEN;
    } else {
      Logger::info << "  ";
    }

    Logger::info << c.name << " (" << c.display_name << ")" << Logger::endl;

    if (is_curr_asmt) {
      Logger::info << Logger::NONE;
    }
  }

  // save to cache as well
  update_course_cache_entry(courses);

  return 0;
}

// list and CRUD enrollments
int manage_enrolls(cmdargs &cmd) {
  cmd.setup_help("autolab enroll",
      "actions:\n"
      "  new   Create a new enrollment for a course\n"
      "  edit  Modify an existing enrollment for a course\n"
      "  drop  Drop a student from a course\n"
      "\n"
      "List, create, and update users affiliated with a course, including "
      "students, course assistants, and instructors.");
  cmd.new_arg("action", false);
  cmd.new_arg("course_name", true);
  std::string option_user = cmd.new_option("-u","--user","email","Email of the user");
  std::string option_lecture = cmd.new_option("-l","--lecture","lecture","Lecture to assign to");
  std::string option_section = cmd.new_option("-s","--section","section","Section to assign to");
  std::string option_grade_policy = cmd.new_option("-p","--grade-policy","policy","Student's grading policy");
  std::string option_nickname = cmd.new_option("-n","--nickname","name","User's nickname");
  bool option_set_dropped = cmd.new_flag_option("--set-dropped","","Set user to dropped");
  std::string option_auth_level = cmd.new_option("-t","--type","type","User's authorization level."
      " One of 'student', 'course_assistant', or 'instructor'");
  bool option_verbose = cmd.new_flag_option("-v","--verbose","Show the resulting "
      "enrollment data after new, edit, or delete");
  cmd.setup_done();

  std::vector<Autolab::Enrollment> enrollments;
  if (cmd.nargs() == 4) {
    std::string action(cmd.args[2]);
    std::string course_name(cmd.args[3]);
    // member actions on enrollments require the email
    if (option_user == "") {
      Logger::fatal << "Must specify email of user with '-u'" << Logger::endl;
      return -1;
    }

    // prepare input data
    Autolab::EnrollmentOption enroll;
    if (action == "new" || action == "edit") {
      if (nonempty(option_lecture))
        enroll.lecture = option_lecture;
      if (nonempty(option_section))
        enroll.section = option_section;
      if (nonempty(option_grade_policy))
        enroll.grade_policy = option_grade_policy;
      if (nonempty(option_nickname))
        enroll.nickname = option_nickname;
      if (nonempty(option_auth_level)) {
        if (option_auth_level != "student" &&
            option_auth_level != "course_assistant" &&
            option_auth_level != "instructor") {
          Logger::fatal << "Unrecognized authorization level: '" << option_auth_level
              << "'. Must be one of 'student', 'course_assistant', or 'instructor'"
              << Logger::endl;
          return -1;
        }
        enroll.auth_level =
            Autolab::Utility::string_to_authorization_level(option_auth_level);
      }
      enroll.dropped = option_set_dropped;
    }

    // parse CRUD action
    Autolab::CrudAction crud_action = Autolab::CrudAction::Create;
    if (action == "new") {
      crud_action = Autolab::CrudAction::Create;
    } else if (action == "edit") {
      crud_action = Autolab::CrudAction::Update;
    } else if (action == "drop") {
      crud_action = Autolab::CrudAction::Delete;
    } else {
      Logger::fatal << "Invalid action: " << action << Logger::endl
        << "Must be one of 'new', 'edit', or 'delete'" << Logger::endl;
      return -1;
    }

    Autolab::Enrollment result;
    client.crud_enrollment(result, course_name, option_user, enroll, crud_action);
    enrollments.push_back(result);
  } else {
    std::string course_name(cmd.args[2]);
    // list all enrollments
    client.get_enrollments(enrollments, course_name);
    LogDebug("Found " << enrollments.size() << " enrollments." << Logger::endl);
    // always show output for the list action
    option_verbose = true;
  }

  if (option_verbose) {
    // draw table
    std::vector<std::vector<std::string>> enrolls_table;
    // prepare table header
    std::vector<std::string> header;
    header.push_back("name");
    header.push_back("email");
    header.push_back("lecture");
    header.push_back("section");
    header.push_back("dropped?");
    header.push_back("type");
    enrolls_table.push_back(header);

    // prepare table body
    for (auto &e : enrollments) {
      std::vector<std::string> row;
      row.push_back(e.user.first_name + " " + e.user.last_name);
      row.push_back(e.user.email);
      row.push_back(e.lecture);
      row.push_back(e.section);
      row.push_back(bool_to_string(e.dropped));
      row.push_back(Autolab::Utility::authorization_level_to_string(e.auth_level));
      enrolls_table.push_back(row);
    }

    Logger::info << format_table(enrolls_table);
  }

  return 0;
}

int show_assessments(cmdargs &cmd) {
  cmd.setup_help("autolab assessments",
      "List all available assessments of a course.");
  cmd.new_arg("course_name", true);
  cmd.setup_done();

  std::string course_name(cmd.args[2]);

  // hidden option --use-cache
  if (cmd.has_option("-u", "--use-cache")) {
    print_asmt_cache_entry(course_name);
    return 0;
  }

  std::vector<Autolab::Assessment> asmts;
  client.get_assessments(asmts, course_name);
  LogDebug("Found " << asmts.size() << " assessments." << Logger::endl);

  std::string course_name_config, asmt_name_config;
  read_asmt_file(course_name_config, asmt_name_config);
  bool is_curr_course = case_insensitive_str_equal(course_name, course_name_config);
  std::string asmt_name_config_lower = to_lowercase(asmt_name_config);

  std::sort(asmts.begin(), asmts.end(), Autolab::Utility::compare_assessments_by_name);
  for (auto &a : asmts) {
    bool is_curr_asmt = is_curr_course && (asmt_name_config_lower == to_lowercase(a.name));
    if (is_curr_asmt) {
      Logger::info << "* " << Logger::GREEN;
    } else {
      Logger::info << "  ";
    }

    Logger::info << a.name << " (" << a.display_name << ")" << Logger::endl;

    if (is_curr_asmt) {
      Logger::info << Logger::NONE;
    }
  }

  // save to cache as well
  update_asmt_cache_entry(course_name, asmts);

  return 0;
}

int show_problems(cmdargs &cmd) {
  cmd.setup_help("autolab problems",
      "List all problems of an assessment. Course and assessment names are "
      "optional if inside an autolab assessment directory.");
  cmd.new_arg("course_name:assessment_name", false);
  cmd.setup_done();

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

  std::vector<Autolab::Problem> problems;
  client.get_problems(problems, course_name, asmt_name);

  LogDebug("Found " << problems.size() << " problems." << Logger::endl);

  for (auto &p : problems) {
    Logger::info << p.name;
    if (!std::isnan(p.max_score)) {
      Logger::info << " (" << p.max_score << ")" << Logger::endl;
    } else {
      Logger::info << Logger::endl;
    }
  }

  return 0;
}

int show_scores(cmdargs &cmd) {
  cmd.setup_help("autolab scores",
      "Show all scores the user got for an assessment. Course and assessment "
      "names are optional if inside an autolab assessment directory.");
  cmd.new_arg("course_name:assessment_name", false);
  bool option_all = cmd.new_flag_option("-a", "--all",
      "Show scores from all submission. Default shows only the latest");
  cmd.setup_done();

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

  std::vector<Autolab::Problem> problems;
  client.get_problems(problems, course_name, asmt_name);

  // get submissions
  std::vector<Autolab::Submission> subs;
  client.get_submissions(subs, course_name, asmt_name);
  LogDebug("Found " << subs.size() << " submissions." << Logger::endl);

  Logger::info << "Scores for " << course_name << ":" << asmt_name << Logger::endl
    << Logger::endl;

  std::vector<std::vector<std::string>> sub_table;
  int max_num_rows = option_all ? subs.size() : 1;
  int num_rows = create_scores_table(sub_table, problems, subs, max_num_rows);

  Logger::info << format_table(sub_table);
  if (num_rows == 0) {
    Logger::info << "[empty]" << Logger::endl;
  }

  return 0;
}

int show_feedback(cmdargs &cmd) {
  cmd.setup_help("autolab feedback",
      "Gets feedback for a problem of an assessment. If version number is not "
      "given, the latest version will be used. If problem_name is not given, "
      "the first problem will be used. Course and assessment names are "
      "optional if inside an autolab assessment directory.");
  cmd.new_arg("course_name:assessment_name", false);
  std::string option_problem = cmd.new_option("-p", "--problem","problem_name",
      "Get feedback for this problem");
  std::string option_version = cmd.new_option("-v", "--version","version_num",
      "Get feedback for this particular version");
  cmd.setup_done();

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
    std::vector<Autolab::Submission> subs;
    client.get_submissions(subs, course_name, asmt_name);

    if (subs.size() == 0) {
      Logger::fatal << "No submissions available for this assessment." << Logger::endl;
      return 0;
    }

    version = subs[0].version;
  } else {
    version = std::stoi(option_version);
  }

  // determine problem name
  if (option_problem.length() == 0) {
    // use first problem
    std::vector<Autolab::Problem> problems;
    client.get_problems(problems, course_name, asmt_name);

    if (problems.size() == 0) {
      Logger::fatal << "This assessment has no problems." << Logger::endl;
      return 0;
    }

    option_problem = problems[0].name;
  }
  LogDebug("Using problem name: " << option_problem << Logger::endl);

  std::string feedback;
  client.get_feedback(feedback, course_name, asmt_name, version, option_problem);

  Logger::info << feedback << Logger::endl;
  return 0;
}
