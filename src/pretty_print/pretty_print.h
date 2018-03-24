/*
 * Functions that help with formatting strings.
 */

#ifndef AUTOLAB_PRETTY_PRINT_H_
#define AUTOLAB_PRETTY_PRINT_H_

#include <cstddef> // size_t

#include <string>
#include <vector>

// utility
int count_words(std::string src);
bool case_insensitive_str_equal(std::string a, std::string b);
bool nonempty(std::string src);

// conversions
std::string double_to_string(double num, int precision);
std::string bool_to_string(bool test);

// simple string processing
std::string to_lowercase(std::string src);
std::string left_trim(std::string src);
std::string right_trim(std::string src);
std::string center_text(std::size_t width, std::string text);

// advanced string processing
std::string wrap_text_with_indent(std::size_t indent, std::string text);
std::string format_table(std::vector<std::vector<std::string>> data);

#endif /* AUTOLAB_PRETTY_PRINT_H_ */