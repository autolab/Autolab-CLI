#ifndef LIBAUTOLAB_CLIENT_H_
#define LIBAUTOLAB_CLIENT_H_

#include "autolab.h"

#include <string>
#include <vector>

namespace Autolab {

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

#endif /* LIBAUTOLAB_CLIENT_H_ */