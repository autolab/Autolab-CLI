#include <chrono>
#include <thread> // sleep_for
#include <sstream>

#include "autolab_client.h"
#include "logger.h"

const std::chrono::seconds device_flow_authorize_wait_duration(5);

const std::string base_uri = "http://localhost:3000";

const std::string oauth_token_path = "/oauth/token";
const std::string device_flow_init_path = "/oauth/device_flow_init";
const std::string device_flow_authorize_path = "/oauth/device_flow_authorize";

/* initialization */
int AutolabClient::curl_ready = false;

AutolabClient::AutolabClient(const std::string &id, const std::string &st, 
  const std::string &ru, void (*tk_cb)(std::string, std::string)) :
  client_id(id), client_secret(st), redirect_uri(ru), api_version(1),
  new_tokens_callback(tk_cb)
{
  AutolabClient::init_curl();
}

int AutolabClient::init_curl() {
  if (AutolabClient::curl_ready) return 0;

  CURLcode err = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (err) return -1;

  AutolabClient::curl_ready = true;
  return 0;
}

// set access_token and refresh_token
void AutolabClient::set_tokens(std::string at, std::string rt) {
  access_token = at;
  refresh_token = rt;
}

// set the function that should be called when tokens are refreshed

/* Basic request helper */


// libcurl header callback function
size_t header_callback(char *data, size_t size, size_t nmemb, 
                  AutolabClient::request_state *rstate) {
  if (!data) return 0;

  if (rstate->consider_download()) {
    // find out if this is supposed to be a download
    // and if so, find out the filename
    std::string header_str(data, size*nmemb);
    std::string::size_type name_start, name_end;

    if (header_str.find("Content-Disposition:") != std::string::npos) {
      rstate->is_download = true;
      Logger::debug << header_str << Logger::endl;
      // look for filename
      name_start = header_str.find("filename=");
      if (name_start != std::string::npos) {
        name_start += 10; // skip 'filename=' and the following quote
        name_end = header_str.find('"', name_start);
        Logger::debug << "  name_start: " << name_start << ", name_end: " << name_end << Logger::endl;
        if (name_end != std::string::npos) {
          rstate->suggested_filename = header_str.substr(name_start, name_end - name_start);
          Logger::debug << "  suggested filename: " << rstate->suggested_filename << Logger::endl;
        }
      }
      // open ofstream for writing output
      std::string full_filename = rstate->download_dir + "/" + rstate->suggested_filename;
      rstate->file_output.open(full_filename.c_str(), std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
      Logger::debug << "Opened file " << full_filename << Logger::endl;
    }
  }

  return size*nmemb;
}

// libcurl write callback function
static size_t write_callback(char *data, size_t size, size_t nmemb,
                  AutolabClient::request_state *rstate) {
  if (!data) return 0;

  if (rstate->is_download) {
    rstate->file_output.write(data, size*nmemb);
  } else {
    rstate->string_output.append(data, size*nmemb);
  }

  return size*nmemb;
}

void setup_multipart_form(CURL *curl, std::string filename) {
  curl_mimepart *field = NULL;
  /* Create the form */ 
  curl_mime *form = curl_mime_init(curl);

  /* Fill in the file upload field */ 
  field = curl_mime_addpart(form);
  curl_mime_name(field, "submission[file]");
  curl_mime_filedata(field, filename.c_str());

  curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
}

std::string AutolabClient::construct_params(CURL *curl, AutolabClient::param_list &params) {
  std::string result;
  size_t length = params.size();
  for (size_t i = 0; i < length; i++) {
    result.append(params[i].key + "=");
    params[i].escaped_value = curl_easy_escape(curl, params[i].value.c_str(), params[i].value.length());
    result.append(params[i].escaped_value);
    if (i < length - 1) result.append("&");
  }
  return result;
}

void AutolabClient::free_params(AutolabClient::param_list &params) {
  for (auto item : params) {
    curl_free(item.escaped_value);
  }
}

/* actually perform the HTTP request using libcurl.
 */
long AutolabClient::raw_request(AutolabClient::request_state *rstate,
  const std::string &path, AutolabClient::param_list &params,
  AutolabClient::HttpMethod method = GET)
{
  CURL *curl;
  CURLcode res;
  curl_mime *form = nullptr;
  curl_mimepart *field = nullptr;

  curl = curl_easy_init();
  err_assert(curl, "Error init-ing easy interface");

  std::string full_path(base_uri);
  full_path.append(path);

  std::string param_str = construct_params(curl, params);

  Logger::debug << "Requesting " << path << " with params " << param_str << Logger::endl << Logger::endl;

  if (method == GET) { // GET
    full_path.append("?" + param_str);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
  } else {             // POST
    if (rstate->file_upload) {
      // setup form
      form = curl_mime_init(curl);
      field = curl_mime_addpart(form);
      curl_mime_name(field, "submission[file]");
      curl_mime_filedata(field, rstate->upload_filename.c_str());
      // insert form
      curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
      // add params
      full_path.append("?" + param_str);
    } else {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, param_str.c_str());
    }
  }

  curl_easy_setopt(curl, CURLOPT_URL, full_path.c_str());

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, rstate);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, rstate);

  res = curl_easy_perform(curl);
  err_assert(res == CURLE_OK, curl_easy_strerror(res));

  long response_code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  rstate->response_code = response_code;

  // free resources
  free_params(params);
  if (form) curl_mime_free(form);

  curl_easy_cleanup(curl);

  return response_code;
}

