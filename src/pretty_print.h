/*
 * Functions that help with formatting strings.
 */

#ifndef AUTOLAB_PRETTY_PRINT_H_
#define AUTOLAB_PRETTY_PRINT_H_

#include <string>
#include <vector>

// utility
int count_words(std::string src);

// conversions
std::string double_to_string(double num, int precision);

// simple string processing
std::string left_trim(std::string src);
std::string right_trim(std::string src);
std::string center_text(int width, std::string text);

// advanced string processing
std::string wrap_text_with_indent(int indent, std::string text);
std::string format_table(std::vector<std::vector<std::string>> data);

#endif /* AUTOLAB_PRETTY_PRINT_H_ */