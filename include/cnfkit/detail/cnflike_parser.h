#pragma once

#include <cnfkit/io/io_zlib.h>
#include <cnfkit/literal.h>

#include <charconv>
#include <filesystem>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>


namespace cnfkit::detail {

constexpr size_t default_chunk_size = (1 << 16);

inline auto is_irrelevant_line(std::string const& line) -> bool
{
  return std::regex_match(line, std::regex{"\\s*"}) ||
         std::regex_match(line, std::regex{"\\s*c.*"});
}

class cnf_source_reader {
public:
  explicit cnf_source_reader(source& source) : m_source{source} {}

  auto read_char() -> std::optional<char>
  {
    std::optional<std::byte> result = m_source.read_byte();
    if (!result.has_value()) {
      return std::nullopt;
    }
    return std::to_integer<char>(*result);
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
    } while (is_irrelevant_line(line) && !m_source.is_eof());

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
    std::byte* byte_buffer = reinterpret_cast<std::byte*>(buffer.data());
    std::byte* read_stop = m_source.read_bytes(byte_buffer, byte_buffer + desired_size);
    buffer.resize(read_stop - byte_buffer);

    // stopped in the middle of a literal ~> read rest, too
    while (!m_source.is_eof() && std::isspace(buffer.back()) == 0) {
      buffer.push_back(*read_char());
    }
  }

  auto is_eof() -> bool { return m_source.is_eof(); };

private:
  source& m_source;
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

struct dimacs_problem_header {
  size_t num_vars = 0;
  size_t num_clauses = 0;
  size_t header_size = 0;
};

enum class cnf_chunk_parser_mode { dimacs, drat };

class cnf_chunk_parser {
public:
  explicit cnf_chunk_parser(cnf_chunk_parser_mode mode) : m_mode{mode} {}

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
      auto [next_lit, ended_in_comment] = skip_dimacs_comments(cursor, end);

      m_is_in_comment = ended_in_comment;
      if (next_lit == end) {
        return;
      }

      if (m_mode == cnf_chunk_parser_mode::drat && *next_lit == 'd') {
        if (!m_lit_buffer.empty()) {
          throw std::invalid_argument{"syntax error: d may only occur before clauses"};
        }

        auto past_delete = skip_whitespace(next_lit + 1, end);
        if (past_delete == next_lit + 1) {
          throw std::invalid_argument{"syntax error: d must be followed by whitespace"};
        }

        next_lit = past_delete;
        m_is_in_delete = true;
      }

      auto [next, errorcode] = std::from_chars(&*next_lit, c_end, literal);

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
        clause_receiver(!m_is_in_delete, m_lit_buffer);
        m_is_in_delete = false;
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
    if (!m_lit_buffer.empty() || m_is_in_delete) {
      throw std::invalid_argument{"Proof data ends in open clause"};
    }
  }

private:
  cnf_chunk_parser_mode m_mode;
  size_t m_num_clauses_read = 0;
  std::vector<lit> m_lit_buffer;
  bool m_is_in_comment = false;
  bool m_is_in_delete = false;
};

}
