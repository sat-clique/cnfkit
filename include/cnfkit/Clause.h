#pragma once

#include <cnfkit/Literal.h>

#include <cassert>
#include <cstdint>
#include <cstring>

namespace cnfkit {
template <typename Derived, typename SizeType = uint32_t>
class clause {
public:
  using size_type = SizeType;
  using iterator = lit*;
  using const_iterator = lit const*;

  auto size() const noexcept -> size_type;
  auto empty() const noexcept -> bool;

  void shrink(size_type new_size) noexcept;

  auto operator[](size_type idx) noexcept -> lit&;
  auto operator[](size_type idx) const noexcept -> lit const&;

  auto begin() noexcept -> iterator;
  auto begin() const noexcept -> const_iterator;

  auto end() noexcept -> iterator;
  auto end() const noexcept -> const_iterator;

  auto cbegin() const noexcept -> const_iterator;
  auto cend() const noexcept -> const_iterator;

  constexpr static auto get_mem_size(size_type num_lits) noexcept -> size_t;
  static auto construct_in(unsigned char* mem, size_type num_lits) noexcept -> Derived*;

protected:
  clause(size_type size);

private:
  constexpr static auto get_lit_offset()
  {
    if constexpr (sizeof(Derived) % alignof(lit) == 0) {
      return sizeof(Derived);
    }
    else {
      return sizeof(Derived) + alignof(lit) - (sizeof(Derived) % alignof(lit));
    }
  }

  constexpr static size_t lit_offset = get_lit_offset();

  auto get_lits() noexcept -> lit*;
  auto get_lits() const noexcept -> lit const*;

  size_type m_size;
};


// *** Implementation ***

template <typename Derived, typename SizeType>
auto clause<Derived, SizeType>::size() const noexcept -> size_type
{
  return m_size;
}

template <typename Derived, typename SizeType>
auto clause<Derived, SizeType>::empty() const noexcept -> bool
{
  return m_size == 0;
}

template <typename Derived, typename SizeType>
void clause<Derived, SizeType>::shrink(size_type new_size) noexcept
{
  assert(new_size <= m_size);
  m_size = new_size;
}

template <typename Derived, typename SizeType>
auto clause<Derived, SizeType>::operator[](size_type idx) noexcept -> lit&
{
  assert(idx < m_size);
  return *(get_lits() + idx);
}

template <typename Derived, typename SizeType>
auto clause<Derived, SizeType>::operator[](size_type idx) const noexcept -> lit const&
{
  assert(idx < m_size);
  return *(get_lits() + idx);
}

template <typename Derived, typename SizeType>
auto clause<Derived, SizeType>::begin() noexcept -> iterator
{
  return get_lits();
}

template <typename Derived, typename SizeType>
auto clause<Derived, SizeType>::begin() const noexcept -> const_iterator
{
  return get_lits();
}

template <typename Derived, typename SizeType>
auto clause<Derived, SizeType>::end() noexcept -> iterator
{
  return get_lits() + m_size;
}

template <typename Derived, typename SizeType>
auto clause<Derived, SizeType>::end() const noexcept -> const_iterator
{
  return get_lits() + m_size;
}

template <typename Derived, typename SizeType>
auto clause<Derived, SizeType>::cbegin() const noexcept -> const_iterator
{
  return get_lits();
}

template <typename Derived, typename SizeType>
auto clause<Derived, SizeType>::cend() const noexcept -> const_iterator
{
  return get_lits() + m_size;
}

template <typename Derived, typename SizeType>
clause<Derived, SizeType>::clause(size_type size) : m_size{size}
{
}

template <typename Derived, typename SizeType>
auto clause<Derived, SizeType>::get_lits() const noexcept -> lit const*
{
  return reinterpret_cast<lit const*>(reinterpret_cast<char const*>(this) + lit_offset);
}

template <typename Derived, typename SizeType>
auto clause<Derived, SizeType>::get_lits() noexcept -> lit*
{
  return reinterpret_cast<lit*>(reinterpret_cast<char*>(this) + lit_offset);
}


template <typename Derived, typename SizeType>
constexpr auto clause<Derived, SizeType>::get_mem_size(size_type num_lits) noexcept -> size_t
{
  return lit_offset + num_lits * sizeof(lit);
}

template <typename Derived, typename SizeType>
auto clause<Derived, SizeType>::construct_in(unsigned char* mem, size_type num_lits) noexcept
    -> Derived*
{
  static_assert(alignof(Derived) % alignof(lit) == 0);
  static_assert(sizeof(Derived) % alignof(lit) == 0);

  Derived* result = new (mem) Derived(num_lits);

  unsigned char* lit_mem = mem + lit_offset;
  std::memset(lit_mem, 0, sizeof(lit) * num_lits);

  return result;
}
}
