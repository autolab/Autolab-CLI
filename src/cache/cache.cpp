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
  while (infile >> curr_line) {
    Logger::info  << "  " << curr_line;
  }

  Logger::info << Logger::endl;

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
