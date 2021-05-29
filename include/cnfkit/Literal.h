#pragma once

#include <cnfkit/detail/CheckCxxVersion.h>

#include <cmath>
#include <cstdint>
#include <limits>

namespace cnfkit {

class var {
public:
  constexpr var(uint32_t raw_value) noexcept : m_raw_value{raw_value} {}
  constexpr auto get_raw_value() const noexcept -> uint32_t;

  auto operator++(int) noexcept -> var&;
  auto operator++() noexcept -> var;
  auto operator--(int) noexcept -> var&;
  auto operator--() noexcept -> var;

private:
  uint32_t m_raw_value;
};

constexpr uint32_t max_raw_var = 0x7FFFFFFF;
constexpr var invalid_var(max_raw_var + 1);

class lit {
public:
  constexpr lit(var variable, bool is_positive) noexcept;

  constexpr auto get_var() const noexcept -> var;
  constexpr auto is_positive() const noexcept -> bool;

  constexpr auto operator-() const noexcept -> lit;
  constexpr auto get_raw_value() const noexcept -> uint32_t;
  constexpr auto is_valid() const noexcept -> bool;

  auto operator++(int) noexcept -> lit&;
  auto operator++() noexcept -> lit;
  auto operator--(int) noexcept -> lit&;
  auto operator--() noexcept -> lit;

