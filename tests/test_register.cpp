/// \file test_register.cpp
/// Tests of register_ref on a volatile variable standing in for a hardware register.
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
/// \date   2026

#include <cstdint>

#include "bit/register.hpp"

#include "check.hpp"

struct en        : bit::field<0> {};
struct irq       : bit::field<1> {};
struct mode      : bit::field<2, 3> {};
struct prescaler : bit::field<8, 15> {};

using P = bit::pack<std::uint32_t, en, irq, mode, prescaler>;
using R = bit::register_ref<P>;
using S = bit::storage<P>;

volatile std::uint32_t reg = 0;

int main()
{
  const R r(&reg);

  // Bits outside of the pack are preserved by read-modify-write operations
  reg = 0xFFFF0000u;

  r.assign<mode, 2>();
  BIT_CHECK(reg == 0xFFFF0008u);
  r.set<en, irq>();
  BIT_CHECK(reg == 0xFFFF000Bu);
  r.clear<irq>();
  BIT_CHECK(reg == 0xFFFF0009u);
  r.toggle<en, mode>();
  BIT_CHECK(reg == 0xFFFF0004u);
  r.assign<prescaler>(0x40);
  BIT_CHECK(reg == 0xFFFF4004u);

  // checked_assign does not touch the register when the value does not fit
  BIT_CHECK(!r.checked_assign<mode>(4));
  BIT_CHECK(reg == 0xFFFF4004u);
  BIT_CHECK(r.checked_assign<mode>(3));
  BIT_CHECK(reg == 0xFFFF400Cu);

  // Several fields with one write
  r.modify([](S& s) { s.assign<prescaler>(0x12).set<en>().clear<mode>(); });
  BIT_CHECK(reg == 0xFFFF1201u);

  // Reads
  BIT_CHECK(r.value() == 0xFFFF1201u);
  BIT_CHECK(r.value_of<prescaler>() == 0x12);
  BIT_CHECK(r.test<en>() && !r.test<irq>());
  BIT_CHECK(r.test_all<en, prescaler>());
  BIT_CHECK(r.test_any<irq, prescaler>());
  BIT_CHECK(!r.test_any<irq, mode>());
  BIT_CHECK(r.load() == S(0xFFFF1201u));

  // store writes the whole register
  r.store(S(0x5));
  BIT_CHECK(reg == 0x5u);

  // Binding by address
  const R by_address(reinterpret_cast<std::uintptr_t>(&reg));
  by_address.set<irq>();
  BIT_CHECK(reg == 0x7u);

  return bit_test::report("register_ref");
}
