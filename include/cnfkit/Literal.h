#pragma once

#include <cnfkit/detail/CheckCxxVersion.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace cnfkit {

class var {
public:
  constexpr var(uint32_t raw_value) noexcept : m_raw_value{raw_value} {}
  constexpr var() noexcept : m_raw_value{0} {}

  constexpr auto get_raw_value() const noexcept -> uint32_t;

  auto operator++() noexcept -> var&;
  auto operator++(int) noexcept -> var;
  auto operator--() noexcept -> var&;
  auto operator--(int) noexcept -> var;

  constexpr auto next() const noexcept -> var;
  constexpr auto prev() const noexcept -> var;

private:
  uint32_t m_raw_value;
};

class lit {
public:
  constexpr lit(var variable, bool is_positive) noexcept;
  constexpr lit() noexcept;

  constexpr auto get_var() const noexcept -> var;
  constexpr auto is_positive() const noexcept -> bool;

  constexpr auto operator-() const noexcept -> lit;
  constexpr auto get_raw_value() const noexcept -> uint32_t;

  auto operator++() noexcept -> lit&;
  auto operator++(int) noexcept -> lit;
  auto operator--() noexcept -> lit&;
  auto operator--(int) noexcept -> lit;

  constexpr auto next_with_same_sign() const noexcept -> lit;
  constexpr auto prev_with_same_sign() const noexcept -> lit;
  constexpr auto next() const noexcept -> lit;
  constexpr auto prev() const noexcept -> lit;

private:
  constexpr lit(uint32_t raw_value) : m_raw_value{raw_value} {}
  uint32_t m_raw_value;
};

constexpr var max_var = var{std::numeric_limits<uint32_t>::max() / 2};

namespace cnfkit_literals {
constexpr auto operator"" _lit(unsigned long long n) noexcept -> lit;
constexpr auto operator"" _var(unsigned long long n) noexcept -> var;
constexpr auto operator"" _dlit(unsigned long long n) noexcept -> lit;
constexpr auto operator"" _dvar(unsigned long long n) noexcept -> var;
}

inline auto lit_to_dimacs(lit lit) -> int32_t;
inline auto dimacs_to_lit(int32_t dimacs_lit) -> lit;
constexpr uint32_t min_dimacs_lit = std::numeric_limits<int32_t>::min() + 1;
constexpr uint32_t max_dimacs_lit = std::numeric_limits<int32_t>::max();


constexpr auto operator==(var lhs, var rhs) noexcept -> bool;
constexpr auto operator!=(var lhs, var rhs) noexcept -> bool;
constexpr auto operator<(var lhs, var rhs) noexcept -> bool;
constexpr auto operator<=(var lhs, var rhs) noexcept -> bool;
constexpr auto operator>(var lhs, var rhs) noexcept -> bool;
constexpr auto operator>=(var lhs, var rhs) noexcept -> bool;

constexpr auto operator==(lit lhs, lit rhs) noexcept -> bool;
constexpr auto operator!=(lit lhs, lit rhs) noexcept -> bool;
constexpr auto operator<(lit lhs, lit rhs) noexcept -> bool;
constexpr auto operator<=(lit lhs, lit rhs) noexcept -> bool;
constexpr auto operator>(lit lhs, lit rhs) noexcept -> bool;
constexpr auto operator>=(lit lhs, lit rhs) noexcept -> bool;


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

constexpr auto operator"" _dlit(unsigned long long n) noexcept -> lit
{
  return lit{var{static_cast<uint32_t>(n - 1)}, true};
}

constexpr auto operator"" _dvar(unsigned long long n) noexcept -> var
{
  return var{static_cast<uint32_t>(n - 1)};
}

}

constexpr auto var::get_raw_value() const noexcept -> uint32_t
{
  return m_raw_value;
}

inline auto var::operator++() noexcept -> var&
{
  ++m_raw_value;
  return *this;
}

inline auto var::operator++(int) noexcept -> var
{
  var copy = *this;
  ++m_raw_value;
  return copy;
}

inline auto var::operator--() noexcept -> var&
{
  --m_raw_value;
  return *this;
}

