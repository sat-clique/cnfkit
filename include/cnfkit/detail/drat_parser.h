#pragma once

#include <cnfkit/detail/cnflike_parser.h>
#include <cnfkit/io.h>

#include <array>
#include <cstddef>
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

class drat_source_reader {
public:
  drat_source_reader(source& source) : m_source{source} {}

  auto is_eof() -> bool { return m_source.is_eof(); }

  auto read_chunk(size_t desired_size) -> std::vector<std::byte> const&
  {
    m_buffer.resize(desired_size);
    std::byte* byte_buffer = reinterpret_cast<std::byte*>(m_buffer.data());
    std::byte* read_stop = m_source.read_bytes(byte_buffer, byte_buffer + desired_size);
    m_buffer.resize(read_stop - byte_buffer);

    // stopped in the middle of a literal ~> read rest, too
    // TODO: only read at most 4 additional chars
    if (!m_buffer.empty()) {
      while (!is_eof() && (m_buffer.back() & std::byte{0x80}) != std::byte{0}) {
        m_buffer.push_back(*m_source.read_byte());
      }
    }

    return m_buffer;
  }

private:
  std::vector<std::byte> m_buffer;
  source& m_source;
};
}
