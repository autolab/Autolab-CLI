#include "context_manager.h"

#include "../app_credentials.h"
#include "../file/file_utils.h"
#include "autolab/autolab.h"
#include "logger.h"
#include "../crypto/pseudocrypto.h"

#define TOKEN_CACHE_FILE_MAXSIZE 256

const std::string token_cache_filename = ".arcache";
const std::string cred_dirname = ".autolab";

std::string token_pair_to_string(std::string at, std::string rt) {
  std::string pre_crypt = at + "\n" + rt;
  return encrypt_string(pre_crypt, crypto_key, crypto_iv);
}

bool token_pair_from_string(char *raw_src, size_t raw_len, std::string &at, std::string &rt) {
  std::string src = decrypt_string(raw_src, raw_len, crypto_key, crypto_iv);

  std::string::size_type split_pos_1 = src.find('\n');
  if (split_pos_1 == std::string::npos) return false;
  std::string::size_type split_pos_2 = src.find('\n', split_pos_1+1);

  at.assign(src, 0, split_pos_1);
  rt.assign(src, split_pos_1+1, split_pos_2 - split_pos_1 - 1);
  return true;
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

  create_dir(get_cred_dir_full_path().c_str());
  return false;
}

bool token_cache_file_exists() {
  return dir_find(get_cred_dir_full_path().c_str(),
                  token_cache_filename.c_str());
}

/* interface */
void store_tokens(std::string at, std::string rt) {
  check_and_create_token_directory();
  try {
      std::string token_pair = token_pair_to_string(at, rt);

      write_file(get_token_cache_file_full_path().c_str(),
                 token_pair.c_str(), token_pair.length());
  } catch (Autolab::CryptoException &e) {
    Logger::fatal << "OpenSSL error in store_tokens." << Logger::endl;
    Logger::fatal << e.what() << Logger::endl;
    exit(-1);
  }
  LogDebug("[ContextManager] tokens stored" << Logger::endl);
}

// returns true if got token, false if failed to get token.
// Failure likely because token cache file doesn't exist.
bool load_tokens(std::string &at, std::string &rt) {
  if (!check_and_create_token_directory()) return false;
  if (!token_cache_file_exists()) return false;

  char raw_result[TOKEN_CACHE_FILE_MAXSIZE];
  size_t num_read = read_file(get_token_cache_file_full_path().c_str(),
            raw_result, TOKEN_CACHE_FILE_MAXSIZE);
  LogDebug("read size " << num_read << "\n");

  try {
    if (!token_pair_from_string(raw_result, num_read, at, rt)) return false;
  } catch (Autolab::CryptoException &e) {
    Logger::fatal << "OpenSSL error in load_tokens." << Logger::endl;
    Logger::fatal << e.what() << Logger::endl;
    remove(get_token_cache_file_full_path().c_str());
    return false;
  }
  LogDebug("[ContextManager] tokens loaded" << Logger::endl);
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
  size_t num_read = read_file(filename.c_str(), buffer, MAX_DIR_LENGTH);

  std::string result(buffer, num_read);
  std::string::size_type split_pos_1 = result.find('\n');
  if (split_pos_1 == std::string::npos) return false;
  std::string::size_type split_pos_2 = result.find('\n', split_pos_1+1);

  course_name.assign(result, 0, split_pos_1);
  asmt_name.assign(result, split_pos_1+1, split_pos_2 - split_pos_1 - 1);

  return true;
}

// write asmt file in directory named dir_name under curr directory
void write_asmt_file(std::string dir_name, std::string course_name, std::string asmt_name) {
  std::string full_path(dir_name);
  full_path.append("/");
  full_path.append(asmt_filename);
  std::string formatted_asmt = format_asmt_file(course_name, asmt_name);
  write_file(full_path.c_str(), formatted_asmt.c_str(), formatted_asmt.length());
}
