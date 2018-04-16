#include "logger.h"

#include "cmdimp.h"
#include "cmdmap.h"

int CommandMap::exec_command(cmdargs &cmd, std::string raw_command) {
  // Translate to default command name and check if it's valid
  std::string command = aliases[raw_command];
  command_info_map::iterator it = info_map.find(command);
  if(it == info_map.end()) {
    Logger::fatal << "Unrecognized command: " << raw_command << Logger::endl;
    return -1;
  }

  // run the command and return its result
  command_info ci = it->second;
  return ci.helper_fn(cmd);
}

CommandMap init_autolab_command_map() {
  command_alias_map aliases;
  aliases["status"] = "status";
  aliases["download"] = "download";
  aliases["submit"] = "submit";
  aliases["courses"] = "courses";
  aliases["assessments"] = "assessments";
  aliases["asmts"] = "assessments";
  aliases["problems"] = "problems";
  aliases["scores"] = "scores";
  aliases["submissions"] = "scores";
  aliases["feedback"] = "feedback";
  aliases["enroll"] = "enroll";

  command_info_map info_map {
    // general commands
    {"status",     {"status              Show status of the local assessment",     &show_status,      false}},
    {"download",   {"download            Download files needed for an assessment", &download_asmt,    false}},
    {"submit",     {"submit              Submit a file to an assessment",          &submit_asmt,      false}},
    {"courses",    {"courses             List all courses",                        &show_courses,     false}},
    {"assessments",{"assessments/asmts   List all assessments of a course",        &show_assessments, false}},
    {"problems",   {"problems            List all problems in an assessment",      &show_problems,    false}},
    {"scores",     {"scores/submissions  Show scores got on an assessment",        &show_scores,      false}},
    {"feedback",   {"feedback            Show feedback on a submission",           &show_feedback,    false}},
    // instructor commands
    {"enroll",     {"enroll              Manage users affiliated with a course",   &manage_enrolls,   true}}
  };

  CommandMap command_map = CommandMap();
  command_map.aliases = aliases;
  command_map.info_map = info_map;

  return command_map;
}