bool AutolabClient::document_has_error(AutolabClient::request_state *rstate, 
  const std::string &error_msg)
{
  if (rstate->is_download) return false;
  rapidjson::Document response;
  response.Parse(rstate->string_output.c_str());
  return response.IsObject() &&
         response.HasMember("error") &&
         response["error"].IsString() &&
         response["error"].GetString() == error_msg;
  return false;
}

/* performs raw_request, and if error is authorization_failed, refresh tokens
 * and try again.
 */
const std::string oauth_auth_failed_response = "OAuth2 authorization failed";
long AutolabClient::raw_request_optional_refresh(
  AutolabClient::request_state *rstate, 
  const std::string &path, AutolabClient::param_list &params, 
  AutolabClient::HttpMethod method = GET, bool refresh = true)
{
  long rc = raw_request(rstate, path, params, method);
  if (!refresh) return rc;

  if (rc == 200 || !document_has_error(rstate, oauth_auth_failed_response)) {
    return rc;
  }

  if (perform_token_refresh()) {
    rstate->reset();
    update_access_token_in_params(params);
    rc = raw_request(rstate, path, params, method);
    if (rc == 200 || !document_has_error(rstate, oauth_auth_failed_response)) {
      // all good now
      Logger::debug << "Successfully refreshed token" << Logger::endl;
      return rc;
    }
  }

  throw InvalidTokenException();
}

/* make a HTTP request
 *
 * params:
 *   - response: stores the JSON response
 *   - path:     HTTP request path
 *   - params:   HTTP request params
 *   - method:   HTTP request method
 *   - refresh:  if true, automatically calls perform_token_refresh when the
 *               request fails the first time, and retries the request.
 *   - download_dir: the directory to download the file if the response is a
 *                   file download. (full path)
 *   - suggested_filename: the default filename if the server didn't provide
 *                         the filename for the downloaded file. (name only)
 *   - upload_filename: the name of the file to upload. (relative path)
 */
long AutolabClient::make_request(rapidjson::Document &response,
  const std::string &path, AutolabClient::param_list &params,
  AutolabClient::HttpMethod method = GET, bool refresh = true,
  const std::string &download_dir = "",
  const std::string &suggested_filename = "",
  const std::string &upload_filename = "")
{
  AutolabClient::request_state rstate(download_dir, suggested_filename);
  if (upload_filename.length() > 0) {
    rstate.upload_filename = upload_filename;
    rstate.file_upload = true;
  }

  long rc = raw_request_optional_refresh(&rstate, path, params, method, refresh);

  Logger::debug << "Completed make request" << Logger::endl;

  rstate.close_file_output();
  if (!rstate.is_download) {
    Logger::debug << rstate.string_output << Logger::endl;
    response.Parse(rstate.string_output.c_str());
  }

  return rc;
}

/* Authorization (device-flow) & Authentication */

void AutolabClient::device_flow_init(std::string &user_code, std::string &verification_uri) {
  // make a local copy and start building params
  AutolabClient::param_list params;
  params.emplace_back("client_id", client_id);

  rapidjson::Document response;
  make_request(response, device_flow_init_path, params, GET, false);

  err_assert(response.HasMember("device_code") && response.HasMember("user_code"),
    "Expected keys not found in response during device_flow_init");

  device_flow_device_code = response["device_code"].GetString();
  user_code = response["user_code"].GetString();
  verification_uri = response["verification_uri"].GetString();
}

void AutolabClient::clear_device_flow_strings() {
  device_flow_device_code.clear();
  device_flow_user_code.clear();
}

/* 
 * param: timeout in seconds
 * response:  1 - denied by user
 *            0 - granted by user
 *           -1 - error, device_flow_init not called
 *           -2 - timed out
 */          
int AutolabClient::device_flow_authorize(size_t timeout) {
  if (!device_flow_device_code.length()) {
    // device flow init not called
    return -1;
  }

  AutolabClient::param_list params;
  params.emplace_back("client_id", client_id);
  params.emplace_back("device_code", device_flow_device_code);

  rapidjson::Document response;

  // get the current time
  auto t_now = std::chrono::steady_clock::now();
  auto t_end = t_now + std::chrono::seconds(timeout);
  // find out end time

  while (t_now < t_end) {
    make_request(response, device_flow_authorize_path, params, GET, false);
    if (response.HasMember("code")) {
      // success!
      std::string code = response["code"].GetString();
      get_token_from_authorization_code(code);
      clear_device_flow_strings();
      return 0;
    }

    // There must be an error field then
    std::string error_string = response["error"].GetString();
    if (error_string != "authorization_pending") {
      // denied!
      clear_device_flow_strings();
      return 1;
    }

    // wait for next request
    std::this_thread::sleep_for(device_flow_authorize_wait_duration);

    t_now = std::chrono::steady_clock::now();
  }

  // timed out, user could call again
  return -2;
}

