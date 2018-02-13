#include "json_helpers.h"

#include <sstream>

#include <rapidjson/document.h>

#include "autolab/autolab.h"

void throw_unexpected_null_error(std::string key, std::string expected_type) {
  std::ostringstream msg_builder;
  msg_builder << "Expected key '" << key << "' to be of type '"
    << expected_type << "' but got type 'null'";
  throw Autolab::InvalidResponseException(msg_builder.str());
}

void require_or_throw_invalid_response(bool guard, std::string msg) {
  if (!guard) {
    throw Autolab::InvalidResponseException(msg);
  }
}

void require_is_array(rapidjson::Value &obj) {
  require_or_throw_invalid_response(obj.IsArray(), "Expected json array not found");
}

void require_is_object(rapidjson::Value &obj) {
  require_or_throw_invalid_response(obj.IsObject(), "Expected json object not found");
}

void require_key_exists(rapidjson::Value &obj, std::string key) {
  require_or_throw_invalid_response(obj.HasMember(key.c_str()),
    "Expected key " + key + " not found in json object.");
}

rapidjson::Value &get_candidate(rapidjson::Value &obj, std::string key) {
  require_key_exists(obj, key);
  return obj[key.c_str()];
}

// Methods for getting basic types from objects: Bool, Double, Int, String
bool get_bool_internal(rapidjson::Value &obj, std::string key, bool &result) {
  rapidjson::Value &candidate = get_candidate(obj, key);
  if (candidate.IsBool()) {
    result = candidate.GetBool();
    return true;
  }
  return false;
}
bool get_double_internal(rapidjson::Value &obj, std::string key, double &result) {
  rapidjson::Value &candidate = get_candidate(obj, key);
  if (candidate.IsDouble()) {
    result = candidate.GetDouble();
    return true;
  }
  return false;
}
bool get_int_internal(rapidjson::Value &obj, std::string key, int &result) {
  rapidjson::Value &candidate = get_candidate(obj, key);
  if (candidate.IsInt()) {
    result = candidate.GetInt();
    return true;
  }
  return false;
}
bool get_string_internal(rapidjson::Value &obj, std::string key, std::string &result) {
  rapidjson::Value &candidate = get_candidate(obj, key);
  if (candidate.IsString()) {
    result = candidate.GetString();
    return true;
  }
  return false;
}

bool get_bool(rapidjson::Value &obj, std::string key, bool fallback) {
  bool result;
  if (get_bool_internal(obj, key, result)) return result;
  return fallback;
}
double get_double(rapidjson::Value &obj, std::string key, double fallback) {
  double result;
  if (get_double_internal(obj, key, result)) return result;
  return fallback;
}
int get_int(rapidjson::Value &obj, std::string key, int fallback) {
  int result;
  if (get_int_internal(obj, key, result)) return result;
  return fallback;
}
std::string get_string(rapidjson::Value &obj, std::string key, std::string fallback) {
  std::string result;
  if (get_string_internal(obj, key, result)) return result;
  return fallback;
}

bool get_bool_force(rapidjson::Value &obj, std::string key) {
  bool result;
  if (!get_bool_internal(obj, key, result)) {
    throw_unexpected_null_error(key, "bool");
  }
  return result;
}
double get_double_force(rapidjson::Value &obj, std::string key) {
  double result;
  if (!get_double_internal(obj, key, result)) {
    throw_unexpected_null_error(key, "double");
  }
  return result;
}
int get_int_force(rapidjson::Value &obj, std::string key) {
  int result;
  if (!get_int_internal(obj, key, result)) {
    throw_unexpected_null_error(key, "int");
  }
  return result;
}
std::string get_string_force(rapidjson::Value &obj, std::string key) {
  std::string result;
  if (!get_string_internal(obj, key, result)) {
    throw_unexpected_null_error(key, "string");
  }
  return result;
}