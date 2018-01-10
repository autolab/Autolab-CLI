#include <chrono>
#include <thread> // sleep_for

#include <curl/curl.h>

#include "autolab_client.h"
#include "logger.h"

const std::chrono::seconds device_flow_authorize_wait_duration(5);

const std::string base_uri = "http://localhost:3000";

const std::string oauth_token_path = "/oauth/token";
const std::string device_flow_init_path = "/oauth/device_flow_init";
const std::string device_flow_authorize_path = "/oauth/device_flow_authorize";

/* initialization */
int AutolabClient::curl_ready = false;

AutolabClient::AutolabClient(std::string &id, std::string &st, std::string &ru) {
  AutolabClient::init_curl();
  client_id = id;
  client_secret = st;
  redirect_uri = ru;
}

int AutolabClient::init_curl() {
  if (AutolabClient::curl_ready) return 0;

  CURLcode err = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (err) return -1;

  AutolabClient::curl_ready = true;
  return 0;
}

// set access_token and refresh_token
int AutolabClient::set_tokens(std::string at, std::string rt) {
  access_token = at;
  refresh_token = rt;
}

/* Basic request helper */

// libcurl write callback function
static int write_callback(char *data, size_t size, size_t nmemb,
                  std::string *userdata) {
  if (!data) return 0;
  userdata->append(data, size*nmemb);
  return size*nmemb;
}

// TODO: make post version using CURLOPT_POSTFIELDS
int AutolabClient::make_request(rapidjson::Document &response, const std::string &path, const std::string &params, bool is_post = false) {
  std::string buffer;

  CURL *curl;
  CURLcode res;
  struct curl_slist *header = NULL;

  curl = curl_easy_init();
  err_assert(curl, "Error init-ing easy interface");

  std::string full_path(base_uri);
  full_path.append(path);

  if (is_post) {
    curl_easy_setopt(curl, CURLOPT_URL, full_path.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
  } else {
    // default to GET
    // append params to query string
    full_path.append("?" + params);
    curl_easy_setopt(curl, CURLOPT_URL, full_path.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
  }

  header = curl_slist_append(header, "Content-Type: application/x-www-form-urlencoded");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

  res = curl_easy_perform(curl);
  err_assert(res == CURLE_OK, curl_easy_strerror(res));

  long response_code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

  curl_easy_cleanup(curl);

  std::cout << buffer << std::endl;

  // parse
  response.Parse(buffer.c_str());

  return 0;
}

/* Authorization (device-flow) & Authentication */

std::string AutolabClient::device_flow_init() {
  // make a local copy and start building params
  std::string params("client_id=" + client_id);

  rapidjson::Document response;
  make_request(response, device_flow_init_path, params);

  err_assert(response.HasMember("device_code") && response.HasMember("user_code"),
    "Expected keys not found in response during device_flow_init");

  device_flow_device_code = response["device_code"].GetString();
  std::cout << "device_code: " << device_flow_device_code << std::endl; 
  return response["user_code"].GetString();
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

  std::string params("client_id=" + client_id +
                     "&device_code=" + device_flow_device_code);

  rapidjson::Document response;

  // get the current time
  auto t_now = std::chrono::steady_clock::now();
  auto t_end = t_now + std::chrono::seconds(timeout);
  // find out end time

  while (t_now < t_end) {
    make_request(response, device_flow_authorize_path, params);
    if (response.HasMember("code")) {
      // success!
      std::string code = response["code"].GetString();
      get_token_from_authorization_code(code);
      clear_device_flow_strings();
      return 0;
    } else if (response.HasMember("error") && 
        response["error"].GetString() != "authorization_pending") {
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

bool AutolabClient::get_token_from_authorization_code(std::string authorization_code) {
  std::string params("grant_type=authorization_code&client_id=" + client_id +
                     "&client_secret=" + client_secret +
                     "&redirect_uri=" + redirect_uri +
                     "&code=" + authorization_code);

  rapidjson::Document response;

  make_request(response, oauth_token_path, params, true);

  if (response.HasMember("access_token") && response.HasMember("refresh_token")) {
    // looks good
    access_token = response["access_token"].GetString();
    refresh_token = response["refresh_token"].GetString();
    return true;
  }

  return false;
}

int perform_token_refresh() {

}

/* REST Interface wrappers */
