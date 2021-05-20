#pragma once

#include <cnfkit/Literal.h>

#include <zlib.h>

#include <charconv>
#include <filesystem>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>


namespace cnfkit {
template <typename UnaryFn>
void parse_cnf_file(std::filesystem::path const& input_file, UnaryFn&& clause_receiver);

template <typename UnaryFn>
void parse_cnf_from_stdin(UnaryFn&& clause_receiver);

template <typename UnaryFn>
void parse_cnf_string(std::string const& cnf, UnaryFn&& clause_receiver);


/*** Implementation ***/

namespace detail {

auto is_irrelevant_line(std::string const& line) -> bool
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
auto skip_dimacs_comments(It start, It stop) -> It
{
  auto iter = skip_whitespace(start, stop);

  while (iter != stop && *iter == 'c') {
    ++iter;
    iter = skip_to_line_end(iter, stop);
    iter = skip_whitespace(iter, stop);
  }

  return iter;
}

struct problem_header {
  size_t num_vars = 0;
  size_t num_clauses = 0;
  size_t header_size = 0;
};

inline auto parse_cnf_header_line(std::string_view buffer) -> problem_header
{
  auto const buffer_past_comments = skip_dimacs_comments(buffer.begin(), buffer.end());

  std::regex const header_regex{"^\\s*p\\s*cnf\\s*([0-9]+)\\s*([0-9]+)"};
  std::match_results<std::string_view::const_iterator> header_match;
  if (std::regex_search(buffer_past_comments, buffer.end(), header_match, header_regex)) {
    if (header_match.size() != 3) {
      throw std::invalid_argument{"Syntax error in CNF header"};
    }

    problem_header result;

    result.header_size = header_match[0].second - header_match[0].first +
                         std::distance(buffer.begin(), buffer_past_comments);

    auto const [next, ec] =
        std::from_chars(header_match[1].first, header_match[1].second, result.num_vars);
    if (ec != std::errc{}) {
      throw std::invalid_argument{"Invalid number of variables"};
    }

    auto const [next2, ec2] =
        std::from_chars(header_match[2].first, header_match[2].second, result.num_clauses);
    if (ec2 != std::errc{}) {
      throw std::invalid_argument{"Invalid number of clauses"};
    }

    return result;
  }
  else {
    throw std::invalid_argument{"Syntax error in CNF header"};
  }
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

struct parse_cnf_chunk_result {
  size_t num_clauses_read = 0;
  std::vector<lit> open_clause;
};

template <typename UnaryFn>
auto parse_cnf_chunk(std::string const& buffer,
                     size_t offset,
                     std::vector<lit> open_clause,
                     UnaryFn&& clause_receiver) -> parse_cnf_chunk_result
{
  size_t num_clauses_read = 0;

  char const* c_end = buffer.c_str() + buffer.size();
  auto const end = buffer.begin() + buffer.size();
  auto cursor = buffer.begin() + offset;

  std::vector<lit> lit_buffer = open_clause;

  int literal = 0;

  while (cursor != end) {
    auto const past_comments = skip_dimacs_comments(cursor, end);

    if (past_comments == end) {
      break;
    }

    auto [next, errorcode] = std::from_chars(&*past_comments, c_end, literal);

    if (errorcode != std::errc{}) {
      if (next == c_end) {
        break;
      }
      throw std::invalid_argument{"syntax error"};
    }

    if (literal != 0) {
      lit_buffer.push_back(dimacs_to_lit(literal));
    }
    else {
      clause_receiver(lit_buffer);
      lit_buffer.clear();
      ++num_clauses_read;
    }

    cursor = buffer.begin() + (next - buffer.c_str());
  }

  return {num_clauses_read, lit_buffer};
}


constexpr size_t default_chunk_size = (1 << 16);

template <typename UnaryFn>
auto parse_cnf_gz_file(cnf_gz_file& file, UnaryFn&& clause_receiver)
{
  std::string const header_line = file.read_header_line();
  problem_header header = parse_cnf_header_line(header_line);

  std::string buffer;
  parse_cnf_chunk_result chunk_result =
      parse_cnf_chunk(header_line, header.header_size, {}, clause_receiver);
  size_t num_clauses = chunk_result.num_clauses_read;

  while (!file.is_eof()) {
    file.read_chunk(default_chunk_size, buffer);
    chunk_result = parse_cnf_chunk(buffer, 0, chunk_result.open_clause, clause_receiver);
    num_clauses += chunk_result.num_clauses_read;
  }

  if (num_clauses != header.num_clauses) {
    throw std::invalid_argument{"invalid amount of clauses in CNF data"};
  }

  if (!chunk_result.open_clause.empty()) {
    throw std::invalid_argument{"cnf data ended in open clause"};
  }
}
}

template <typename UnaryFn>
void parse_cnf_file(std::filesystem::path const& input_file, UnaryFn&& clause_receiver)
{
  using namespace detail;
  cnf_gz_file file{input_file};
  parse_cnf_gz_file(file, clause_receiver);
}

template <typename UnaryFn>
void parse_cnf_from_stdin(UnaryFn&& clause_receiver)
{
  using namespace detail;
  cnf_gz_file stdin_file;
  parse_cnf_fz_file(stdin_file, clause_receiver);
}

template <typename UnaryFn>
void parse_cnf_string(std::string const& cnf, UnaryFn&& clause_receiver)
{
  using namespace detail;

  problem_header header = parse_cnf_header_line(cnf);
  auto const result = parse_cnf_chunk(cnf, header.header_size, {}, clause_receiver);

  if (result.num_clauses_read != header.num_clauses) {
    throw std::invalid_argument{"invalid amount of clauses in CNF data"};
  }

  if (!result.open_clause.empty()) {
    throw std::invalid_argument{"cnf data ended in open clause"};
  }
}
}
