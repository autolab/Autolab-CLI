/*
 * High level helpers for autolab_client
 */

#ifndef CLIENT_HELPERS_H
#define CLIENT_HELPERS_H

#include "autolab_client.h"

bool find_course(AutolabClient &ac, std::string &name_hint);
bool find_assessment(AutolabClient &ac, const std::string &course_name, std::string name_hint);

#endif /* CLIENT_HELPERS_H */