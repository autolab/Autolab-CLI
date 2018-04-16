#ifndef AUTOLAB_CACHE_H_
#define AUTOLAB_CACHE_H_

#include <string>

#include "autolab/autolab.h"

/* courses cache file */
void update_course_cache_entry(std::vector<Autolab::Course> &courses);
void print_course_cache_entry();

/* asmts cache file */
void update_asmt_cache_entry(std::string course_id, std::vector<Autolab::Assessment> &asmts);
void print_asmt_cache_entry(std::string course_id);

#endif /* AUTOLAB_CACHE_H_ */
