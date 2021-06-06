#pragma once

#include <cnfkit/detail/check_cxx_version.h>

#include <cnfkit/io.h>

#include <cstddef>
#include <istream>
#include <optional>
#include <ostream>

namespace cnfkit {

/**
 * \brief Reader for (uncompressed) std::istream objects.
 *
 * \ingroup io
 */
class istream_source : public source {
public:
  explicit istream_source(std::istream& buf);

  auto read_bytes(std::byte* start, std::byte* stop) -> std::byte* override;
  auto read_byte() -> std::optional<std::byte> override;
  auto is_eof() -> bool override;

  auto operator=(istream_source const&) -> istream_source& = delete;
  istream_source(istream_source const&) = delete;

  auto operator=(istream_source&& rhs) noexcept -> istream_source& = default;
  istream_source(istream_source&&) noexcept = default;

private:
  std::istream* m_input = nullptr;
};

/**
 * \brief Non-compressing writer for std::ostream objects.
 *
 * \ingroup io
 */

class ostream_sink : public sink {
public:
  explicit ostream_sink(std::ostream& buf);
  void write_bytes(std::byte const* start, std::byte const* stop) override;
  void flush() override;

  auto operator=(ostream_sink const&) -> ostream_sink& = delete;
  ostream_sink(ostream_sink const&) = delete;

  auto operator=(ostream_sink&& rhs) noexcept -> ostream_sink& = default;
  ostream_sink(ostream_sink&& rhs) noexcept = default;

private:
  std::ostream* m_output = nullptr;
};


// *** Implementation ***

inline istream_source::istream_source(std::istream& buf) : m_input{&buf} {}

inline auto istream_source::read_bytes(std::byte* start, std::byte* stop) -> std::byte*
{
  if (m_input == nullptr) {
    return start;
  }

  m_input->read(reinterpret_cast<char*>(start), std::distance(start, stop));

  if (m_input->fail() && !m_input->eof()) {
    throw std::runtime_error{"read error"};
  }

  return start + m_input->gcount();
}

inline auto istream_source::read_byte() -> std::optional<std::byte>
{
  if (m_input == nullptr) {
    return std::nullopt;
  }

  auto result = m_input->get();

  if (result == std::istream::traits_type::eof()) {
    return std::nullopt;
  }

  if (m_input->fail()) {
    throw std::runtime_error{"read error"};
  }
  else {
    return static_cast<std::byte>(result);
  }
}

inline auto istream_source::is_eof() -> bool
{
  return m_input != nullptr ? m_input->eof() : true;
}

inline ostream_sink::ostream_sink(std::ostream& buf) : m_output{&buf} {}

inline void ostream_sink::write_bytes(std::byte const* start, std::byte const* stop)
{
  if (m_output == nullptr) {
    return;
  }

  m_output->write(reinterpret_cast<char const*>(start), std::distance(start, stop));
  if (m_output->fail()) {
    throw std::runtime_error{"I/O error"};
  }
}

inline void ostream_sink::flush()
{
  if (m_output == nullptr) {
    return;
  }

  m_output->flush();
  if (m_output->fail()) {
    throw std::runtime_error{"I/O error"};
  }
}
}
