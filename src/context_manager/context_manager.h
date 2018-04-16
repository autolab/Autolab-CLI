/*
 * Functions that help with understanding the context of the program.
 *
 * Includes functions that help manipulate the tokens cache file and the
 * asmt config files.
 */

#ifndef AUTOLAB_CONTEXT_MANAGER_H_
#define AUTOLAB_CONTEXT_MANAGER_H_

#include <string>

std::string get_cred_dir_full_path();
bool check_and_create_token_directory();

// read tokens from file. If nonexistent, return false.
bool load_tokens(std::string &at, std::string &rt);

// store tokens to file.
void store_tokens(std::string at, std::string rt);

bool read_asmt_file(std::string &course_name, std::string &asmt_name);
void write_asmt_file(std::string filename, std::string course_name, std::string asmt_name);


#endif /* AUTOLAB_CONTEXT_MANAGER_H_ */