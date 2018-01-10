#ifndef AUTOLAB_CLIENT_H
#define AUTOLAB_CLIENT_H

#include <rapidjson/document.h>

class AutolabClient {
public:
  AutolabClient(std::string &id, std::string &st, std::string &ru);

  // initializes curl interface. Must be called before anything else.
  static int init_curl();

  // setters and getters
  int set_tokens(std::string at, std::string rt);

  const std::string get_access_token() { return access_token; }
  const std::string get_refresh_token() { return refresh_token; }

  /* oauth-related */
  std::string device_flow_init();
  int device_flow_authorize(size_t timeout);

private:
  static int curl_ready;

  std::string client_id;
  std::string client_secret;
  std::string redirect_uri;
  std::string access_token;
  std::string refresh_token;
  std::string device_flow_device_code;
  std::string device_flow_user_code;
  // perform HTTP request and return result, default method is GET.
  int make_request(rapidjson::Document &response, const std::string &path, const std::string &params, bool is_post);

  void clear_device_flow_strings();

  bool get_token_from_authorization_code(std::string authorization_code);
  int perform_token_refresh();
};

#endif /* AUTOLAB_CLIENT_H */