#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>

// read tokens from file. If nonexistent, return false.
bool load_tokens(std::string &at, std::string &rt);

// store tokens to file.
void store_tokens(std::string at, std::string rt);

bool read_asmt_file(std::string &course_name, std::string &asmt_name);
bool write_asmt_file(std::string filename, std::string course_name, std::string asmt_name);


#endif /* CONFIG_MANAGER_H */