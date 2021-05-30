#pragma once

/**
 * \file
 */

#include <cnfkit/detail/check_cxx_version.h>

#include <cstddef>
#include <optional>

/**
 * \defgroup io I/O Utilities
 *
 * \brief Simple stream-like classes and related utilities
 */

namespace cnfkit {

/**
 * \brief Interface for objects containing data to be parsed.
 *
 * \ingroup io
 */
class source {
public:
  /**
   * \brief Reads up to `buf_stop - buf_start` bytes into the buffer starting with
   * `buf_start`.
   *
   * The pointer past the end of the actually written data is
   * returned. The provided buffer is filled unless the source has reached EOF.
   *
   * \throws std::runtime_error   Thrown on I/O failure.
   */
  virtual auto read_bytes(std::byte* buf_start, std::byte* buf_stop) -> std::byte* = 0;

  /**
   * \brief Reads a single byte from the source.
   *
   * If the file has reached EOF, nothing is returned.
   *
   * \throws std::runtime_error   Thrown on I/O failure.
   */
  virtual auto read_byte() -> std::optional<std::byte> = 0;

  /**
   * \brief Returns true if and only if the source has reached EOF.
   *
   * \throws std::runtime_error   Thrown on I/O failure.
   */
  virtual auto is_eof() -> bool = 0;

  virtual ~source() = default;
};

/**
 * \brief Interface for objects receiving serialized data.
 *
 * \ingroup io
 */
class sink {
public:
  /**
   * \brief Writes the given range of bytes to the sink.
   *
   * \throws std::runtime_error   Thrown on I/O failure.
   */
  virtual void write_bytes(std::byte const* start, std::byte const* stop) = 0;

  /**
   * \brief Flushes the sink. This causes underlying buffers/files to be
   * flushed as well.
   *
   * \throws std::runtime_error   Thrown on I/O failure
   */
  virtual void flush() = 0;

  virtual ~sink() = default;
};
}
