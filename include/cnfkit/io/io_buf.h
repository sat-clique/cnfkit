#pragma once

#include <cnfkit/detail/check_cxx_version.h>

#include <cnfkit/io.h>

#include <cstddef>
#include <optional>
#include <string>

namespace cnfkit {
class buf_source final : public source {
public:
  explicit buf_source(std::string const& str);
  explicit buf_source(std::byte const* start, std::byte const* stop);

  auto read_bytes(std::byte* buf_start, std::byte* buf_stop) -> std::byte* override;
  auto read_byte() -> std::optional<std::byte> override;
  auto is_eof() -> bool override;

  auto operator=(buf_source const&) noexcept -> buf_source& = default;
  buf_source(buf_source const&) noexcept = default;
  auto operator=(buf_source&&) noexcept -> buf_source& = default;
  buf_source(buf_source&&) noexcept = default;

private:
  std::byte const* m_cursor = nullptr;
  size_t m_remaining_size = 0;
};

// *** Implementation ***

inline buf_source::buf_source(std::string const& str)
  : m_cursor{reinterpret_cast<std::byte const*>(str.data())}
  , m_remaining_size{static_cast<size_t>(str.size())}
{
}

inline buf_source::buf_source(std::byte const* start, std::byte const* stop)
  : m_cursor{start}, m_remaining_size{static_cast<size_t>(stop - start)}
{
}

inline auto buf_source::read_bytes(std::byte* buf_start, std::byte* buf_stop) -> std::byte*
{
  size_t const num_bytes_desired = (buf_stop - buf_start);
  size_t const to_copy = std::min(num_bytes_desired, m_remaining_size);
  std::copy(m_cursor, m_cursor + to_copy, buf_start);
  m_remaining_size -= to_copy;
  m_cursor += to_copy;
  return buf_start + to_copy;
}

inline auto buf_source::read_byte() -> std::optional<std::byte>
{
  if (m_remaining_size > 0) {
    std::byte const result = *m_cursor;
    ++m_cursor;
    --m_remaining_size;
    return result;
  }
  else {
    return std::nullopt;
  }
}

inline auto buf_source::is_eof() -> bool
{
  return m_remaining_size == 0;
}
}
