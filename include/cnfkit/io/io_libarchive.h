#pragma once

/**
 * \file
 */

#include <cnfkit/detail/check_cxx_version.h>

#include <cnfkit/io.h>

#include <archive.h>
#include <archive_entry.h>

#include <filesystem>
#include <stdexcept>

namespace cnfkit {
class libarchive_source : public source {
public:
  explicit libarchive_source(std::filesystem::path const& path);
  ~libarchive_source();

  auto read_bytes(std::byte* start, std::byte* stop) -> std::byte* override;
  auto read_byte() -> std::optional<std::byte> override;
  auto is_eof() -> bool override;

  auto operator=(libarchive_source const&) -> libarchive_source& = delete;
  libarchive_source(libarchive_source const&) = delete;

  auto operator=(libarchive_source&& rhs) noexcept -> libarchive_source&;
  libarchive_source(libarchive_source&& rhs) noexcept;

private:
  void close_and_throw(char const* message);

  archive* m_file = nullptr;
  bool m_eof = false;
};

// *** Implementation ***

libarchive_source::libarchive_source(std::filesystem::path const& path)
{
  m_file = archive_read_new();
  if (m_file == nullptr) {
    throw std::bad_alloc{};
  }

  archive_read_support_filter_all(m_file);
  archive_read_support_format_raw(m_file);

  size_t const buffer_size = 16384;
  // TODO: this won't work properly on Windows (lossy narrow string conversion)
  if (archive_read_open_filename(m_file, path.string().c_str(), buffer_size) != ARCHIVE_OK) {
    close_and_throw(archive_error_string(m_file));
  }

  archive_entry* entry = nullptr;
  int const read_header_result = archive_read_next_header(m_file, &entry);
  if (read_header_result == ARCHIVE_EOF) {
    m_eof = true;
  }
  else if (read_header_result != ARCHIVE_OK) {
    close_and_throw(archive_error_string(m_file));
  }
}

libarchive_source::~libarchive_source()
{
  if (m_file != nullptr) {
    archive_read_free(m_file);
  }
}

void libarchive_source::close_and_throw(char const* message)
{
  if (m_file != nullptr) {
    archive_read_free(m_file);
    m_file = nullptr;
  }
  m_eof = true;
  throw std::runtime_error(message);
}

auto libarchive_source::read_bytes(std::byte* start, std::byte* stop) -> std::byte*
{
  if (m_file == nullptr) {
    return start;
  }

  int result =
      archive_read_data(m_file, reinterpret_cast<char*>(start), std::distance(start, stop));
  if (result == 0) {
    m_eof = true;
  }
  else if (result < 0) {
    // TODO: deal with ARCHIVE_WARN and ARCHIVE_RETRY
    close_and_throw(archive_error_string(m_file));
  }
  return start + result;
}

auto libarchive_source::read_byte() -> std::optional<std::byte>
{
  std::byte buf;
  std::byte* result = read_bytes(&buf, &buf + 1);
  if (result == &buf) {
    return std::nullopt;
  }
  return buf;
}

auto libarchive_source::is_eof() -> bool
{
  return m_eof;
}

auto libarchive_source::operator=(libarchive_source&& rhs) noexcept -> libarchive_source&
{
  m_file = rhs.m_file;
  m_eof = rhs.m_eof;
  rhs.m_file = nullptr;
  rhs.m_eof = true;
  return *this;
}

libarchive_source::libarchive_source(libarchive_source&& rhs) noexcept
  : m_file{rhs.m_file}, m_eof{rhs.m_eof}
{
  rhs.m_file = nullptr;
  rhs.m_eof = true;
}

}
