#include "cache.h"
#include "../file/file_utils.h"

bool cache_exists() {
  printf("%s\n", get_home_dir());

  return true;
}

bool cache_file_exists();
bool cache_entry_exists();
