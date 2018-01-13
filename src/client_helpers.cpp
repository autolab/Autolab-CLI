#include "client_helpers.h"
#include "logger.h"

/* find course using name_hint
 */
bool find_course(AutolabClient &ac, std::string &name_hint) {
  std::string course_name;
  rapidjson::Document courses;
  ac.get_courses(courses);
  check_error_and_exit(courses);

  Logger::debug << "Found " << courses.Size() << " current courses." << Logger::endl;
  bool found = false;
  for (auto &c : courses.GetArray()) {
    course_name = c.GetObject()["name"].GetString();
    if (course_name == name_hint) {
      return true;
    }
  }

  return false;
}

/* find assessment in course using name_hint
 */
bool find_assessment(AutolabClient &ac, const std::string &course_name, std::string name_hint) {
  std::string asmt_name;
  rapidjson::Document asmts;
  ac.get_assessments(asmts, course_name);
  check_error_and_exit(asmts);

  Logger::info << "Found " << asmts.Size() << " assessments." << Logger::endl;
  bool found = false;
  for (auto &item : asmts.GetArray()) {
    asmt_name = item.GetObject()["name"].GetString();
    if (asmt_name == name_hint) {
      return true;
    }
  }

  return false;
}