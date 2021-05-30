#pragma once

#include <cnfkit/detail/CheckCxxVersion.h>

#include <cnfkit/Drat.h>
#include <cnfkit/Literal.h>
#include <cnfkit/Sink.h>

#include <array>
#include <charconv>
#include <cstddef>
#include <vector>


// TODO: extract sink to separate header & implement file + string sink

namespace cnfkit {

class drat_writer {
public:
  drat_writer(sink& sink, drat_format format);
  void add_clause(lit const* start, lit const* stop);
  void del_clause(lit const* start, lit const* stop);
  void flush();

private:
  void write_clause(char prefix, lit const* start, lit const* stop);
  void write_lit(lit literal);

  void begin_clause(char prefix);
  void end_clause();

  sink* m_sink;
  drat_format m_format;
  std::vector<std::byte> m_buffer;
};

// *** Implementation ***

drat_writer::drat_writer(sink& sink, drat_format format) : m_sink{&sink}, m_format{format} {}

inline void drat_writer::add_clause(lit const* start, lit const* stop)
{
  write_clause('a', start, stop);
}

inline void drat_writer::del_clause(lit const* start, lit const* stop)
{
  write_clause('d', start, stop);
}

inline void drat_writer::flush()
{
  m_sink->flush();
}

inline void drat_writer::write_clause(char prefix, lit const* start, lit const* stop)
{
  m_buffer.clear();

  begin_clause(prefix);
  for (lit const* cursor = start; cursor != stop; ++cursor) {
    write_lit(*cursor);
  }
  end_clause();

  m_sink->write_bytes(m_buffer.data(), m_buffer.data() + m_buffer.size());
}

inline void drat_writer::write_lit(lit literal)
{
  if (m_format == drat_format::binary) {
    std::array<std::byte, 6> buffer;

    uint32_t binary_lit =
        (literal.get_var().get_raw_value() + 1) * 2 + (literal.is_positive() ? 0 : 1);
    size_t index = 0;

    while (binary_lit != 0) {
      buffer[index] = std::byte((binary_lit & 0xff) | 0x80);
      binary_lit = binary_lit >> 7;
      ++index;
    }

    buffer[index - 1] &= std::byte(0x7f);
    m_buffer.insert(m_buffer.end(), buffer.begin(), buffer.begin() + index);
  }
  else {
    std::array<char, 11> buffer;
    auto [ptr, ec] =
        std::to_chars(buffer.data(), buffer.data() + buffer.size(), lit_to_dimacs(literal));
    size_t const old_size = m_buffer.size();
    size_t const lit_size = ptr - buffer.begin();
    m_buffer.resize(m_buffer.size() + lit_size + 1);
    std::memcpy(m_buffer.data() + old_size, buffer.data(), lit_size);
    m_buffer.back() = std::byte{' '};
  }
}

inline void drat_writer::begin_clause(char prefix)
{
  if (m_format == drat_format::binary) {
    m_buffer.push_back(std::byte(prefix));
  }

  if (m_format == drat_format::text && prefix == 'd') {
    m_buffer.push_back(std::byte('d'));
    m_buffer.push_back(std::byte(' '));
  }
}

inline void drat_writer::end_clause()
{
  if (m_format == drat_format::binary) {
    m_buffer.push_back(std::byte(0));
  }
  else {
    m_buffer.push_back(std::byte('0'));
    m_buffer.push_back(std::byte('\n'));
  }
}
}
