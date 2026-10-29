/// \file atomic_storage.hpp
/// Implements a thread safe memory place holder for specific pack.
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

/// \author Claude Opus 5.5 <noreply@anthropic.com>
/// \date   2021

#ifndef BIT_ATOMIC_STORAGE_HPP
#define BIT_ATOMIC_STORAGE_HPP

#include <atomic>

#include "field.hpp"
#include "pack.hpp"
#include "storage.hpp"

namespace bit
{

/// Thread safe counterpart of storage.
///
/// Every operation is a single atomic operation on the whole value:
/// set, clear and toggle map to fetch_or, fetch_and and fetch_xor;
/// assign and modify use a compare-exchange loop. Modifying operations
/// return the previous contents as a plain storage, like std::atomic's
/// fetch_* functions. They are intentionally not chainable, since a chain
/// of operations would not be atomic as a whole; use modify() for that.
template
<
  typename Pack
>
class atomic_storage
{
public:
  typedef Pack                          pack;
  typedef typename Pack::value_type     value_type;
  typedef storage<Pack>                 storage_type;

  constexpr atomic_storage(const value_type v = 0) noexcept
  : m_value(v)
  {}

  atomic_storage(const atomic_storage&)            = delete;
  atomic_storage& operator=(const atomic_storage&) = delete;

  /// Atomically reads the whole value
  storage_type load(std::memory_order order = std::memory_order_seq_cst) const noexcept
  {
    return storage_type(m_value.load(order));
  }

  /// Atomically replaces the whole value
  void store(const storage_type& s,
             std::memory_order order = std::memory_order_seq_cst) noexcept
  {
    m_value.store(s.value(), order);
  }

  /// Returns the current value of storage
  value_type value(std::memory_order order = std::memory_order_seq_cst) const noexcept
  {
    return m_value.load(order);
  }

  /// Atomically sets all bits in multiple field regions. Returns previous contents.
  template<typename... Fields>
  storage_type set(std::memory_order order = std::memory_order_seq_cst) noexcept
  {
    return storage_type(
      m_value.fetch_or(storage_type::template get_mask<Fields...>(), order));
  }

  /// Atomically clears all bits in multiple field regions. Returns previous contents.
  template<typename... Fields>
  storage_type clear(std::memory_order order = std::memory_order_seq_cst) noexcept
  {
    return storage_type(
      m_value.fetch_and(
        bit::invert(storage_type::template get_mask<Fields...>()), order));
  }

  /// Atomically toggles all bits in multiple field regions. Returns previous contents.
  template<typename... Fields>
  storage_type toggle(std::memory_order order = std::memory_order_seq_cst) noexcept
  {
    return storage_type(
      m_value.fetch_xor(storage_type::template get_mask<Fields...>(), order));
  }

  /// Atomically reads the value, lets f change a copy of it and writes it back.
  /// f may be called more than once when other threads change the value
  /// concurrently, so it must not have side effects. Returns previous contents.
  ///   s.modify([](auto& v) { v.template assign<mode>(2).template set<enable>(); });
  template<typename F>
  storage_type modify(F f, std::memory_order order = std::memory_order_seq_cst)
  {
    value_type expected = m_value.load(std::memory_order_relaxed);
    storage_type desired;
    do
    {
      desired = storage_type(expected);
      f(desired);
    }
    while (!m_value.compare_exchange_weak(expected, desired.value(),
                                          order, std::memory_order_relaxed));
    return storage_type(expected);
  }

  /// Atomically assigns the value in field region. Returns previous contents.
  /// Asserts in debug builds when v does not fit in the field (see storage::assign).
  template<typename Field>
  storage_type assign(value_type v,
                      std::memory_order order = std::memory_order_seq_cst) noexcept
  {
    return modify([v](storage_type& s) { s.template assign<Field>(v); }, order);
  }

  /// Atomically assigns the value in field region only if it fits, in all
  /// build modes. The check is done once, before any atomic operation;
  /// returns false and leaves the value untouched when v exceeds Field::max.
  template<typename Field>
  bool checked_assign(value_type v,
                      std::memory_order order = std::memory_order_seq_cst) noexcept
  {
    pack::template assert_if_not_exists<Field>();

    if (!bit::fits<Field>(v))
    {
      return false;
    }

    assign<Field>(v, order);
    return true;
  }

  /// Atomically assigns a compile time constant to field region:
  /// s.assign<mode, 2>(); A value that does not fit is a compile error.
  /// Returns previous contents.
  template<typename Field, std::uint64_t Value>
  storage_type assign(std::memory_order order = std::memory_order_seq_cst) noexcept
  {
    static_assert(Value <= Field::max, "Value does not fit in field");

    return assign<Field>(value_type(Value), order);
  }

  /// Returns the current value of specified field
  template<typename Field>
  value_type value_of(std::memory_order order = std::memory_order_seq_cst) const noexcept
  {
    return load(order).template value_of<Field>();
  }

  /// Tests the value in field region. Returns true if value is greater than zero.
  template<typename Field>
  bool test(std::memory_order order = std::memory_order_seq_cst) const noexcept
  {
    return load(order).template test<Field>();
  }

  /// True when every listed field is non zero, from a single atomic load
  template<typename... Fields>
  bool test_all(std::memory_order order = std::memory_order_seq_cst) const noexcept
  {
    return load(order).template test_all<Fields...>();
  }

  /// True when at least one listed field is non zero, from a single atomic load
  template<typename... Fields>
  bool test_any(std::memory_order order = std::memory_order_seq_cst) const noexcept
  {
    return load(order).template test_any<Fields...>();
  }

  /// True when the operations are always lock free for this value type
  static constexpr bool is_always_lock_free =
    std::atomic<value_type>::is_always_lock_free;

private:
  std::atomic<value_type>  m_value;
};

} // namespace bit

// BIT_ATOMIC_STORAGE_HPP
#endif
