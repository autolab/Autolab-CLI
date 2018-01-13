#include "config_manager.h"
#include "file_utils.h"
#include "logger.h"

#define TOKEN_CACHE_FILE_MAXSIZE 256

const std::string token_cache_filename = ".arcache";
const std::string cred_dirname = ".autolab";

std::string token_pair_to_string(std::string at, std::string rt) {
  return at + "\n" + rt;
}

/* private helpers */
std::string cred_dir_full_path;
std::string token_cache_file_full_path;

std::string get_cred_dir_full_path() {
  if (cred_dir_full_path.length() > 0)
    return cred_dir_full_path;

  cred_dir_full_path.append(get_home_dir());
  cred_dir_full_path.append("/");
  cred_dir_full_path.append(cred_dirname);
  return cred_dir_full_path;
}

std::string get_token_cache_file_full_path() {
  if (token_cache_file_full_path.length() > 0)
    return token_cache_file_full_path;

  token_cache_file_full_path.append(get_cred_dir_full_path());
  token_cache_file_full_path.append("/");
  token_cache_file_full_path.append(token_cache_filename);
  return token_cache_file_full_path;
}

// returns true if exists, returns false if had to create dir
bool check_and_create_token_directory() {
  const char *homedir = get_home_dir();

  bool exists = dir_find(homedir, cred_dirname.c_str(), true);
  if (exists) return true;

  bool success = create_dir(get_cred_dir_full_path().c_str());
  err_assert(success);
  return false;
}

bool token_cache_file_exists() {
  return dir_find(get_cred_dir_full_path().c_str(),
                  token_cache_filename.c_str());
}

/* interface */
void store_tokens(std::string at, std::string rt) {
  check_and_create_token_directory();

  int res = write_file(get_token_cache_file_full_path().c_str(),
                       token_pair_to_string(at, rt).c_str());
  Logger::debug << "[FileUtils] tokens stored" << Logger::endl;
  err_assert(res);
}

// returns true if got token, false if failed to get token.
// Failure likely because token cache file doesn't exist.
bool load_tokens(std::string &at, std::string &rt) {
  if (!check_and_create_token_directory()) return false;
  if (!token_cache_file_exists()) return false;

  char raw_result[TOKEN_CACHE_FILE_MAXSIZE];
  int res = read_file(get_token_cache_file_full_path().c_str(),
                      raw_result, TOKEN_CACHE_FILE_MAXSIZE);
  err_assert(res);

  std::string result(raw_result);
  std::string::size_type split_pos_1 = result.find('\n');
  if (split_pos_1 == std::string::npos) return false;
  std::string::size_type split_pos_2 = result.find('\n', split_pos_1+1);

  at.assign(result.substr(0, split_pos_1));
  rt.assign(result.substr(split_pos_1+1, split_pos_2 - split_pos_1 - 1));
  Logger::debug << "[FileUtils] tokens loaded" << Logger::endl;
  return true;
}



/************* asmt *************/
#define ASMT_FILE_MAXSIZE 128
const std::string asmt_filename = ".autolab-asmt";

std::string format_asmt_file(std::string course_name, std::string asmt_name) {
  return course_name + "\n" + asmt_name;
}

// tries to read asmt file from curr directory upwards
bool read_asmt_file(std::string &course_name, std::string &asmt_name) {
  char buffer[MAX_DIR_LENGTH];
  bool found = recur_find(buffer, get_curr_dir(), asmt_filename.c_str());
  if (!found) return false;

  std::string filename(buffer);
  int res = read_file(filename.c_str(), buffer, MAX_DIR_LENGTH);
  err_assert(res);

  std::string result(buffer);
  std::size_t split_pos = result.find("\n");
  if (split_pos == std::string::npos) return false;

  course_name.assign(result.substr(0, split_pos));
  asmt_name.assign(result.substr(split_pos+1, std::string::npos));

  return true;
}

// write asmt file in directory named dir_name under curr directory
bool write_asmt_file(std::string dir_name, std::string course_name, std::string asmt_name) {
  std::string full_path(get_curr_dir());
  full_path.append("/");
  full_path.append(dir_name);
  if (!dir_find(get_curr_dir(), dir_name.c_str(), true)) {
    create_dir(full_path.c_str());
  }

  full_path.append("/");
  full_path.append(asmt_filename);
  int res = write_file(full_path.c_str(),
                       format_asmt_file(course_name, asmt_name).c_str());
  err_assert(res);
}