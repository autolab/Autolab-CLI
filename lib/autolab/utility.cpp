#include "autolab/autolab.h"

#include <cstdio>
#include <ctime>

#include <chrono>
#include <sstream>
#include <iomanip>

#include "logger.h"

namespace Autolab {
  
namespace Utility {

// string conversion methods
// get time zone offset in hours (e.g. -4 means we're at UTC-04:00)
double get_timezone_offset() {
  std::time_t raw_time_utc;
  std::time(&raw_time_utc);

  std::tm *utc = std::gmtime(&raw_time_utc);
  std::time_t raw_time_local = std::mktime(utc);

  double diff_in_seconds = std::difftime(raw_time_utc, raw_time_local);
  return diff_in_seconds / 3600; // convert seconds to hours
}

std::time_t string_to_time(std::string str_time) {
  std::tm tms = {};
  int source_timezone_offset = 0;

  int res = std::sscanf(str_time.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d.%*3c%3d",
    &tms.tm_year, &tms.tm_mon, &tms.tm_mday,
    &tms.tm_hour, &tms.tm_min, &tms.tm_sec,
    &source_timezone_offset);

  if (res != 7 /* number of parsed args */) {
    Logger::debug << "string_to_time parse fail!" << Logger::endl;
  }

  // adjust tm struct according to spec
  // no information on whether daylight savings time is in effect
  tms.tm_isdst = -1;
  // adjust year to start from 1900
  tms.tm_year -= 1900;
  // adjust month to start with 0
  tms.tm_mon -= 1;
  
  // time zone conversion
  int net_timezone_offset = source_timezone_offset - get_timezone_offset();
  std::time_t raw_src_time = std::mktime(&tms);
  std::chrono::system_clock::time_point src_time =
      std::chrono::system_clock::from_time_t(raw_src_time);
  src_time -= std::chrono::hours(net_timezone_offset);
  std::time_t local_time = std::chrono::system_clock::to_time_t(src_time);

  Logger::debug << "Parsed time: " << std::ctime(&local_time) << Logger::endl;

  return local_time;
}

AuthorizationLevel string_to_authorization_level(std::string str_auth) {
  AuthorizationLevel auth = AuthorizationLevel::student;
  if (str_auth == "administrator") {
    auth = AuthorizationLevel::administrator;
  } else if (str_auth == "instructor") {
    auth = AuthorizationLevel::instructor;
  } else if (str_auth == "course_assistant") {
    auth = AuthorizationLevel::course_assistant;
  }
  return auth;
}

AttachmentFormat string_to_attachment_format(std::string str_format) {
  AttachmentFormat format = AttachmentFormat::none;
  if (str_format == "url") {
    format = AttachmentFormat::url;
  } else if (str_format == "file") {
    format = AttachmentFormat::file;
  }
  return format;
}

// comparators
bool compare_courses_by_name(const Course &a, const Course &b) {
  if (a.name == b.name) return a.semester < b.semester;
  return a.name < b.name;
}

bool compare_assessments_by_name(const Assessment &a, const Assessment &b) {
  if (a.name == b.name) return a.category_name < b.category_name;
  return a.name < b.name;
}

}
  
}