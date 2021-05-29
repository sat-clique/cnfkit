#pragma once

#include <cnfkit/detail/CnfLikeParser.h>

#include <zlib.h>

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace cnfkit {
enum class drat_format { text, binary };
}

namespace cnfkit::detail {

template <typename It>
auto parse_drat_binary_lit(It start, It stop) -> std::pair<lit, It>
{
  if (start == stop) {
    throw std::invalid_argument{"invalid binary drat literal"};
  }

  uint32_t raw_lit = 0;
  uint32_t shift = 0;
  bool found_end = false;
  It cursor = start;

  do {
    if (shift == 28 && (*cursor & 0x7F) > 0x0F) {
      throw std::invalid_argument{"literal out of range"};
    }

    raw_lit |= ((*cursor & 0x7F) << shift);
    shift += 7;
    found_end = (*cursor & 0x80) == 0;
    ++cursor;
  } while (!found_end && cursor != stop && shift <= 28);

  if (!found_end) {
    throw std::invalid_argument{"unexpected end of binary drat literal"};
  }

  uint32_t const raw_var = raw_lit / 2;
  if (raw_var == 0) {
    throw std::invalid_argument{"invalid variable 0"};
  }
  var const cnfkit_var = var{raw_var - 1};

  lit result{cnfkit_var, (raw_lit & 1) == 0};

  return {result, cursor};
}

class drat_binary_chunk_parser {
public:
  template <typename It, typename BinaryFn>
  void parse(It start, It stop, BinaryFn&& clause_receiver)
  {
    It cursor = start;
    while (cursor != stop) {
      if (*cursor == 0x61) {
        m_is_in_add_mode = true;
        m_is_in_clause = true;
        ++cursor;
      }
      else if (*cursor == 0x64) {
        m_is_in_add_mode = false;
        m_is_in_clause = true;
        ++cursor;
      }
      else if (*cursor == 0) {
        if (!m_is_in_clause) {
          throw std::invalid_argument{"clause not preceded by a or d"};
        }

        clause_receiver(m_is_in_add_mode, m_lit_buffer);
        m_is_in_clause = false;
        m_lit_buffer.clear();
        ++cursor;
      }
      else {
        if (!m_is_in_clause) {
          throw std::invalid_argument{"clause not preceded by a or d"};
        }

        auto const [lit, next] = parse_drat_binary_lit(cursor, stop);
        cursor = next;
        m_lit_buffer.push_back(lit);
      }
    }
  }

  void check_on_drat_finish()
  {
    if (m_is_in_clause) {
      throw std::invalid_argument{"unexpected end of proof"};
    }
  }

private:
  std::vector<lit> m_lit_buffer;
  bool m_is_in_add_mode = false;
  bool m_is_in_clause = false;
};

// TODO: create utility class for gz file access, unify with cnf_gz_file
class drat_binary_input_file {
public:
  drat_binary_input_file(std::filesystem::path const& file)
  {
    m_file = gzopen(file.string().data(), "rb");
    if (m_file == nullptr) {
      std::perror(file.string().data());
      throw std::runtime_error{"Could not open input file."};
    }
  }

  drat_binary_input_file()
  {
    m_file = gzdopen(0, "rb");
    if (m_file == nullptr) {
      throw std::runtime_error{"Could not open stdin for reading."};
    }
  }

  ~drat_binary_input_file()
  {
    if (m_file != nullptr) {
      gzclose(m_file);
    }
  }

  auto is_eof() const -> bool { return gzeof(m_file); }

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

  auto read_chunk(size_t desired_size) -> std::vector<unsigned char> const&
  {
    m_buffer.resize(desired_size);
    int bytes_read = ::gzread(m_file, m_buffer.data(), desired_size);

    if (bytes_read < 0) {
      int errnum = 0;
      char const* message = gzerror(m_file, &errnum);
      throw std::runtime_error{message};
    }

    m_buffer.resize(bytes_read);

    // stopped in the middle of a literal ~> read rest, too
    // TODO: only read at most 4 additional chars
    if (!m_buffer.empty()) {
      while (!is_eof() && (m_buffer.back() & 0x80) != 0) {
        m_buffer.push_back(*read_char());
      }
    }

    return m_buffer;
  }


  drat_binary_input_file(drat_binary_input_file const& rhs) = delete;
  auto operator=(drat_binary_input_file const& rhs) -> drat_binary_input_file& = delete;
  drat_binary_input_file(drat_binary_input_file&& rhs) = default;
  auto operator=(drat_binary_input_file&& rhs) -> drat_binary_input_file& = default;

private:
  std::vector<unsigned char> m_buffer;
  gzFile m_file;
};

template <typename BinaryFn>
auto parse_drat_text_gz_file(cnf_gz_file& file, BinaryFn&& clause_receiver)
{
  cnf_chunk_parser parser{cnf_chunk_parser_mode::drat};

  std::string buffer;
  while (!file.is_eof()) {
    file.read_chunk(default_chunk_size, buffer);
    parser.parse(buffer, 0, clause_receiver);
  }

  parser.check_on_drat_finish();
}

template <typename BinaryFn>
auto parse_drat_binary_gz_file(drat_binary_input_file& file, BinaryFn&& clause_receiver)
{
  drat_binary_chunk_parser parser;
  while (!file.is_eof()) {
    auto const& buffer = file.read_chunk(default_chunk_size);
    parser.parse(buffer.begin(), buffer.end(), clause_receiver);
  }

  parser.check_on_drat_finish();
}

template <typename BinaryFn>
void parse_drat_file_impl(std::filesystem::path const& input_file,
                          drat_format format,
                          BinaryFn&& clause_receiver)
{
  if (format == drat_format::binary) {
    drat_binary_input_file file{input_file};
    parse_drat_binary_gz_file(file, clause_receiver);
  }
  else {
    cnf_gz_file file{input_file};
    parse_drat_text_gz_file(file, clause_receiver);
  }
}

template <typename BinaryFn>
void parse_drat_from_stdin_impl(drat_format format, BinaryFn&& clause_receiver)
{
  if (format == drat_format::binary) {
    drat_binary_input_file stdin_file;
    parse_drat_binary_gz_file(stdin_file, clause_receiver);
  }
  else {
    cnf_gz_file stdin_file;
    parse_drat_text_gz_file(stdin_file, clause_receiver);
  }
}

template <typename BinaryFn>
void parse_drat_string_impl(std::string const& drat, drat_format format, BinaryFn&& clause_receiver)
{
  if (format == drat_format::binary) {
    drat_binary_chunk_parser parser;
    parser.parse(drat.begin(), drat.end(), clause_receiver);
    parser.check_on_drat_finish();
  }
  else {
    cnf_chunk_parser parser{cnf_chunk_parser_mode::drat};
    parser.parse(drat, 0, clause_receiver);
    parser.check_on_drat_finish();
  }
}
}