bool AutolabClient::save_tokens_from_response(rapidjson::Document &response) {
  if (response.HasMember("access_token") && response.HasMember("refresh_token")) {
    // looks good
    access_token = response["access_token"].GetString();
    refresh_token = response["refresh_token"].GetString();
    if (new_tokens_callback) {
      new_tokens_callback(access_token, refresh_token);
    }
    return true;
  }
  return false;
}

bool AutolabClient::get_token_from_authorization_code(std::string authorization_code) {
  AutolabClient::param_list params;
  params.emplace_back("grant_type", "authorization_code");
  params.emplace_back("client_id", client_id);
  params.emplace_back("client_secret", client_secret);
  params.emplace_back("redirect_uri", redirect_uri);
  params.emplace_back("code", authorization_code);

  rapidjson::Document response;
  make_request(response, oauth_token_path, params, POST, false);

  return save_tokens_from_response(response);
}

bool AutolabClient::perform_token_refresh() {
  AutolabClient::param_list params;
  params.emplace_back("grant_type", "refresh_token");
  params.emplace_back("client_id", client_id);
  params.emplace_back("client_secret", client_secret);
  params.emplace_back("refresh_token", refresh_token);

  rapidjson::Document response;
  make_request(response, oauth_token_path, params, POST, false);

  return save_tokens_from_response(response);
}

/* REST Interface wrappers */
void AutolabClient::init_regular_path(std::string &path) {
  path.clear();
  path.append("/api/v" + std::to_string(api_version));
}

void AutolabClient::init_regular_params(AutolabClient::param_list &params) {
  params.clear();
  params.emplace_back("access_token", access_token);
}

void AutolabClient::update_access_token_in_params(AutolabClient::param_list &params) {
  for (auto &param : params) {
    if (param.key == "access_token") {
      param.value = access_token;
      break;
    }
  }
}

void AutolabClient::get_user_info(rapidjson::Document &result) {
  std::string path;
  init_regular_path(path);
  path.append("/user");

  AutolabClient::param_list params;
  init_regular_params(params);

  make_request(result, path, params);
}

void AutolabClient::get_courses(rapidjson::Document &result) {
  std::string path;
  init_regular_path(path);
  path.append("/courses");

  AutolabClient::param_list params;
  init_regular_params(params);
  params.emplace_back("state", "current");

  make_request(result, path, params);
}

void AutolabClient::get_assessments(rapidjson::Document &result, std::string course_name) {
  std::string path;
  init_regular_path(path);
  path.append("/courses/" + course_name + "/assessments");

  AutolabClient::param_list params;
  init_regular_params(params);

  make_request(result, path, params);
}

void AutolabClient::get_assessment_details(rapidjson::Document &result, std::string course_name, std::string asmt_name) {
  std::string path;
  init_regular_path(path);
  path.append("/courses/" + course_name + "/assessments/" + asmt_name);

  AutolabClient::param_list params;
  init_regular_params(params);

  make_request(result, path, params);
}

void AutolabClient::get_problems(rapidjson::Document &result, std::string course_name, std::string asmt_name) {
  std::string path;
  init_regular_path(path);
  path.append("/courses/" + course_name + "/assessments/" + asmt_name + "/problems");

  AutolabClient::param_list params;
  init_regular_params(params);

  make_request(result, path, params);
}

void AutolabClient::download_handout(rapidjson::Document &result, std::string download_dir, std::string course_name, std::string asmt_name) {
  std::string path;
  init_regular_path(path);
  path.append("/courses/" + course_name + "/assessments/" + asmt_name + "/handout");

  AutolabClient::param_list params;
  init_regular_params(params);

  make_request(result, path, params, GET, true, download_dir, "handout");
}

void AutolabClient::download_writeup(rapidjson::Document &result, std::string download_dir, std::string course_name, std::string asmt_name) {
  std::string path;
  init_regular_path(path);
  path.append("/courses/" + course_name + "/assessments/" + asmt_name + "/writeup");

  AutolabClient::param_list params;
  init_regular_params(params);

  make_request(result, path, params, GET, true, download_dir, "writeup");
}

void AutolabClient::submit_assessment(rapidjson::Document &result, std::string course_name, std::string asmt_name, std::string filename) {
  std::string path;
  init_regular_path(path);
  path.append("/courses/" + course_name + "/assessments/" + asmt_name + "/submit");

  AutolabClient::param_list params;
  init_regular_params(params);

  make_request(result, path, params, POST, true, "", "", filename);
}