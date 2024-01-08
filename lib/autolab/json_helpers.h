/*
 * Functions for getting data from json values.
 */

#ifndef LIBAUTOLAB_JSON_HELPERS_H_
#define LIBAUTOLAB_JSON_HELPERS_H_

#include <cmath>
#include <string>
#include <rapidjson/document.h>

void require_is_array(rapidjson::Value &obj);
void require_is_object(rapidjson::Value &obj);

// Methods for getting basic types from objects: Bool, Double, Int, String
// Default fallbacks:
//   get_string: empty string ""
//   get_double: NaN
// All other functions require a fallback value.
bool get_bool(rapidjson::Value &obj, std::string key, bool fallback);
double get_double(rapidjson::Value &obj, std::string key,
    double fallback = std::nan(""));
int get_int(rapidjson::Value &obj, std::string key, int fallback);
std::string get_string(rapidjson::Value &obj, std::string key, 
    std::string fallback = std::string());

bool get_bool_force(rapidjson::Value &obj, std::string key);
double get_double_force(rapidjson::Value &obj, std::string key);
int get_int_force(rapidjson::Value &obj, std::string key);
std::string get_string_force(rapidjson::Value &obj, std::string key);

#endif /* LIBAUTOLAB_JSON_HELPERS_H_ */