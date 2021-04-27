#include "autolab/client.h"

#include <cmath>

#include <map>
#include <vector>

#include <rapidjson/document.h>

#include "autolab/raw_client.h"
#include "json_helpers.h"
#include "logger.h"

namespace Autolab {

Client::Client(std::string domain, std::string client_id,
               std::string client_secret, std::string redirect_uri,
               void (*new_token_callback)(std::string, std::string))
  : raw_client(domain, client_id, client_secret, redirect_uri, new_token_callback) {}

void Client::set_tokens(std::string access_token, std::string refresh_token) {
  raw_client.set_tokens(access_token, refresh_token);
}

/* oauth-related */
void Client::device_flow_init(std::string &user_code, std::string &verification_uri) {
  raw_client.device_flow_init(user_code, verification_uri);
}

int Client::device_flow_authorize(size_t timeout) {
  return raw_client.device_flow_authorize(timeout);
}

/* custom utility */
void download_response_to_attachment_format(Attachment &attachment,
      rapidjson::Value &response) {
  if (response.IsObject()) {
    // check if it is 'none' or 'url'
    if (response.HasMember("url")) {
      attachment.format = AttachmentFormat::url;
      attachment.url = get_string_force(response, "url");
    } else {
      // it is none
      attachment.format = AttachmentFormat::none;
    }
  } else {
    attachment.format = AttachmentFormat::file;
  }
}

void check_for_error_response(rapidjson::Value &response) {
  if (response.IsObject() &&
      response.HasMember("error")) {
    std::string error_msg = get_string(response, "error");
    LogDebug("API returned error: " << error_msg << Logger::endl);
    throw ErrorResponseException(error_msg);
  }
}

/* packagers */
void user_from_json(User &user, rapidjson::Value &user_json) {
  user.first_name = get_string_force(user_json, "first_name");
  user.last_name  = get_string_force(user_json, "last_name");
  user.email      = get_string_force(user_json, "email");
  user.school     = get_string(user_json, "school");
  user.major      = get_string(user_json, "major");
  user.year       = get_string(user_json, "year");
}

void assessment_from_json(Assessment &asmt, rapidjson::Value &asmt_json) {
  asmt.name          = get_string_force(asmt_json, "name");
  asmt.display_name  = get_string(asmt_json, "display_name");
  asmt.category_name = get_string(asmt_json, "category_name");

  asmt.start_at = Utility::string_to_time(get_string_force(asmt_json, "start_at"));
  asmt.due_at   = Utility::string_to_time(get_string_force(asmt_json, "due_at"));
  asmt.end_at   = Utility::string_to_time(get_string_force(asmt_json, "end_at"));
  asmt.grading_deadline = Utility::string_to_time(get_string(asmt_json, "grading_deadline"));
}

void enrollment_from_json(Enrollment &enrollment, rapidjson::Value &enrollment_json) {
  enrollment.lecture = get_string(enrollment_json, "lecture");
  enrollment.section = get_string(enrollment_json, "section");
  enrollment.grade_policy = get_string(enrollment_json, "grade_policy");
  enrollment.nickname = get_string(enrollment_json, "nickname");
  enrollment.dropped = get_bool(enrollment_json, "dropped", false);
  enrollment.auth_level = Utility::string_to_authorization_level(
      get_string_force(enrollment_json, "auth_level"));
  user_from_json(enrollment.user, enrollment_json);
}

/* resource-related */
void Client::get_user_info(User &user) {
  rapidjson::Document user_info_doc;
  raw_client.get_user_info(user_info_doc);
  check_for_error_response(user_info_doc);

  require_is_object(user_info_doc);

  user_from_json(user, user_info_doc);
}

void Client::get_courses(std::vector<Course> &courses) {
  rapidjson::Document courses_doc;
  raw_client.get_courses(courses_doc);
  check_for_error_response(courses_doc);

  require_is_array(courses_doc);
  for (auto &c_doc : courses_doc.GetArray()) {
    Course course;
    course.name         = get_string_force(c_doc, "name");
    course.display_name = get_string(c_doc, "display_name");
    course.semester     = get_string(c_doc, "semester");
    course.late_slack   = get_int(c_doc, "late_slack", 0);
    course.grace_days   = get_int(c_doc, "grace_days", 0);
    course.auth_level   = Utility::string_to_authorization_level(
        get_string_force(c_doc, "auth_level"));

    courses.push_back(course);
  }
}

void Client::get_assessments(std::vector<Assessment> &asmts, const std::string &course_name) {
  rapidjson::Document asmts_doc;
  raw_client.get_assessments(asmts_doc, course_name);
  check_for_error_response(asmts_doc);

  require_is_array(asmts_doc);
  for (auto &a_doc : asmts_doc.GetArray()) {
    Assessment asmt;
    assessment_from_json(asmt, a_doc);

    asmts.push_back(asmt);
  }
}

void Client::get_assessment_details(DetailedAssessment &dasmt,
    const std::string &course_name, const std::string &asmt_name) {
  rapidjson::Document dasmt_doc;
  raw_client.get_assessment_details(dasmt_doc, course_name, asmt_name);
  check_for_error_response(dasmt_doc);

  require_is_object(dasmt_doc);

  assessment_from_json(dasmt.asmt, dasmt_doc);
  
  dasmt.description     = get_string(dasmt_doc, "description");
  dasmt.max_grace_days  = get_int(dasmt_doc, "max_grace_days", -1);
  dasmt.max_submissions = get_int(dasmt_doc, "max_submissions", -1);
  dasmt.max_unpenalized_submissions = get_int(dasmt_doc, "max_unpenalized_submissions", -1);
  dasmt.group_size      = get_int(dasmt_doc, "group_size", 1);
  dasmt.disable_handins = get_bool(dasmt_doc, "disable_handins", false);
  dasmt.has_scoreboard  = get_bool(dasmt_doc, "has_scoreboard", false);
  dasmt.has_autograder  = get_bool(dasmt_doc, "has_autograder", false);
  dasmt.handout_format  = Utility::string_to_attachment_format(
      get_string_force(dasmt_doc, "handout_format"));
  dasmt.writeup_format  = Utility::string_to_attachment_format(
      get_string_force(dasmt_doc, "writeup_format"));
}

void Client::get_problems(std::vector<Problem> &probs, const std::string &course_name,
    const std::string &asmt_name) {
  rapidjson::Document probs_doc;
  raw_client.get_problems(probs_doc, course_name, asmt_name);
  check_for_error_response(probs_doc);

  require_is_array(probs_doc);
  for (auto &p_doc : probs_doc.GetArray()) {
    Problem prob;
    prob.name        = get_string_force(p_doc, "name");
    prob.description = get_string(p_doc, "description");
    prob.max_score   = get_double(p_doc, "max_score");
    prob.optional    = get_bool(p_doc, "optional", false);

    probs.push_back(prob);
  }
}

void Client::get_submissions(std::vector<Submission> &subs, 
    const std::string &course_name, const std::string &asmt_name) {
  rapidjson::Document subs_doc;
  raw_client.get_submissions(subs_doc, course_name, asmt_name);
  check_for_error_response(subs_doc);

  require_is_array(subs_doc);
  for (auto &s_doc : subs_doc.GetArray()) {
    Submission sub;
    sub.version    = get_int_force(s_doc, "version");
    sub.created_at = Utility::string_to_time(get_string_force(s_doc, "created_at"));
    sub.filename   = get_string(s_doc, "filename");
    std::map<std::string, double> &scores = sub.scores;

    rapidjson::Value &scores_doc = s_doc["scores"];
    require_is_object(scores_doc);
    // iterate through members of the object
    for (auto &m : scores_doc.GetObject()) {
      const std::string &problem_name = m.name.GetString();
      double score;
      if (m.value.IsDouble()) {
        score = m.value.GetDouble();
      } else {
        score = std::nan(""); // unreleased
      }
      scores[problem_name] = score;
    }

    subs.push_back(sub);
  }
}

void Client::get_feedback(std::string &feedback, const std::string &course_name,
    const std::string &asmt_name, int sub_version, const std::string &problem_name) {
  rapidjson::Document feedback_doc;
  raw_client.get_feedback(feedback_doc, course_name, asmt_name, sub_version, problem_name);
  check_for_error_response(feedback_doc);

  require_is_object(feedback_doc);
  feedback = get_string_force(feedback_doc, "feedback");
}

void Client::get_enrollments(std::vector<Enrollment> &enrollments, const std::string &course_name) {
  rapidjson::Document enrolls_doc;
  raw_client.get_enrollments(enrolls_doc, course_name);
  check_for_error_response(enrolls_doc);

  require_is_array(enrolls_doc);
  for (auto &e_doc : enrolls_doc.GetArray()) {
    Enrollment enrollment;
    enrollment_from_json(enrollment, e_doc);

    enrollments.push_back(enrollment);
  }
}

void Client::crud_enrollment(Enrollment &result, const std::string &course_name,
    std::string email, EnrollmentOption &input, CrudAction action) {
  RawClient::Params in_params;

  if (action == Create || action == Update) {
    if (!input.lecture.NONE)
      in_params.push_back(std::make_pair("lecture", input.lecture.SOME));
    if (!input.section.NONE)
      in_params.push_back(std::make_pair("section", input.section.SOME));
    if (!input.grade_policy.NONE)
      in_params.push_back(std::make_pair("grade_policy", input.grade_policy.SOME));
    if (!input.nickname.NONE)
      in_params.push_back(std::make_pair("nickname", input.nickname.SOME));
    if (!input.dropped.NONE)
      in_params.push_back(std::make_pair("dropped",
          Utility::bool_to_string(input.dropped.SOME)));
    if (!input.auth_level.NONE)
      in_params.push_back(std::make_pair("auth_level",
          Utility::authorization_level_to_string(input.auth_level.SOME)));
  }

  rapidjson::Document enroll_doc;
  raw_client.crud_enrollment(enroll_doc, course_name, email, in_params, action);
  check_for_error_response(enroll_doc);

  require_is_object(enroll_doc);
  enrollment_from_json(result, enroll_doc);
}


void Client::download_handout(Attachment &handout, std::string download_dir,
    const std::string &course_name, const std::string &asmt_name) {
  rapidjson::Document response_doc;
  raw_client.download_handout(response_doc, download_dir, course_name, asmt_name);
  check_for_error_response(response_doc);

  download_response_to_attachment_format(handout, response_doc);
}

void Client::download_writeup(Attachment &writeup, std::string download_dir,
    const std::string &course_name, const std::string &asmt_name) {
  rapidjson::Document response_doc;
  raw_client.download_writeup(response_doc, download_dir, course_name, asmt_name);
  check_for_error_response(response_doc);

  download_response_to_attachment_format(writeup, response_doc);
}

int Client::submit_assessment(const std::string &course_name, const std::string &asmt_name,
      std::string filename) {
  rapidjson::Document response_doc;
  raw_client.submit_assessment(response_doc, course_name, asmt_name, filename);
  check_for_error_response(response_doc);

  require_is_object(response_doc);
  return get_int_force(response_doc, "version");
}

}