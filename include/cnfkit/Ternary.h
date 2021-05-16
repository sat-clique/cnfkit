#pragma once

#include <cstdint>

namespace cnfkit {

class tbool {
public:
  constexpr tbool() : m_value(0) {}
  constexpr tbool(uint8_t value) : m_value(value) {}

  constexpr auto operator&(tbool const& rhs) const -> tbool;
  constexpr auto operator|(tbool const& rhs) const -> tbool;
  constexpr auto operator!() const -> tbool;

  constexpr operator bool() const;

  constexpr auto operator==(tbool const& rhs) const -> bool;
  constexpr auto operator!=(tbool const& rhs) const -> bool;

private:
  uint8_t m_value;
};

constexpr tbool t_false = tbool{0u};
constexpr tbool t_true = tbool{1u};
constexpr tbool t_indet = tbool{2u};

constexpr auto to_tbool(bool it) -> tbool;
}

#include "cnfkit/TernaryImpl.h"
