#ifndef LIBAUTOLAB_AUTOLAB_H_
#define LIBAUTOLAB_AUTOLAB_H_

#include <ctime>

#include <map>
#include <string>
#include <vector>

namespace Autolab {

/* resources */
enum AuthorizationLevel {student,
                         course_assistant,
                         instructor,
                         administrator};

enum AttachmentFormat {none, url, file};

struct Attachment {
  AttachmentFormat format;
  std::string url;
};

struct Course {
  std::string name;
  std::string display_name;
  std::string semester;
  int late_slack;
  int grace_days;
  AuthorizationLevel auth_level;
};

struct Assessment {
  std::string name;
  std::string display_name;
  std::string category_name;
  std::time_t start_at;
  std::time_t due_at;
  std::time_t end_at;
  std::time_t grading_deadline;
};

struct DetailedAssessment {
  Assessment asmt;
  std::string description;
  int max_grace_days;
  int max_submissions;
  int group_size;
  bool disable_handins;
  bool has_scoreboard;
  bool has_autograder;
  AttachmentFormat handout_format;
  AttachmentFormat writeup_format;
};

struct Problem {
  std::string name;
  std::string description;
  double max_score;
  bool optional;
};

struct Submission {
  int version;
  std::time_t created_at;
  std::string filename;
  // maps problem name to the score (a NaN score means it's unreleased).
  std::map<std::string, double> scores;
};

struct User {
  std::string first_name;
  std::string last_name;
  std::string email;
  std::string school;
  std::string major;
  std::string year;
};

/* exceptions */

// Indicates an error that occurred in HTTP operations.
class HTTPException: public std::exception {
private:
  std::string msg;
public:
  explicit HTTPException(std::string m) : msg(m) {}
  virtual const char* what() const throw() {
      return msg.c_str();
  }
};

// Indicates the current access token and refresh token both do not work.
// A new set of tokens should be acquired by re-preforming user authorization.
class InvalidTokenException: public std::exception {
public:
  virtual const char* what() const throw() {
      return "The provided access token is invalid and the refresh operation failed.";
  }
};

// Indicates the client failed to receive the data it expected.
// This likely indicates a version mismatch between the client and the server.
class InvalidResponseException: public std::exception {
private:
  std::string msg;
public:
  explicit InvalidResponseException(std::string m) : msg(m) {}
  virtual const char* what() const throw() {
      return msg.c_str();
  }
};

// Indicates the server could not fulfill the request for some reason.
// This exception's msg will contain the error message returned from the API
// server. It is intended to be shown directly to the user so they could decide
// how to proceed. The application is not expected to parse and understand it.
class ErrorResponseException: public std::exception {
private:
  std::string msg;
public:
  explicit ErrorResponseException(std::string m) : msg(m) {}
  virtual const char* what() const throw() {
      return msg.c_str();
  }
};

namespace Utility {
// string conversions
std::time_t string_to_time(std::string str);
AuthorizationLevel string_to_authorization_level(std::string str);
AttachmentFormat string_to_attachment_format(std::string str_format);

// comparators
bool compare_courses_by_name(const Course &a, const Course &b);
bool compare_assessments_by_name(const Assessment &a, const Assessment &b);
}

}

#endif /* LIBAUTOLAB_AUTOLAB_H_ */