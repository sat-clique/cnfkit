#pragma once

#include <cmath>
#include <cstdint>

namespace cnfkit {

class var {
public:
  constexpr var(uint32_t raw_value) : m_raw_value{raw_value} {}
  constexpr auto get_raw_value() const -> uint32_t;

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
  constexpr lit(var variable, bool is_positive);

  constexpr auto get_var() const -> var;
  constexpr auto is_positive() const -> bool;

  constexpr auto operator-() const -> lit;
  constexpr auto get_raw_value() const -> uint32_t;
  constexpr auto is_valid() const -> bool;

  auto operator++(int) noexcept -> lit&;
  auto operator++() noexcept -> lit;
  auto operator--(int) noexcept -> lit&;
  auto operator--() noexcept -> lit;

  constexpr auto next_with_same_sign() const -> lit;
  constexpr auto prev_with_same_sign() const -> lit;

private:
  constexpr lit(uint32_t raw_value) : m_raw_value{raw_value} {}
  uint32_t m_raw_value;
};

namespace cnfkit_literals {
constexpr lit operator"" _lit(unsigned long long n);
constexpr var operator"" _var(unsigned long long n);
}

constexpr auto operator==(var const& lhs, var const& rhs) -> bool;
constexpr auto operator!=(var const& lhs, var const& rhs) -> bool;
constexpr auto operator<(var const& lhs, var const& rhs) -> bool;
constexpr auto operator<=(var const& lhs, var const& rhs) -> bool;
constexpr auto operator>(var const& lhs, var const& rhs) -> bool;
constexpr auto operator>=(var const& lhs, var const& rhs) -> bool;

constexpr auto operator==(lit const& lhs, lit const& rhs) -> bool;
constexpr auto operator!=(lit const& lhs, lit const& rhs) -> bool;
constexpr auto operator<(lit const& lhs, lit const& rhs) -> bool;
constexpr auto operator<=(lit const& lhs, lit const& rhs) -> bool;
constexpr auto operator>(lit const& lhs, lit const& rhs) -> bool;
constexpr auto operator>=(lit const& lhs, lit const& rhs) -> bool;


// *** Implementation ***

namespace cnfkit_literals {
constexpr lit operator"" _lit(unsigned long long n)
{
  return lit{var{static_cast<uint32_t>(n)}, true};
}

constexpr var operator"" _var(unsigned long long n)
{
  return var{static_cast<uint32_t>(n)};
}
}

constexpr auto var::get_raw_value() const -> uint32_t
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

constexpr lit::lit(var variable, bool is_positive)
  : m_raw_value{(variable.get_raw_value() << 1) + (is_positive ? 1 : 0)}
{
}

constexpr auto lit::get_var() const -> var
{
  return var{m_raw_value >> 1};
}

constexpr auto lit::is_positive() const -> bool
{
  return (m_raw_value & 1) == 1;
}

constexpr auto lit::operator-() const -> lit
{
  return lit{m_raw_value ^ 1};
}


constexpr auto lit::get_raw_value() const -> uint32_t
{
  return m_raw_value;
}

constexpr auto lit::is_valid() const -> bool
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

constexpr auto lit::next_with_same_sign() const -> lit
{
  return lit{m_raw_value + 2};
}

constexpr auto lit::prev_with_same_sign() const -> lit
{
  return lit{m_raw_value - 2};
}

constexpr auto operator==(var const& lhs, var const& rhs) -> bool
{
  return &lhs == &rhs || lhs.get_raw_value() == rhs.get_raw_value();
}

constexpr auto operator!=(var const& lhs, var const& rhs) -> bool
{
  return !(lhs == rhs);
}

constexpr auto operator<(var const& lhs, var const& rhs) -> bool
{
  return lhs.get_raw_value() < rhs.get_raw_value();
}

constexpr auto operator<=(var const& lhs, var const& rhs) -> bool
{
  return lhs.get_raw_value() <= rhs.get_raw_value();
}

constexpr auto operator>(var const& lhs, var const& rhs) -> bool
{
  return lhs.get_raw_value() > rhs.get_raw_value();
}

constexpr auto operator>=(var const& lhs, var const& rhs) -> bool
{
  return lhs.get_raw_value() >= rhs.get_raw_value();
}

constexpr auto operator==(lit const& lhs, lit const& rhs) -> bool
{
  return lhs.get_raw_value() == rhs.get_raw_value();
}

constexpr auto operator!=(lit const& lhs, lit const& rhs) -> bool
{
  return lhs.get_raw_value() != rhs.get_raw_value();
}

constexpr auto operator<(lit const& lhs, lit const& rhs) -> bool
{
  return lhs.get_raw_value() < rhs.get_raw_value();
}

constexpr auto operator<=(lit const& lhs, lit const& rhs) -> bool
{
  return lhs.get_raw_value() <= rhs.get_raw_value();
}

constexpr auto operator>(lit const& lhs, lit const& rhs) -> bool
{
  return lhs.get_raw_value() > rhs.get_raw_value();
}

constexpr auto operator>=(lit const& lhs, lit const& rhs) -> bool
{
  return lhs.get_raw_value() > rhs.get_raw_value();
}
}
