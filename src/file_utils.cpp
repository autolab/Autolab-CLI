#include "file_utils.h"

#include <string.h>
#include <dirent.h>   // dir-related
#include <sys/stat.h> // mkdir
#include <fcntl.h>    // open
#include <unistd.h>   // close, write
#include <pwd.h>      // getpwuid
#include <errno.h>
#include <stdlib.h>

const char *home_directory = NULL;
char curr_directory[MAX_DIR_LENGTH];

bool dir_find(const char *dirname, const char *targetname, bool target_is_dir) {
  bool found = false;
  unsigned char target_type = target_is_dir ? DT_DIR : DT_REG;

  DIR *dir = opendir(dirname);
  if (!dir) {
    // std::cout << "error opening directory" << std::endl;
    return false;
  }

  struct dirent *entry;
  while (entry = readdir(dir)) {
    if ((entry->d_type == target_type) &&
        (strcmp(entry->d_name, targetname) == 0)) {
      found = true;
      break;
    }
  }

  closedir(dir);
  return found;
}

// private helper for recur_find
// modifies currdir so that it is one level less
bool one_level_up(char *currdir) {
  size_t len = strnlen(currdir, MAX_DIR_LENGTH);
  if (len <= 1) return false; // already at root

  char *ptr = currdir + len - 2; // skip last char
  while (*ptr != '/') {
    if (ptr == currdir) return false; // invalid path, cannot go up
    --ptr;
  }

  *ptr = '\0';
  return true;
}

bool recur_find(char *result, const char *dirstart, const char *targetname, 
                  bool target_is_dir, int levels) {
  char currdir[MAX_DIR_LENGTH];

  if (strnlen(dirstart, MAX_DIR_LENGTH) >= MAX_DIR_LENGTH) return false;

  // make copy into local modifiable buffer
  strcpy(currdir, dirstart);

  for (int i = 0; i < levels; i++) {
    if (dir_find(currdir, targetname, target_is_dir)) {
      if (result) {
        strcpy(result, currdir);
        size_t len = strlen(result);
        result[len] = '/';
        strncpy(result + len + 1, targetname, MAX_DIR_LENGTH - len - 1);
        result[MAX_DIR_LENGTH] = '\0';
      }
      return true;
    }
    // didn't find it, go up one level and retry
    if (!one_level_up(currdir)) return false;
  }

  return false;
}

// create a directory with only owner read/write/execute permissions
bool create_dir(const char *dirname) {
  int res = mkdir(dirname, S_IRWXU);
  if (res < 0 && errno != EEXIST) return false;
  return true;
}

// open a file for reading only. Reads at most max_length chars into result.
bool read_file(const char *filename, char *result, size_t max_length) {
  int fd = open(filename, O_RDONLY | O_CREAT, S_IRWXU);
  if (fd < 0) return false;

  ssize_t amount = 0;
  size_t total_read = 0;
  while (amount = TEMP_FAILURE_RETRY(read(fd, result + total_read, max_length))) {
    if (amount < 0) {
      close(fd);
      return false;
    }
    // amount is positive
    total_read += (size_t)amount;
    max_length -= (size_t)amount;
  }
  result[total_read] = '\0';

  close(fd);
  return true;
}

// open a file for writing only, and sets permissions to only
// owner read/write/execute
bool write_file(const char *filename, const char *data) {
  int fd = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
  if (fd < 0) return false;

  size_t remaining = strlen(data);
  size_t total_written = 0;
  while (remaining > 0) {
    ssize_t amount = TEMP_FAILURE_RETRY(write(fd, data + total_written, remaining));
    if (amount < 0) {
      close(fd);
      return false;
    }
    // amount is non-negative
    remaining -= (size_t)amount;
    total_written += (size_t)amount;
  }

  close(fd);
  return true;
}

const char *get_home_dir() {
  if (home_directory) return home_directory;

  const char *homedir = getenv("HOME");
  if (homedir == NULL) {
      homedir = getpwuid(getuid())->pw_dir;
  }
  home_directory = homedir;
  return home_directory;
}

const char *get_curr_dir() {
  if (*curr_directory != '\0') return curr_directory;

  getcwd(curr_directory, MAX_DIR_LENGTH);
  return curr_directory;
}