#ifndef LIBAUTOLAB_AUTOLAB_H_
#define LIBAUTOLAB_AUTOLAB_H_

#include <ctime>
#include <string>
#include <vector>

namespace Autolab {

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

}

#endif /* LIBAUTOLAB_AUTOLAB_H_ */