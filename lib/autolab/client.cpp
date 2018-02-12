#include "autolab/client.h"

#include <limits>
#include <map>

#include <rapidjson/document.h>

#include "autolab/raw_client.h"
#include "json_helpers.h"
#include "logger.h"

namespace Autolab {

Client::Client(std::string client_id, std::string client_secret, 
               std::string redirect_uri,
               void (*new_token_callback)(std::string, std::string))
  : raw_client(client_id, client_secret, redirect_uri, new_token_callback) {}

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
    Logger::debug << "API returned error: " << error_msg << Logger::endl;
    throw ErrorResponseException(error_msg);
  }
}

/* resource-related */
void Client::get_user_info(User &user) {
  rapidjson::Document user_info_doc;
  raw_client.get_user_info(user_info_doc);
  check_for_error_response(user_info_doc);

  require_is_object(user_info_doc);

  user.first_name = get_string_force(user_info_doc, "first_name");
  user.last_name  = get_string_force(user_info_doc, "last_name");
  user.email      = get_string_force(user_info_doc, "email");
  user.school     = get_string(user_info_doc, "school");
  user.major      = get_string(user_info_doc, "major");
  user.year       = get_string(user_info_doc, "year");
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
    course.late_slack   = get_int_force(c_doc, "late_slack");
    course.grace_days   = get_int_force(c_doc, "grace_days");
    course.auth_level   = Utility::string_to_authorization_level(
        get_string_force(c_doc, "auth_level"));

    courses.push_back(course);
  }
}

void Client::get_assessments(std::vector<Assessment> &asmts, std::string course_name) {
  rapidjson::Document asmts_doc;
  raw_client.get_assessments(asmts_doc, course_name);
  check_for_error_response(asmts_doc);

  require_is_array(asmts_doc);
  for (auto &a_doc : asmts_doc.GetArray()) {
    Assessment asmt;
    asmt.name          = get_string_force(a_doc, "name");
    asmt.display_name  = get_string(a_doc, "display_name");
    asmt.category_name = get_string(a_doc, "category_name");
    asmt.start_at      = Utility::string_to_time(get_string_force(a_doc, "start_at"));
    asmt.due_at        = Utility::string_to_time(get_string_force(a_doc, "due_at"));
    asmt.end_at        = Utility::string_to_time(get_string_force(a_doc, "end_at"));
    asmt.grading_deadline = Utility::string_to_time(get_string(a_doc, "grading_deadline"));
    
    asmts.push_back(asmt);
  }
}

void Client::get_assessment_details(DetailedAssessment &dasmt,
    std::string course_name, std::string asmt_name) {
  rapidjson::Document dasmt_doc;
  raw_client.get_assessments(dasmt_doc, course_name);
  check_for_error_response(dasmt_doc);

  require_is_object(dasmt_doc);

  Assessment &asmt = dasmt.asmt;
  asmt.name             = get_string_force(dasmt_doc, "name");
  asmt.display_name     = get_string(dasmt_doc, "display_name");
  asmt.category_name    = get_string(dasmt_doc, "category_name");
  asmt.start_at         = Utility::string_to_time(get_string_force(dasmt_doc, "start_at"));
  asmt.due_at           = Utility::string_to_time(get_string_force(dasmt_doc, "due_at"));
  asmt.end_at           = Utility::string_to_time(get_string_force(dasmt_doc, "end_at"));
  asmt.grading_deadline = Utility::string_to_time(get_string(dasmt_doc, "grading_deadline"));
  
  dasmt.description     = get_string(dasmt_doc, "description");
  dasmt.max_grace_days  = get_int_force(dasmt_doc, "max_grace_days");
  dasmt.max_submissions = get_int_force(dasmt_doc, "max_submissions");
  dasmt.group_size      = get_int_force(dasmt_doc, "group_size");
  dasmt.disable_handins = get_bool_force(dasmt_doc, "disable_handins");
  dasmt.has_scoreboard  = get_bool_force(dasmt_doc, "has_scoreboard");
  dasmt.has_autograder  = get_bool_force(dasmt_doc, "has_autograder");
  dasmt.handout_format  = Utility::string_to_attachment_format(
      get_string_force(dasmt_doc, "handout_format"));
  dasmt.writeup_format  = Utility::string_to_attachment_format(
      get_string_force(dasmt_doc, "writeup_format"));
}

void Client::get_problems(std::vector<Problem> &probs, std::string course_name,
    std::string asmt_name) {
  rapidjson::Document probs_doc;
  raw_client.get_problems(probs_doc, course_name, asmt_name);
  check_for_error_response(probs_doc);

  require_is_array(probs_doc);
  for (auto &p_doc : probs_doc.GetArray()) {
    Problem prob;
    prob.name        = get_string_force(p_doc, "name");
    prob.description = get_string(p_doc, "description");
    prob.max_score   = get_double_force(p_doc, "max_score");
    prob.optional    = get_bool_force(p_doc, "optional");

    probs.push_back(prob);
  }
}

void Client::get_submissions(std::vector<Submission> &subs, 
    std::string course_name, std::string asmt_name) {
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
      std::string problem_name = m.name.GetString();
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

std::string Client::get_feedback(std::string course_name, std::string asmt_name,
    int sub_version, std::string problem_name) {
  rapidjson::Document feedback_doc;
  raw_client.get_feedback(feedback_doc, course_name, asmt_name, sub_version, problem_name);
  check_for_error_response(feedback_doc);

  require_is_object(feedback_doc);
  return get_string_force(feedback_doc, "feedback");
}

void Client::download_handout(Attachment &handout, std::string download_dir,
    std::string course_name, std::string asmt_name) {
  rapidjson::Document response_doc;
  raw_client.download_handout(response_doc, download_dir, course_name, asmt_name);
  check_for_error_response(response_doc);

  download_response_to_attachment_format(handout, response_doc);
}

void Client::download_writeup(Attachment &writeup, std::string download_dir,
    std::string course_name, std::string asmt_name) {
  rapidjson::Document response_doc;
  raw_client.download_writeup(response_doc, download_dir, course_name, asmt_name);
  check_for_error_response(response_doc);

  download_response_to_attachment_format(writeup, response_doc);
}

int Client::submit_assessment(std::string course_name, std::string asmt_name,
      std::string filename) {
  rapidjson::Document response_doc;
  raw_client.submit_assessment(response_doc, course_name, asmt_name, filename);
  check_for_error_response(response_doc);

  require_is_object(response_doc);
  return get_int_force(response_doc, "version");
}

}