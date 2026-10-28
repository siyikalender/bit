/// \file register.hpp
/// Access to memory mapped (volatile) hardware registers through a pack.
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

#ifndef BIT_REGISTER_HPP
#define BIT_REGISTER_HPP

#include <cstdint>

#include "field.hpp"
#include "pack.hpp"
#include "storage.hpp"

namespace bit
{

/// Refers to a volatile register laid out by Pack.
///
/// Every operation performs exactly one volatile read and/or one volatile
/// write of the whole register, so field updates are explicit
/// read-modify-write sequences. Compound assignment on volatile objects
/// (deprecated in C++20) is never used.
///
/// Note: read-modify-write sequences are not atomic. Guard them yourself
/// when the register is shared with an interrupt handler or another core.
template
<
  typename Pack
>
class register_ref
{
public:
  typedef Pack                          pack;
  typedef typename Pack::value_type     value_type;
  typedef storage<Pack>                 storage_type;

  /// Binds to a register at a fixed address, e.g. register_ref<P>(0x40021018)
  explicit register_ref(std::uintptr_t address)
  : m_reg(reinterpret_cast<volatile value_type*>(address))
  {}

  /// Binds to a register given by a pointer
  explicit constexpr register_ref(volatile value_type* reg)
  : m_reg(reg)
  {}

  /// One volatile read of the whole register
  storage_type load() const
  {
    return storage_type(*m_reg);
  }

  /// One volatile write of the whole register
  void store(const storage_type& s) const
  {
    *m_reg = s.value();
  }

  /// Raw value of the register
  value_type value() const
  {
    return *m_reg;
  }

  /// Reads the register once, lets f change the copy, writes it back once.
  /// Use it to update several fields with a single write:
  ///   r.modify([](auto& s) { s.template assign<mode>(2).template set<enable>(); });
  template<typename F>
  void modify(F f) const
  {
    storage_type s = load();
    f(s);
    store(s);
  }

  /// Assigns the value in field region (read-modify-write)
  template<typename Field>
  void assign(value_type v) const
  {
    modify([v](storage_type& s) { s.template assign<Field>(v); });
  }

  /// Sets all bits in multiple field regions (read-modify-write)
  template<typename... Fields>
  void set() const
  {
    modify([](storage_type& s) { s.template set<Fields...>(); });
  }

  /// Clears all bits in multiple field regions (read-modify-write)
  template<typename... Fields>
  void clear() const
  {
    modify([](storage_type& s) { s.template clear<Fields...>(); });
  }

  /// Toggles all bits in multiple field regions (read-modify-write)
  template<typename... Fields>
  void toggle() const
  {
    modify([](storage_type& s) { s.template toggle<Fields...>(); });
  }

  /// Returns the current value of specified field (one volatile read)
  template<typename Field>
  value_type value_of() const
  {
    return load().template value_of<Field>();
  }

  /// Tests the value in field region (one volatile read)
  template<typename Field>
  bool test() const
  {
    return load().template test<Field>();
  }

private:
  volatile value_type*  m_reg;
};

} // namespace bit

// BIT_REGISTER_HPP
#endif
