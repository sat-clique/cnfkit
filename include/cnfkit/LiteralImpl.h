#include <cnfkit/Literal.h>

namespace cnfkit {

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
  return lit{m_raw_value};
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
