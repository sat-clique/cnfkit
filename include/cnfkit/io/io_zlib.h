#pragma once

/**
 * \file
 */

#include <cnfkit/detail/check_cxx_version.h>

#if !__has_include(<zlib.h>)
#error "zlib.h not found. The headers of zlib must be added to the include search path."
#endif

#include <cnfkit/io.h>

#include <zlib.h>

#include <cstddef>
#include <filesystem>
#include <optional>

namespace cnfkit {

/**
 * \brief Reader for files that are uncompressed or gz-compressed.
 *
 * This class uses the zlib library. The reader may be used to read
 * gzip-compressed, deflate-compressed or uncompressed files.
 *
 * \ingroup io
 */
class zlib_source final : public source {
public:
  /**
   * \brief Constructs a zlib_source object backed by the given file.
   *
   * \throws std::runtime_error   Thrown on I/O failure or if the file is
   *                              corrupt.
   */
  explicit zlib_source(std::filesystem::path const& path);

  /**
   * \brief Constructs a zlib_source object reading from stdin.
   *
   * \throws std::runtime_error   Thrown on I/O failure.
   */
  zlib_source();

  auto read_bytes(std::byte* buf_start, std::byte* buf_stop) -> std::byte* override;
  auto read_byte() -> std::optional<std::byte> override;
  auto is_eof() -> bool override;

  virtual ~zlib_source();

  auto operator=(zlib_source const&) -> zlib_source& = delete;
  zlib_source(zlib_source const&) = delete;

  auto operator=(zlib_source&& rhs) noexcept -> zlib_source&;
  zlib_source(zlib_source&& rhs) noexcept;

private:
  gzFile m_file = nullptr;
};

/** Implementation **/

inline zlib_source::zlib_source(std::filesystem::path const& path)
{
#ifdef WIN32
  // Avoiding lossy conversion to non-UTF narrow string encoding
  m_file = gzopen_w(path.wstring().data(), "rb");
#else
  m_file = gzopen(path.string().data(), "rb");
#endif

  if (m_file == nullptr) {
    std::perror(path.string().data());
    throw std::runtime_error{"Could not open input file."};
  }
}

inline zlib_source::zlib_source()
{
  m_file = gzdopen(0, "rb");
  if (m_file == nullptr) {
    throw std::runtime_error{"Could not open input file."};
  }
}

inline zlib_source::~zlib_source()
{
  if (m_file != nullptr) {
    gzclose(m_file);
  }
}

inline auto zlib_source::read_bytes(std::byte* buf_start, std::byte* buf_stop) -> std::byte*
{
  if (m_file == nullptr) {
    return buf_start;
  }

  int const bytes_read = gzread(m_file, buf_start, buf_stop - buf_start);
  if (bytes_read < 0) {
    int errnum = 0;
    char const* message = gzerror(m_file, &errnum);
    throw std::runtime_error{message};
  }

  return buf_start + bytes_read;
}

inline auto zlib_source::read_byte() -> std::optional<std::byte>
{
  if (m_file == nullptr) {
    return std::nullopt;
  }

  std::byte result;
  int bytes_read = gzread(m_file, &result, 1);
  if (bytes_read == 0 && is_eof()) {
    return std::nullopt;
  }
  else if (bytes_read <= 0) {
    int errnum = 0;
    char const* message = gzerror(m_file, &errnum);
    throw std::runtime_error{message};
  }

  return result;
}

inline auto zlib_source::is_eof() -> bool
{
  if (m_file == nullptr) {
    return true;
  }

  return gzeof(m_file) != 0;
}

inline auto zlib_source::operator=(zlib_source&& rhs) noexcept -> zlib_source&
{
  std::swap(m_file, rhs.m_file);
  return *this;
}

inline zlib_source::zlib_source(zlib_source&& rhs) noexcept
{
  std::swap(m_file, rhs.m_file);
}
}
