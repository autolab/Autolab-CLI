#ifndef AUTOLAB_H
#define AUTOLAB_H

#include <ctime>
#include <vector>

namespace Autolab {

  enum authorization {student,
                      course_assistant,
                      instructor,
                      administrator};

  enum attachment_format {none, url, file};

  struct Course {
    std::string name;
    std::string display_name;
    std::string semester;
    int late_slack;
    int grace_days;
    authorization auth_level;
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
    attachment_format writeup_format;
    attachment_format handout_format;
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

}

#endif /* AUTOLAB_H */