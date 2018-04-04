#include "cache.h"
#include "../file/file_utils.h"
#include <stdio.h>
#include <string.h>

bool cache_exists() {

  char buf[256];
  strcpy(buf, get_home_dir());
  strcat(buf, "/.autolab/cache/");

  return dir_exists(buf);
}

bool cache_file_exists();
bool cache_entry_exists();
