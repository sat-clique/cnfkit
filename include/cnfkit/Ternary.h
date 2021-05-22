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


// *** Implementation ***

constexpr auto tbool::operator&(tbool const& rhs) const -> tbool
{
  return (*this == t_true && rhs == t_true)
             ? t_true
             : ((*this == t_false || rhs == t_false) ? t_false : t_indet);
}

constexpr auto tbool::operator|(tbool const& rhs) const -> tbool
{
  return *this == t_false && rhs == t_false ? t_false
                                            : (*this == t_true || rhs == t_true ? t_true : t_indet);
}

constexpr auto tbool::operator!() const -> tbool
{
  return *this == t_indet ? t_indet : (*this == t_true ? t_false : t_true);
}

constexpr tbool::operator bool() const
{
  return *this == t_true;
}

constexpr auto tbool::operator==(tbool const& rhs) const -> bool
{
  return this == &rhs || m_value == rhs.m_value;
}

constexpr auto tbool::operator!=(tbool const& rhs) const -> bool
{
  return !(*this == rhs);
}

constexpr auto to_tbool(bool it) -> tbool
{
  return it ? t_true : t_false;
}

}
