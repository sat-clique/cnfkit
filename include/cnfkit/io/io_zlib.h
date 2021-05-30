#pragma once

#include <cnfkit/detail/check_cxx_version.h>

#include <cnfkit/io.h>

#include <zlib.h>

#include <cstddef>
#include <filesystem>
#include <optional>

namespace cnfkit {
class zlib_source final : public source {
public:
  explicit zlib_source(std::filesystem::path const& path);
  zlib_source();

  auto read_bytes(std::byte* buf_start, std::byte* buf_stop) -> std::byte* override;
  auto read_byte() -> std::optional<std::byte> override;
  auto is_eof() -> bool override;

  virtual ~zlib_source();

private:
  gzFile m_file = nullptr;
};

/** Implementation **/

inline zlib_source::zlib_source(std::filesystem::path const& path)
{
  m_file = gzopen(path.string().data(), "rb");
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
  gzclose(m_file);
}

inline auto zlib_source::read_bytes(std::byte* buf_start, std::byte* buf_stop) -> std::byte*
{
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
  return gzeof(m_file) != 0;
}
}
