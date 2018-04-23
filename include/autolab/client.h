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
  Client(std::string domain, std::string client_id, std::string client_secret,
         std::string redirect_uri, void (*new_token_callback)(std::string, std::string));
  void set_tokens(std::string access_token, std::string refresh_token);

  /* oauth-related */
  void device_flow_init(std::string &user_code, std::string &verification_uri);
  int device_flow_authorize(size_t timeout);

  /* resource-related */
  void get_user_info(User &user);
  void get_courses(std::vector<Course> &courses);
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