#pragma once

#include <cnfkit/detail/CheckCxxVersion.h>

#include <cstddef>

namespace cnfkit {
class sink {
public:
  virtual void write_bytes(std::byte const* start, std::byte const* stop) = 0;
  virtual void flush() = 0;
  virtual ~sink() = default;
};
}

