#ifndef AUTOLAB_PRETTY_PRINT_H_
#define AUTOLAB_PRETTY_PRINT_H_

#include <string>
#include <vector>

std::string double_to_string(double num, int precision);
std::string left_trim(std::string src);
std::string right_trim(std::string src);
std::string center_text(int width, std::string text);
int count_words(std::string src);
void print_wrapped(int indent, std::string text);
std::string format_table(std::vector<std::vector<std::string>> data);

#endif /* AUTOLAB_PRETTY_PRINT_H_ */