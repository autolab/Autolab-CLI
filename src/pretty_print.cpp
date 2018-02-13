#include "pretty_print.h"

#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "logger.h"

const int output_line_width = 80;
const std::string whitespace_chars = " \t\n";

std::string double_to_string(double num, int precision) {
  std::ostringstream num_str;
  num_str << std::fixed << std::setprecision(precision) << num;
  return num_str.str();
}

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

std::string center_text(int width, std::string text) {
  if (text.length() >= width) return text;

  int left = (width - text.length()) / 2;

  std::string output;
  output.resize(left, ' ');
  output.append(text);
  output.resize(width, ' ');

  return output;
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

// print a table
std::string format_table(std::vector<std::vector<std::string>> data) {
  std::ostringstream out;
  int num_rows, num_cols;
  num_rows = data.size();
  num_cols = data[0].size();

  // find out width of each column
  std::vector<int> col_widths;
  col_widths.resize(num_cols, 0);
  for (auto &row : data) {
    for (int i = 0; i < num_cols; i++) {
      if (row[i].length() > col_widths[i]) {
        col_widths[i] = row[i].length();
      }
    }
  }

  // actually print the table
  // print header
  auto &header = data[0];
  out << "| ";
  for (int i = 0; i < num_cols; i++) {
    out << center_text(col_widths[i], header[i]) << " | ";
  }
  out << "\n";

  // print horiontal line
  out << "+";
  for (int i = 0; i < num_cols; i++) {
    out << std::string(col_widths[i] + 2 /* padding */, '-') << "+";
  }
  out << "\n";

  // print body
  for (int i = 1; i < num_rows; i++) {
    auto &row = data[i];

    out << "| ";
    for (int j = 0; j < num_cols; j++) {
      out << std::setw(col_widths[j]) << row[j] << " | ";
    }
    out << "\n";
  }

  return out.str();
}