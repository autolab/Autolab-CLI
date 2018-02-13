#include "pretty_print.h"

#include <iomanip>
#include <string>

#include "logger.h"

const int output_line_width = 80;
const std::string whitespace_chars = " \t\n";

// trim whitespace on the left
std::string left_trim(std::string src) {
  std::size_t word_begin = src.find_first_not_of(whitespace_chars);
  if (std::string::npos != word_begin) {
    src = src.substr(word_begin, std::string::npos);
  }
  return src;
}

// trim spaces on the right
std::string right_trim(std::string src) {
  std::size_t word_end = src.find_last_not_of(whitespace_chars);
  if (std::string::npos != word_end) {
    src = src.substr(0, word_end + 1);
  }
  return src;
}

int count_words(std::string src) {
  std::string input = left_trim(right_trim(src));

  int num_words = 0;
  bool in_word = false;
  for (char &c : input) {
    if (in_word && c == ' ') {
      in_word = false;
    } else if (!in_word && c != ' ') {
      in_word = true;
      num_words++;
    }
  }

  return num_words;
}

// print text wrapped and with left indent. First line is not indented
void print_wrapped(int indent, std::string text) {
  if (indent >= output_line_width) {
    // this should not happen
    Logger::info << text << Logger::endl;
  }

  int len_per_line = output_line_width - indent;
  int start = 0;
  while (start < text.length()) {
    std::string curr_str = text.substr(start, len_per_line);
    start += len_per_line;
    // left trim spaces
    curr_str = left_trim(curr_str);
    // leave out incomplete words on the right side
    if (start < text.length() &&
          curr_str.back() != ' ' && text[start] != ' ') {
      std::size_t char_end = curr_str.find_last_of(' ');
      start -= (curr_str.length() - char_end - 1);
      curr_str = curr_str.substr(0, char_end);
    }
    // print
    Logger::info << curr_str << Logger::endl;
    if (start >= text.length()) break;
    Logger::info << std::setw(indent) << "";
  }
}