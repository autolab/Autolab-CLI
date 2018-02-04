#ifndef AUTOLAB_H
#define AUTOLAB_H

#include <ctime>
#include <vector>

namespace Autolab {

  /* resource packages */
  enum authorization {student,
                      course_assistant,
                      instructor,
                      administrator};

  enum attachment_format {none, url, file};

  struct Attachment {
    attachment_format format;
    std::string url;
  };

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

  struct User {
    std::string first_name;
    std::string last_name;
    std::string email;
    std::string school;
    std::string major;
    std::string year;
  };

  /* classes and methods */
  class Client {
  public:
    /* setup-related */
    Client(std::string client_id, std::string client_secret, std::string redirect_uri,
           void (*new_token_callback)(std::string, std::string));
    void SetTokens(std::string access_token, std::string refresh_token);

    /* oauth-related */
    void DeviceFlowInit(std::string &user_code, std::string &verification_uri);
    int DeviceFlowAuthorize(size_t timeout);

    /* resource-related */
    void GetUserInfo(User &user);
    void GetCourses(Course &course);
    void GetAssessments(std::vector<Assessment> &asmts, std::string course_name);
    void GetAssessmentDetails(DetailedAssessment &asmt, std::string course_name, std::string asmt_name);
    void GetProblems(std::vector<Problem> &probs, std::string course_name, std::string asmt_name);
    void GetSubmissions(std::vector<Submission> &subs, std::string course_name, std::string asmt_name);
    
    /* action-related */
    std::string GetFeedback(std::string course_name, std::string asmt_name, int sub_version, std::string problem_name);
    void DownloadHandout(Attachment &handout, std::string download_dir, std::string course_name, std::string asmt_name);
    void DownloadWriteup(Attachment &writeup, std::string download_dir, std::string course_name, std::string asmt_name);
    // returns the new submission version number on success
    int SubmitAssessment(std::string course_name, std::string asmt_name, std::string filename);
  };

}

#endif /* AUTOLAB_H */