  constexpr auto next_with_same_sign() const noexcept -> lit;
  constexpr auto prev_with_same_sign() const noexcept -> lit;

private:
  constexpr lit(uint32_t raw_value) : m_raw_value{raw_value} {}
  uint32_t m_raw_value;
};

namespace cnfkit_literals {
constexpr auto operator"" _lit(unsigned long long n) noexcept -> lit;
constexpr auto operator"" _var(unsigned long long n) noexcept -> var;
}

constexpr auto lit_to_dimacs(lit const& lit) noexcept -> int32_t;
inline auto dimacs_to_lit(int32_t dimacs_lit) noexcept -> lit;

constexpr auto operator==(var const& lhs, var const& rhs) noexcept -> bool;
constexpr auto operator!=(var const& lhs, var const& rhs) noexcept -> bool;
constexpr auto operator<(var const& lhs, var const& rhs) noexcept -> bool;
constexpr auto operator<=(var const& lhs, var const& rhs) noexcept -> bool;
constexpr auto operator>(var const& lhs, var const& rhs) noexcept -> bool;
constexpr auto operator>=(var const& lhs, var const& rhs) noexcept -> bool;

constexpr auto operator==(lit const& lhs, lit const& rhs) noexcept -> bool;
constexpr auto operator!=(lit const& lhs, lit const& rhs) noexcept -> bool;
constexpr auto operator<(lit const& lhs, lit const& rhs) noexcept -> bool;
constexpr auto operator<=(lit const& lhs, lit const& rhs) noexcept -> bool;
constexpr auto operator>(lit const& lhs, lit const& rhs) noexcept -> bool;
constexpr auto operator>=(lit const& lhs, lit const& rhs) noexcept -> bool;


// *** Implementation ***

namespace cnfkit_literals {
constexpr auto operator"" _lit(unsigned long long n) noexcept -> lit
{
  return lit{var{static_cast<uint32_t>(n)}, true};
}

constexpr auto operator"" _var(unsigned long long n) noexcept -> var
{
  return var{static_cast<uint32_t>(n)};
}
}

constexpr auto var::get_raw_value() const noexcept -> uint32_t
{
  return m_raw_value;
}

inline auto var::operator++(int) noexcept -> var&
{
  ++m_raw_value;
  return *this;
}

inline auto var::operator++() noexcept -> var
{
  var copy = *this;
  ++m_raw_value;
  return copy;
}

inline auto var::operator--(int) noexcept -> var&
{
  --m_raw_value;
  return *this;
}

inline auto var::operator--() noexcept -> var
{
  var copy = *this;
  --m_raw_value;
  return *this;
}

constexpr lit::lit(var variable, bool is_positive) noexcept
  : m_raw_value{(variable.get_raw_value() << 1) + (is_positive ? 1 : 0)}
{
}

constexpr auto lit::get_var() const noexcept -> var
{
  return var{m_raw_value >> 1};
}

constexpr auto lit::is_positive() const noexcept -> bool
{
  return (m_raw_value & 1) == 1;
}

constexpr auto lit::operator-() const noexcept -> lit
{
  return lit{m_raw_value ^ 1};
}


constexpr auto lit::get_raw_value() const noexcept -> uint32_t
{
  return m_raw_value;
}

constexpr auto lit::is_valid() const noexcept -> bool
{
  return get_var() != invalid_var;
}

inline auto lit::operator++(int) noexcept -> lit&
{
  ++m_raw_value;
  return *this;
}

inline auto lit::operator++() noexcept -> lit
{
  lit copy = *this;
  ++m_raw_value;
  return *this;
}

inline auto lit::operator--(int) noexcept -> lit&
{
  --m_raw_value;
  return *this;
}

inline auto lit::operator--() noexcept -> lit
{
  lit copy = *this;
  --m_raw_value;
  return *this;
}

constexpr auto lit::next_with_same_sign() const noexcept -> lit
{
  return lit{m_raw_value + 2};
}

constexpr auto lit::prev_with_same_sign() const noexcept -> lit
{
  return lit{m_raw_value - 2};
}

constexpr auto lit_to_dimacs(lit const& lit) noexcept -> int32_t
{
  return static_cast<int32_t>(lit.get_var().get_raw_value()) * (lit.is_positive() ? 1 : -1);
}

inline auto dimacs_to_lit(int32_t dimacs_lit) noexcept -> lit
{
  if (dimacs_lit == std::numeric_limits<int32_t>::min()) {
    return lit{invalid_var, false};
  }

  uint32_t abs_dimacs_lit = std::abs(dimacs_lit);
  if (abs_dimacs_lit > max_raw_var) {
    return lit{invalid_var, false};
  }

  return lit{var{abs_dimacs_lit}, dimacs_lit > 0};
}


constexpr auto operator==(var const& lhs, var const& rhs) noexcept -> bool
{
  return &lhs == &rhs || lhs.get_raw_value() == rhs.get_raw_value();
}

constexpr auto operator!=(var const& lhs, var const& rhs) noexcept -> bool
{
  return !(lhs == rhs);
}

constexpr auto operator<(var const& lhs, var const& rhs) noexcept -> bool
{
  return lhs.get_raw_value() < rhs.get_raw_value();
}

constexpr auto operator<=(var const& lhs, var const& rhs) noexcept -> bool
{
  return lhs.get_raw_value() <= rhs.get_raw_value();
}

constexpr auto operator>(var const& lhs, var const& rhs) noexcept -> bool
{
  return lhs.get_raw_value() > rhs.get_raw_value();
}

constexpr auto operator>=(var const& lhs, var const& rhs) noexcept -> bool
{
  return lhs.get_raw_value() >= rhs.get_raw_value();
}

constexpr auto operator==(lit const& lhs, lit const& rhs) noexcept -> bool
{
  return lhs.get_raw_value() == rhs.get_raw_value();
}

constexpr auto operator!=(lit const& lhs, lit const& rhs) noexcept -> bool
{
  return lhs.get_raw_value() != rhs.get_raw_value();
}

constexpr auto operator<(lit const& lhs, lit const& rhs) noexcept -> bool
{
  return lhs.get_raw_value() < rhs.get_raw_value();
}

constexpr auto operator<=(lit const& lhs, lit const& rhs) noexcept -> bool
{
  return lhs.get_raw_value() <= rhs.get_raw_value();
}

constexpr auto operator>(lit const& lhs, lit const& rhs) noexcept -> bool
{
  return lhs.get_raw_value() > rhs.get_raw_value();
}

constexpr auto operator>=(lit const& lhs, lit const& rhs) noexcept -> bool
{
  return lhs.get_raw_value() >= rhs.get_raw_value();
}
}
