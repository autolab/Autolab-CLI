#include "cmdmap.h"
#include "logger.h"
#include "cmdimp.h"

int CommandMap::exec_command(cmdargs &cmd, std::string raw_command) {
  // Translate to default command name and check if it's valid
  std::string command = aliases[raw_command];
  command_info_map::iterator it = info_map.find(command);
  if(it == info_map.end()) {
    Logger::fatal << "Unrecognized command: " << raw_command << Logger::endl;
    return -1;
  }

  // run the command and return its result
  command_info *ci = it->second;
  return ci->helper_fn(cmd);
}

CommandMap init_autolab_command_map() {

  command_info_map info_map;


  // Setup status command_info

  // We need to malloc this so that the memory isn't cleaned up after this
  // function finishes execution. Problem is, we now need a function to free
  // all of these.

  command_alias_map aliases;
  aliases["status"] = "status";
  aliases["download"] = "download";
  aliases["submit"] = "submit";
  aliases["courses"] = "courses";
  aliases["assessments"] = "assessments";
  aliases["asmts"] = "assessments";
  aliases["problems"] = "problems";
  aliases["scores"] = "scores";
  aliases["feedback"] = "feedback";
  aliases["enroll"] = "enroll";

  command_info *status_ci = new command_info;
  status_ci->usage = "status              Show status of the local assessment";
  status_ci->helper_fn = &show_status;
  status_ci->instructor_command = false;
  info_map["status"] = status_ci;

  // Setup download command_info
  command_info *download_ci = new command_info;
  download_ci->usage = "download            Download files needed for an assessment";
  download_ci->helper_fn = &download_asmt;
  download_ci->instructor_command = false;
  info_map["download"] = download_ci;

  // Setup submit command_info
  command_info *submit_ci = new command_info;
  submit_ci->usage = "submit              Submit a file to an assessment";
  submit_ci->helper_fn = &submit_asmt;
  submit_ci->instructor_command = false;
  info_map["submit"] = submit_ci;

  // Setup courses command_info
  command_info *courses_ci = new command_info;
  courses_ci->usage = "courses             List all courses";
  courses_ci->helper_fn = &show_courses;
  courses_ci->instructor_command = false;
  info_map["courses"] = courses_ci;

  // Setup assessments command_info
  command_info *assessments_ci = new command_info;
  assessments_ci->usage = "assessments/asmts   List all assessments of a course";
  assessments_ci->helper_fn = &show_assessments;
  assessments_ci->instructor_command = false;
  info_map["assessments"] = assessments_ci;

  // Setup problems command_info
  command_info *problems_ci = new command_info;
  problems_ci->usage = "problems            List all problems in an assessment";
  problems_ci->helper_fn = &show_problems;
  problems_ci->instructor_command = false;
  info_map["problems"] = problems_ci;

  // Setup scores command_info
  command_info *scores_ci = new command_info;
  scores_ci->usage = "scores              Show scores got on an assessment";
  scores_ci->helper_fn = &show_scores;
  scores_ci->instructor_command = false;
  info_map["scores"] = scores_ci;

  // Setup feedback command_info
  command_info *feedback_ci = new command_info;
  feedback_ci->usage = "feedback            Show feedback on a submission";
  feedback_ci->helper_fn = &show_feedback;
  feedback_ci->instructor_command = false;
  info_map["feedback"] = feedback_ci;

  // Setup scores command_info
  command_info *enroll_ci = new command_info;
  enroll_ci->usage = "enroll              Manage users affiliated with a course";
  enroll_ci->helper_fn = &manage_enrolls;
  enroll_ci->instructor_command = true;
  info_map["enroll"] = enroll_ci;

  CommandMap command_map = CommandMap();
  command_map.aliases = aliases;
  command_map.info_map = info_map;

  return command_map;
}
