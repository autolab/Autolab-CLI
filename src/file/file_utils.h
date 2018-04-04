/*
 * Functions to help interface with the filesystem.
 *
 * Purely C-based, and is very platform-dependent.
 * Once C++17 filesystems becomes standardized, these functions should be
 * rewritten using the new standard.
 */

#ifndef AUTOLAB_FILE_UTILS_H_
#define AUTOLAB_FILE_UTILS_H_

#include <stddef.h>

#define MAX_DIR_LENGTH 256
#define DEFAULT_RECUR_LEVEL 8

bool file_exists(const char *path_to_file);
bool dir_exists(const char *path_to_dir);
bool dir_find(const char *dirname, const char *targetname, bool target_is_dir = false);
bool recur_find(char *result, const char *dirstart, const char *targetname,
                  bool target_is_dir = false, int levels = DEFAULT_RECUR_LEVEL);

// filesystem manipulation methods
// always succeeds on return. If an action fails, error is printed to stderr
// and the program is exited immediately.
void create_dir(const char *dirname);
size_t read_file(const char *filename, char *result, size_t max_length);
void write_file(const char *filename, const char *data, size_t length);

const char *get_home_dir();
const char *get_curr_dir();

#endif /* AUTOLAB_FILE_UTILS_H_ */
