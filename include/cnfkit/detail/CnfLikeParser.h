#pragma once

#include <cnfkit/Literal.h>

#include <zlib.h>

#include <charconv>
#include <filesystem>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>

namespace cnfkit::detail {

inline auto is_irrelevant_line(std::string const& line) -> bool
{
  return std::regex_match(line, std::regex{"\\s*"}) ||
         std::regex_match(line, std::regex{"\\s*c.*"});
}

class cnf_gz_file {
public:
  explicit cnf_gz_file(std::filesystem::path const& file)
  {
    m_file = gzopen(file.string().data(), "rb");
    if (m_file == nullptr) {
      std::perror(file.string().data());
      throw std::runtime_error{"Could not open input file."};
    }
  }

  cnf_gz_file()
  {
    m_file = gzdopen(0, "rb");
    if (m_file == nullptr) {
      throw std::runtime_error{"Could not open stdin for reading."};
    }
  }

  ~cnf_gz_file()
  {
    if (m_file != nullptr) {
      gzclose(m_file);
    }
  }

  auto read_char() -> std::optional<char>
  {
    char character = 0;
    int chars_read = gzread(m_file, &character, 1);

    if (chars_read == 0 && is_eof()) {
      return std::nullopt;
    }

    if (chars_read <= 0) {
      int errnum = 0;
      char const* message = gzerror(m_file, &errnum);
      throw std::runtime_error{message};
    }

    return character;
  }

  auto read_line() -> std::string
  {
    std::string result;
    constexpr std::string::size_type initial_buf_size = 512;
    result.reserve(initial_buf_size);
    std::optional<char> character;
    while ((character = read_char())) {
      if (*character == '\n') {
        break;
      }
      result += *character;
    }
    return result;
  }

  auto read_header_line() -> std::string
  {
    std::string line;
    do {
      line = read_line();
    } while (is_irrelevant_line(line) && !is_eof());

    return line;
  }

  void skip_line()
  {
    std::optional<char> character = '\0';
    while (character.has_value() && *character != '\n') {
      character = read_char();
    }
  }

  void read_chunk(size_t desired_size, std::string& buffer)
  {
    if (desired_size == 0) {
      buffer.clear();
      return;
    }

    buffer.resize(desired_size);
    int const bytes_read = gzread(m_file, buffer.data(), desired_size);

    if (bytes_read < 0) {
      int errnum = 0;
      char const* message = gzerror(m_file, &errnum);
      throw std::runtime_error{message};
    }

    buffer.resize(bytes_read);

    // stopped in the middle of a literal ~> read rest, too
    while (!is_eof() && std::isspace(buffer.back()) == 0) {
      buffer.push_back(*read_char());
    }
  }

  bool is_eof() const { return gzeof(m_file) != 0; }

  cnf_gz_file(cnf_gz_file const& rhs) = delete;
  auto operator=(cnf_gz_file const& rhs) -> cnf_gz_file& = delete;
  cnf_gz_file(cnf_gz_file&& rhs) = default;
  auto operator=(cnf_gz_file&& rhs) -> cnf_gz_file& = default;

private:
  gzFile m_file;
};

template <typename It>
auto skip_whitespace(It start, It stop) -> It
{
  return std::find_if_not(start, stop, [](char c) { return std::isspace(c); });
}

template <typename It>
auto skip_to_line_end(It start, It stop) -> It
{
  return std::find(start, stop, '\n');
}

template <typename It>
auto skip_dimacs_comments(It start, It stop) -> std::pair<It, bool>
{
  auto iter = skip_whitespace(start, stop);

  while (iter != stop && *iter == 'c') {
    ++iter;
    iter = skip_to_line_end(iter, stop);
    if (iter == stop) {
      return std::make_pair(iter, true);
    }
    iter = skip_whitespace(iter, stop);
  }

  return std::make_pair(iter, false);
}

inline auto dimacs_to_lit(int dimacs_lit) -> lit
{
  if (dimacs_lit == std::numeric_limits<int>::min()) {
    throw std::invalid_argument{"variable out of range"};
  }

  int const variable = std::abs(dimacs_lit);
  if (variable > max_raw_var) {
    throw std::invalid_argument{"variable out of range"};
  }

  return lit{var{static_cast<uint32_t>(variable)}, dimacs_lit > 0};
}

struct dimacs_problem_header {
  size_t num_vars = 0;
  size_t num_clauses = 0;
  size_t header_size = 0;
};

class cnf_chunk_parser {
public:
  template <typename UnaryFn>
  void parse(std::string const& buffer, size_t offset, UnaryFn&& clause_receiver)
  {
    char const* c_end = buffer.c_str() + buffer.size();
    auto const end = buffer.begin() + buffer.size();
    auto cursor = buffer.begin() + offset;

    if (m_is_in_comment) {
      cursor = skip_to_line_end(cursor, end);
      if (cursor != end) {
        m_is_in_comment = false;
      }
    }

    int literal = 0;
    while (cursor != end) {
      auto const [past_comments, ended_in_comment] = skip_dimacs_comments(cursor, end);

      if (past_comments == end) {
        m_is_in_comment = ended_in_comment;
        return;
      }
      else {
        m_is_in_comment = false;
      }

      auto [next, errorcode] = std::from_chars(&*past_comments, c_end, literal);

      if (errorcode != std::errc{}) {
        if (next == c_end) {
          return;
        }
        throw std::invalid_argument{"syntax error"};
      }

      if (literal != 0) {
        m_lit_buffer.push_back(dimacs_to_lit(literal));
      }
      else {
        clause_receiver(m_lit_buffer);
        m_lit_buffer.clear();
        ++m_num_clauses_read;
      }

      cursor = buffer.begin() + (next - buffer.c_str());
    }
  }

  void check_on_dimacs_finish(dimacs_problem_header const& header)
  {
    if (m_num_clauses_read != header.num_clauses) {
      throw std::invalid_argument{"invalid number of clauses in CNF data"};
    }

    if (!m_lit_buffer.empty()) {
      throw std::invalid_argument{"CNF data ends in open clause"};
    }
  }

  void check_on_drat_finish()
  {
    if (!m_lit_buffer.empty()) {
      throw std::invalid_argument{"Proof data ends in open clause"};
    }
  }

private:
  size_t m_num_clauses_read = 0;
  std::vector<lit> m_lit_buffer;
  bool m_is_in_comment = false;
};

}
