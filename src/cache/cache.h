#ifndef CACHE
#define CACHE

#include <string>

bool cache_exists();

bool cache_file_exists();

bool cache_course_entry_exists();

void print_course_cache_entry();

void update_course_cache_entry();

bool cache_asmt_entry_exists(std::string course_id);

void print_asmt_cache_entry(std::string course_id);

void update_asmt_cache_entry(std::string course_id);

#endif