inline auto var::operator--(int) noexcept -> var
{
  var copy = *this;
  --m_raw_value;
  return copy;
}

constexpr auto var::next() const noexcept -> var
{
  return var{m_raw_value + 1};
}

constexpr auto var::prev() const noexcept -> var
{
  return var{m_raw_value - 1};
}

constexpr lit::lit(var variable, bool is_positive) noexcept
  : m_raw_value{(variable.get_raw_value() << 1) + (is_positive ? 1 : 0)}
{
}

constexpr lit::lit() noexcept : m_raw_value{0} {}

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

inline auto lit::operator++() noexcept -> lit&
{
  ++m_raw_value;
  return *this;
}

inline auto lit::operator++(int) noexcept -> lit
{
  lit copy = *this;
  ++m_raw_value;
  return copy;
}

inline auto lit::operator--() noexcept -> lit&
{
  --m_raw_value;
  return *this;
}

inline auto lit::operator--(int) noexcept -> lit
{
  lit copy = *this;
  --m_raw_value;
  return copy;
}

constexpr auto lit::next_with_same_sign() const noexcept -> lit
{
  return lit{m_raw_value + 2};
}

constexpr auto lit::prev_with_same_sign() const noexcept -> lit
{
  return lit{m_raw_value - 2};
}

constexpr auto lit::next() const noexcept -> lit
{
  return lit{m_raw_value + 1};
}

constexpr auto lit::prev() const noexcept -> lit
{
  return lit{m_raw_value - 1};
}

inline auto lit_to_dimacs(lit literal) -> int32_t
{
  uint32_t const raw_abs_value = literal.get_var().get_raw_value() + 1;
  if (raw_abs_value > std::numeric_limits<int32_t>::max()) {
    throw std::invalid_argument{"DIMACS literal out of range"};
  }
  return static_cast<int32_t>(raw_abs_value) * (literal.is_positive() ? 1 : -1);
}

inline auto dimacs_to_lit(int32_t dimacs_lit) -> lit
{
  if (dimacs_lit == 0 || dimacs_lit == std::numeric_limits<int32_t>::min()) {
    throw std::invalid_argument{"DIMACS literal out of range"};
  }
  uint32_t const raw_abs_value = static_cast<uint32_t>(std::abs(dimacs_lit)) - 1;

  return lit{var{raw_abs_value}, dimacs_lit > 0};
}

constexpr auto operator==(var lhs, var rhs) noexcept -> bool
{
  return lhs.get_raw_value() == rhs.get_raw_value();
}

constexpr auto operator!=(var lhs, var rhs) noexcept -> bool
{
  return !(lhs == rhs);
}

constexpr auto operator<(var lhs, var rhs) noexcept -> bool
{
  return lhs.get_raw_value() < rhs.get_raw_value();
}

constexpr auto operator<=(var lhs, var rhs) noexcept -> bool
{
  return lhs.get_raw_value() <= rhs.get_raw_value();
}

constexpr auto operator>(var lhs, var rhs) noexcept -> bool
{
  return lhs.get_raw_value() > rhs.get_raw_value();
}

constexpr auto operator>=(var lhs, var rhs) noexcept -> bool
{
  return lhs.get_raw_value() >= rhs.get_raw_value();
}

constexpr auto operator==(lit lhs, lit rhs) noexcept -> bool
{
  return lhs.get_raw_value() == rhs.get_raw_value();
}

constexpr auto operator!=(lit lhs, lit rhs) noexcept -> bool
{
  return lhs.get_raw_value() != rhs.get_raw_value();
}

constexpr auto operator<(lit lhs, lit rhs) noexcept -> bool
{
  return lhs.get_raw_value() < rhs.get_raw_value();
}

constexpr auto operator<=(lit lhs, lit rhs) noexcept -> bool
{
  return lhs.get_raw_value() <= rhs.get_raw_value();
}

constexpr auto operator>(lit lhs, lit rhs) noexcept -> bool
{
  return lhs.get_raw_value() > rhs.get_raw_value();
}

constexpr auto operator>=(lit lhs, lit rhs) noexcept -> bool
{
  return lhs.get_raw_value() >= rhs.get_raw_value();
}
}
