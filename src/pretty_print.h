#ifndef AUTOLAB_PRETTY_PRINT_H_
#define AUTOLAB_PRETTY_PRINT_H_

#include <string>

std::string left_trim(std::string src);
std::string right_trim(std::string src);
int count_words(std::string src);
void print_wrapped(int indent, std::string text);

#endif /* AUTOLAB_PRETTY_PRINT_H_ */