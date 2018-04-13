#include <stdio.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>

#include "logger.h"

#include "cache.h"
#include "../context_manager/context_manager.h"
#include "../file/file_utils.h"

const std::string courses_cache_filename = "courses.txt";
const std::string cache_dirname = "cache";

std::string get_cache_dir_full_path() {
  std::string cache_dir_full_path = get_cred_dir_full_path();
  cache_dir_full_path.append("/");
  cache_dir_full_path.append(cache_dirname);
  return cache_dir_full_path;
}

std::string get_courses_cache_file_full_path() {
  std::string courses_cache_file_full_path = get_cache_dir_full_path();
  courses_cache_file_full_path.append("/");
  courses_cache_file_full_path.append(courses_cache_filename);
  return courses_cache_file_full_path;
}

std::string get_asmts_cache_file_full_path(std::string course_id) {
  std::string asmts_cache_file_full_path = get_cache_dir_full_path();
  asmts_cache_file_full_path.append("/");
  asmts_cache_file_full_path.append(course_id);
  asmts_cache_file_full_path.append(".txt");
  return asmts_cache_file_full_path;
}

bool check_and_create_cache_directory() {
  check_and_create_token_directory();
  std::string cred_dir = get_cred_dir_full_path();

  bool exists = dir_find(cred_dir.c_str(), cache_dirname.c_str(), true);
  if (exists) return true;

  create_dir(get_cache_dir_full_path().c_str());
  return false;
}

void print_cache_entry(std::string filename) {
  if (!file_exists(filename.c_str())) {
    return; // no need to print anything in this case
  }

  std::ifstream cache_file;
  cache_file.open(filename.c_str());

  std::string curr_line;

  // Read through the cache file line by line, cat to Logger::info
  while (std::getline(cache_file, curr_line)) {
    Logger::info << curr_line << Logger::endl;
  }

  cache_file.close();
}

/* courses cache file */
void update_course_cache_entry(std::vector<Autolab::Course> &courses) {
  check_and_create_cache_directory();

  std::ostringstream out;
  for (auto &c : courses) {
    out << "  " << c.name << " (" << c.display_name << ")\n";
  }
  std::string cache_contents = out.str();

  write_file(get_courses_cache_file_full_path().c_str(),
             cache_contents.c_str(), cache_contents.length());

  LogDebug("[Cache] courses cache saved" << Logger::endl);
}

void print_course_cache_entry() {
  print_cache_entry(get_courses_cache_file_full_path());
}

/* asmts cache file */
void update_asmt_cache_entry(std::string course_id, std::vector<Autolab::Assessment> &asmts) {
  check_and_create_cache_directory();

  std::ostringstream out;
  for (auto &a : asmts) {
    out << "  " << a.name << " (" << a.display_name << ")\n";
  }
  std::string cache_contents = out.str();

  write_file(get_asmts_cache_file_full_path(course_id).c_str(),
             cache_contents.c_str(), cache_contents.length());

  LogDebug("[Cache] asmts cache saved for course: " << course_id << Logger::endl);
}

void print_asmt_cache_entry(std::string course_id) {
  print_cache_entry(get_asmts_cache_file_full_path(course_id));
}
