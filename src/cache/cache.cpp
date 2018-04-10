#include "cache.h"
#include "../file/file_utils.h"
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <iostream>

#include "logger.h"

#include <stdlib.h>

bool cache_exists() {
  char buf[256];
  strcpy(buf, get_home_dir());
  strcat(buf, "/.autolab/cache/");

  return dir_exists(buf);
}

bool cache_file_exists();

bool cache_course_entry_exists() {
  char buf[256];
  strcpy(buf, get_home_dir());
  strcat(buf, "/.autolab/cache/courses.txt");

  return file_exists(buf);
}

void print_course_cache_entry() {
  char buf[256];
  strcpy(buf, get_home_dir());
  strcat(buf, "/.autolab/cache/courses.txt");

  std::ifstream infile;
  infile.open(buf);

  std::string curr_line;

  // Read through the cache file line by line, cat to Logger::info
  while (std::getline(infile, curr_line)) {
    Logger::info  << "  " << curr_line << Logger::endl;
  }

  infile.close();
}

void update_course_cache_entry() {
  if(!cache_exists()) {
    int res = system("mkdir $HOME/.autolab/cache/");
    if(res) {
      exit(1);
    }
  }
  int res1 = system("autolab courses -q -i >> $HOME/.autolab/cache/courses.txt");
  if(res1) {
    exit(1);
  }
}

bool cache_asmt_entry_exists(std::string course_id) {
  std::string home_dir(get_home_dir());

  std::string buf = home_dir + "/.autolab/cache/" + course_id + ".txt";

  return file_exists(buf.c_str());
}

void print_asmt_cache_entry(std::string course_id) {
  std::string home_dir(get_home_dir());

  std::string buf = home_dir + "/.autolab/cache/" + course_id + ".txt";

  std::ifstream infile;
  infile.open(buf.c_str());

  std::string curr_line;

  // Read through the cache file line by line, cat to Logger::info
  while (std::getline(infile, curr_line)) {
    Logger::info  << "  " << curr_line << Logger::endl;
  }

  infile.close();
}

void update_asmt_cache_entry(std::string course_id) {
  if(!cache_exists()) {
    int res = system("mkdir $HOME/.autolab/cache/");
    if(res) {
      exit(1);
    }
  }

  std::string buf = "autolab asmts " + course_id + " -q -i >> $HOME/.autolab/cache/" + course_id + ".txt";


  int res1 = system(buf.c_str());
  if(res1) {
    exit(1);
  }
}
