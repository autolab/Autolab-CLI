/*
 * The raw client interface.
 *
 * The RawClient class abstracts away HTTP operations, but still allows the
 * users of the library to view the returned json themselves. This client should
 * be used when the user wishes to skip the response verification and packaging
 * step of the easy client.
 */

#ifndef LIBAUTOLAB_RAW_CLIENT_H_
#define LIBAUTOLAB_RAW_CLIENT_H_

#include <fstream>
#include <ostream>
#include <string>
#include <vector>

#include <curl/curl.h>
#include <rapidjson/document.h>

#include "autolab/autolab.h"
#include "autolab/multi_server.h"

namespace Autolab {

class RawClient {
public:
  RawClient();


  /* oauth-related */
  void set_auth_info_list(std::vector<AuthInfo> auth_info_list);
  bool has_auth_for_server(const std::string& server_name);
  void device_flow_init(std::string &user_code, std::string &verification_uri, std::string& device_code, const ServerInfo& server_info);
  int device_flow_authorize(size_t timeout, const std::string& device_code,
    const ServerInfo& server_info);

  // keeps track of state and config for the current request.
  struct request_state {
    bool file_upload;
    std::string upload_filename;

    bool is_download;
    std::string suggested_filename;
    std::string download_dir;
    std::string string_output;
    std::ofstream file_output;
    long response_code;

    request_state() :
      file_upload(false), is_download(false) {}
    request_state(std::string dir, std::string name_hint) :
      file_upload(false), is_download(false), suggested_filename(name_hint), 
      download_dir(dir) {}

    void reset() {
      is_download = false;
      string_output.clear();
    }

    void close_file_output() {
      if (file_output.is_open()) file_output.close();
    }

    bool consider_download() {
      return download_dir.length() > 0;
    }
  };

  typedef std::vector<std::pair<std::string, std::string>> Params;

  /* REST interface methods */
  void get_user_info(rapidjson::Document &result, const ServerInfo& server_info__);
  void get_courses(rapidjson::Document &result);
  void get_assessments(rapidjson::Document &result, const std::string &course_name);
  void get_assessment_details(rapidjson::Document &result, const std::string &course_name, const std::string &asmt_name);
  void get_problems(rapidjson::Document &result, const std::string &course_name, const std::string &asmt_name);
  void download_handout(rapidjson::Document &result, std::string download_dir, const std::string &course_name, const std::string &asmt_name);
  void download_writeup(rapidjson::Document &result, std::string download_dir, const std::string &course_name, const std::string &asmt_name);
  void submit_assessment(rapidjson::Document &result, const std::string &course_name, const std::string &asmt_name, std::string filename);
  void get_submissions(rapidjson::Document &result, const std::string &course_name, const std::string &asmt_name);
  void get_feedback(rapidjson::Document &result, const std::string &course_name, const std::string &asmt_name, int sub_version, const std::string &problem_name);
  void get_enrollments(rapidjson::Document &result, const std::string &course_name);
  void crud_enrollment(rapidjson::Document &result, const std::string &course_name, std::string email, Params &in_params, CrudAction action);

private:

  // initializes curl interface. Must be called before anything else.
  static int curl_ready;
  static int init_curl();

  // tokens-related

  enum HttpMethod {GET, POST, PUT, DELETE};
  HttpMethod crud_to_http(CrudAction action);

  // represents the parameters used in a HTTP request.
  struct request_param {
    std::string key;
    std::string value;
    char *escaped_value;

    request_param(std::string k, std::string v) :
      key(k), value(v), escaped_value(nullptr) {}
  };
  typedef std::vector<request_param> param_list;

  struct request_path_segment {
    std::string value;
    char *escaped_value;
    request_path_segment(std::string v) :
      value(v), escaped_value(nullptr) {}
  };
  typedef std::vector<request_path_segment> path_segments;

  std::string construct_path(CURL *curl, std::string base, RawClient::path_segments &path);
  void free_path(RawClient::path_segments &path);
  std::string construct_params(CURL *curl, param_list &params);
  void free_params(param_list &params);

  // private instance vars
  int api_version;

  AllAuthInfo all_auth_info;

  // perform HTTP request and return result, default method is GET.
  long raw_request(request_state *rstate, path_segments &path, param_list &params, const ServerInfo& server_info__, HttpMethod method);
  long raw_request_optional_refresh(request_state *rstate, path_segments &path, param_list &params, const ServerInfo& server_info__, HttpMethod method, bool refresh);
  long make_request(rapidjson::Document &response, path_segments &path, param_list &params, const ServerInfo& server_info__, HttpMethod method, bool refresh, 
    const std::string &download_dir, const std::string &suggested_filename, const std::string &upload_filename);

  void clear_device_flow_strings();

  bool save_tokens_from_response(rapidjson::Document &response, const std::string& server_name);
  bool get_token_from_authorization_code(std::string authorization_code, const ServerInfo& server_name);
  bool perform_token_refresh(const ServerInfo& server_info);

  bool document_has_error(request_state *rstate, const std::string &error_msg);
  void init_regular_path(path_segments &path);
  void init_regular_params(param_list &params, const ServerInfo& server_info__);
  void init_oauth_token_path(path_segments &path);
  void init_device_flow_init_path(path_segments &path);
  void init_device_flow_authorize_path(path_segments &path);
  void update_access_token_in_params(param_list &params, const ServerInfo& server_info__);
};

}

#endif /* LIBAUTOLAB_RAW_CLIENT_H_ */