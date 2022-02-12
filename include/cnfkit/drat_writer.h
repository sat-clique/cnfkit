#pragma once

/**
 * \file
 */

#include <cnfkit/detail/check_cxx_version.h>

#include <cnfkit/io.h>
#include <cnfkit/literal.h>

#include <array>
#include <charconv>
#include <cstddef>
#include <cstring>
#include <vector>

/**
 * \defgroup drat_writers DRAT Proof Writers
 *
 * \brief Writers for DRAT proofs
 */

namespace cnfkit {

/**
 * \brief Interface for DRAT writers.
 *
 * \ingroup drat_writers
 */
class drat_writer {
public:
  /**
   * \brief Adds the given clause to the proof, marking it as added.
   *
   * If the given clause is non-empty, its first literal must be the pivot
   * literal.
   *
   * \throws std::runtime_error     Thrown on I/O failure.
   * \throws std::invalid_argument  Thrown when a literal cannot be represented in the
   *                                supported range of DIMACS literals (see
   *                                `to_dimacs_lit()`).
   */
  virtual void add_clause(lit const* start, lit const* stop) = 0;

  /**
   * \brief Adds the given clause to the proof, marking it as deleted.
   *
   * \throws std::runtime_error     Thrown on I/O failure.
   * \throws std::invalid_argument  Thrown when a literal cannot be represented in the
   *                                supported range of DIMACS literals (see
   *                                `to_dimacs_lit()`).
   */
  virtual void del_clause(lit const* start, lit const* stop) = 0;

  /**
   * \brief Flushes the sink backing the writer.
   *
   * \throws std::runtimer_error    Thrown on I/O failure.
   */
  virtual void flush() = 0;

  virtual ~drat_writer() = default;
};

/**
 * \brief drat_writer implementation writing proofs in the text DRAT format.
 *
 * \ingroup drat_writers
 */
class drat_text_writer final : public drat_writer {
public:
  drat_text_writer(sink& sink);
  void add_clause(lit const* start, lit const* stop) override;
  void del_clause(lit const* start, lit const* stop) override;
  void flush() override;

  auto operator=(drat_text_writer const&) -> drat_text_writer& = delete;
  drat_text_writer(drat_text_writer const&) = delete;
  auto operator=(drat_text_writer&&) noexcept -> drat_text_writer& = default;
  drat_text_writer(drat_text_writer&&) noexcept = default;

private:
  void write_clause(char prefix, lit const* start, lit const* stop);
  void write_lit(lit literal);

  void begin_clause(char prefix);
  void end_clause();

  sink* m_sink;
  std::vector<std::byte> m_buffer;
};

/**
 * \brief drat_writer implementation writing proofs in the binary DRAT format.
 *
 * \ingroup drat_writers
 */
class drat_binary_writer final : public drat_writer {
public:
  drat_binary_writer(sink& sink);
  void add_clause(lit const* start, lit const* stop) override;
  void del_clause(lit const* start, lit const* stop) override;
  void flush() override;

  auto operator=(drat_binary_writer const&) -> drat_binary_writer& = delete;
  drat_binary_writer(drat_binary_writer const&) = delete;
  auto operator=(drat_binary_writer&&) noexcept -> drat_binary_writer& = default;
  drat_binary_writer(drat_binary_writer&&) noexcept = default;

private:
  void write_clause(char prefix, lit const* start, lit const* stop);
  void write_lit(lit literal);

  void begin_clause(char prefix);
  void end_clause();

  sink* m_sink;
  std::vector<std::byte> m_buffer;
};


// *** Implementation ***

inline drat_text_writer::drat_text_writer(sink& sink) : m_sink{&sink} {}

inline void drat_text_writer::add_clause(lit const* start, lit const* stop)
{
  write_clause('a', start, stop);
}

inline void drat_text_writer::del_clause(lit const* start, lit const* stop)
{
  write_clause('d', start, stop);
}

inline void drat_text_writer::flush()
{
  m_sink->flush();
}

inline void drat_text_writer::write_clause(char prefix, lit const* start, lit const* stop)
{
  m_buffer.clear();

  begin_clause(prefix);
  for (lit const* cursor = start; cursor != stop; ++cursor) {
    write_lit(*cursor);
  }
  end_clause();

  m_sink->write_bytes(m_buffer.data(), m_buffer.data() + m_buffer.size());
}

inline void drat_text_writer::write_lit(lit literal)
{
  std::array<char, 11> buffer;
  auto [ptr, ec] =
      std::to_chars(buffer.data(), buffer.data() + buffer.size(), lit_to_dimacs(literal));
  size_t const old_size = m_buffer.size();
  size_t const lit_size = ptr - buffer.begin();
  m_buffer.resize(m_buffer.size() + lit_size + 1);
  std::memcpy(m_buffer.data() + old_size, buffer.data(), lit_size);
  m_buffer.back() = std::byte{' '};
}

inline void drat_text_writer::begin_clause(char prefix)
{
  if (prefix == 'd') {
    m_buffer.push_back(std::byte('d'));
    m_buffer.push_back(std::byte(' '));
  }
}

inline void drat_text_writer::end_clause()
{
  m_buffer.push_back(std::byte('0'));
  m_buffer.push_back(std::byte('\n'));
}

inline drat_binary_writer::drat_binary_writer(sink& sink) : m_sink{&sink} {}

inline void drat_binary_writer::add_clause(lit const* start, lit const* stop)
{
  write_clause('a', start, stop);
}

inline void drat_binary_writer::del_clause(lit const* start, lit const* stop)
{
  write_clause('d', start, stop);
}

inline void drat_binary_writer::flush()
{
  m_sink->flush();
}

inline void drat_binary_writer::write_clause(char prefix, lit const* start, lit const* stop)
{
  m_buffer.clear();

  begin_clause(prefix);
  for (lit const* cursor = start; cursor != stop; ++cursor) {
    write_lit(*cursor);
  }
  end_clause();

  m_sink->write_bytes(m_buffer.data(), m_buffer.data() + m_buffer.size());
}

inline void drat_binary_writer::write_lit(lit literal)
{
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

inline void drat_binary_writer::begin_clause(char prefix)
{
  m_buffer.push_back(std::byte(prefix));
}

inline void drat_binary_writer::end_clause()
{
  m_buffer.push_back(std::byte(0));
}

}
