#ifndef LIBAUTOLAB_AUTOLAB_H_
#define LIBAUTOLAB_AUTOLAB_H_

#include <time.h>

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
  time_t start_at;
  time_t due_at;
  time_t end_at;
  time_t grading_deadline;
};

struct DetailedAssessment {
  Assessment brief;
  std::string description;
  int max_grace_days;
  int max_submissions;
  int group_size;
  bool disable_handins;
  bool has_scoreboard;
  bool has_autograder;
  AttachmentFormat writeup_format;
  AttachmentFormat handout_format;
};

struct Problem {
  std::string name;
  std::string description;
  double max_score;
  bool optional;
};

struct Score {
  std::string problem_name;
  double score; /* NaN means unreleased */
};

struct Submission {
  int version;
  time_t created_at;
  std::string filename;
  std::vector<Score> scores;
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

}

#endif /* LIBAUTOLAB_AUTOLAB_H_ */