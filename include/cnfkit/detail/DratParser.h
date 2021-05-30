#pragma once

#include <cnfkit/Drat.h>

#include <cnfkit/detail/CnfLikeParser.h>

#include <zlib.h>

#include <array>
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace cnfkit::detail {

inline auto parse_drat_binary_lit(std::byte const* start, std::byte const* stop)
    -> std::pair<lit, std::byte const*>
{
  if (start == stop) {
    throw std::invalid_argument{"invalid binary drat literal"};
  }

  uint32_t raw_lit = 0;
  uint32_t shift = 0;
  bool found_end = false;
  std::byte const* cursor = start;

  do {
    if (shift == 28 && (*cursor & std::byte{0x70}) != std::byte{0}) {
      throw std::invalid_argument{"literal out of range"};
    }

    raw_lit |= (std::to_integer<uint32_t>(*cursor & std::byte{0x7F}) << shift);
    shift += 7;
    found_end = (*cursor & std::byte{0x80}) == std::byte{0};
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

  return std::make_pair(result, cursor);
}

class drat_binary_chunk_parser {
public:
  template <typename BinaryFn>
  void parse(std::byte const* start, std::byte const* stop, BinaryFn&& clause_receiver)
  {
    std::byte const* cursor = start;
    while (cursor != stop) {
      if (*cursor == std::byte{0x61}) {
        m_is_in_add_mode = true;
        m_is_in_clause = true;
        ++cursor;
      }
      else if (*cursor == std::byte{0x64}) {
        m_is_in_add_mode = false;
        m_is_in_clause = true;
        ++cursor;
      }
      else if (*cursor == std::byte{0}) {
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

  auto read_byte() -> std::optional<std::byte>
  {
    unsigned char character = 0;
    int chars_read = gzread(m_file, &character, 1);

    if (chars_read == 0 && is_eof()) {
      return std::nullopt;
    }

    if (chars_read <= 0) {
      int errnum = 0;
      char const* message = gzerror(m_file, &errnum);
      throw std::runtime_error{message};
    }

    return std::byte{character};
  }

  auto read_chunk(size_t desired_size) -> std::vector<std::byte> const&
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
      while (!is_eof() && (m_buffer.back() & std::byte{0x80}) != std::byte{0}) {
        m_buffer.push_back(*read_byte());
      }
    }

    return m_buffer;
  }


  drat_binary_input_file(drat_binary_input_file const& rhs) = delete;
  auto operator=(drat_binary_input_file const& rhs) -> drat_binary_input_file& = delete;
  drat_binary_input_file(drat_binary_input_file&& rhs) = default;
  auto operator=(drat_binary_input_file&& rhs) -> drat_binary_input_file& = default;

private:
  std::vector<std::byte> m_buffer;
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
    parser.parse(buffer.data(), buffer.data() + buffer.size(), clause_receiver);
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
void parse_drat_string_impl(std::string const& drat, BinaryFn&& clause_receiver)
{
  cnf_chunk_parser parser{cnf_chunk_parser_mode::drat};
  parser.parse(drat, 0, clause_receiver);
  parser.check_on_drat_finish();
}

template <typename BinaryFn>
void parse_drat_binary_buffer_impl(std::byte const* start,
                                   std::byte const* stop,
                                   BinaryFn&& clause_receiver)
{
  drat_binary_chunk_parser parser;
  parser.parse(start, stop, clause_receiver);
  parser.check_on_drat_finish();
}
}
