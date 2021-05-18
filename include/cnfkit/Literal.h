#pragma once

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

}

#include "cnfkit/LiteralImpl.h"
