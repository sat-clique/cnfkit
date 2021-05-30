#pragma once

#include <cnfkit/detail/check_cxx_version.h>

#include <cstddef>
#include <optional>

namespace cnfkit {
class source {
public:
  virtual auto read_bytes(std::byte* buf_start, std::byte* buf_stop) -> std::byte* = 0;
  virtual auto read_byte() -> std::optional<std::byte> = 0;
  virtual auto is_eof() -> bool = 0;
  virtual ~source() = default;
};

class sink {
public:
  virtual void write_bytes(std::byte const* start, std::byte const* stop) = 0;
  virtual void flush() = 0;
  virtual ~sink() = default;
};
}
