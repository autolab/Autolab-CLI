#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <unistd.h>

#define MAX_DIR_LENGTH 256
#define DEFAULT_RECUR_LEVEL 5

bool dir_find(const char *dirname, const char *targetname, bool target_is_dir = false);
bool recur_find(char *result, const char *dirstart, const char *targetname, 
                  bool target_is_dir = false, int levels = DEFAULT_RECUR_LEVEL);

bool create_dir(const char *dirname);
bool read_file(const char *filename, char *result, size_t max_length);
bool write_file(const char *filename, const char *data);

const char *get_home_dir();
const char *get_curr_dir();

#endif /* FILE_UTILS_H */