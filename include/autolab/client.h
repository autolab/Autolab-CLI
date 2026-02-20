/*
 * The (easy) client interface.
 *
 * The Client class abstracts away HTTP operations and json parsing into
 * functions that return native types. Recommended for most use cases of the
 * autolab client library. (Uses RawClient internally).
 */

#ifndef LIBAUTOLAB_CLIENT_H_
#define LIBAUTOLAB_CLIENT_H_

#include <string>
#include <vector>

#include "autolab.h"
#include "raw_client.h"

namespace Autolab {

class Client {
private:
  RawClient raw_client;

public:
  /* setup-related */
  Client();

  /* oauth-related */
  void set_auth_info_list(std::vector<AuthInfo> auth_info_list);
  bool has_auth_for_server(const std::string& server_name);
  void device_flow_init(std::string &user_code, std::string &verification_uri, std::string& device_code, const ServerInfo& server_info);
  int device_flow_authorize(size_t timeout, const std::string& device_code, const ServerInfo& server_info);

  /* resource-related */
  void get_user_info(User &user, const ServerInfo& server_info__);
  void get_courses(std::vector<Course> &courses); // should get all courses across all servers
  void get_assessments(std::vector<Assessment> &asmts, const std::string &course_name);
  void get_assessment_details(DetailedAssessment &dasmt, const std::string &course_name, const std::string &asmt_name);
  void get_problems(std::vector<Problem> &probs, const std::string &course_name, const std::string &asmt_name);
  void get_submissions(std::vector<Submission> &subs, const std::string &course_name, const std::string &asmt_name);
  void get_feedback(std::string &feedback, const std::string &course_name, const std::string &asmt_name, int sub_version, const std::string &problem_name);

  void get_enrollments(std::vector<Enrollment> &enrollments, const std::string &course_name);
  void crud_enrollment(Enrollment &result, const std::string &course_name, std::string email, EnrollmentOption &input, CrudAction action);

  /* action-related */
  void download_handout(Attachment &handout, std::string download_dir, const std::string &course_name, const std::string &asmt_name);
  void download_writeup(Attachment &writeup, std::string download_dir, const std::string &course_name, const std::string &asmt_name);
  // returns the new submission version number on success
  int submit_assessment(const std::string &course_name, const std::string &asmt_name, std::string filename);
};

}

#endif /* LIBAUTOLAB_CLIENT_H_ */