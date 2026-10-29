/// \file test_storage.cpp
/// Tests of pack and storage.
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
#include <type_traits>

#include "bit/storage.hpp"

#include "check.hpp"

struct en        : bit::field<0> {};
struct irq       : bit::field<1> {};
struct mode      : bit::field<2, 3> {};
struct prescaler : bit::field<8, 15> {};

using P = bit::pack<std::uint16_t, en, irq, mode, prescaler>;
using S = bit::storage<P>;

// Pack
static_assert(std::is_same<P::value_type, std::uint16_t>::value, "");
static_assert(P::mask == 0xFF0F, "");
static_assert(P::exists<mode>() && !P::exists<bit::field<4>>(), "");
static_assert(P::get_field_mask<prescaler>() == 0xFF00, "");

// Storage layout: a plain, trivially copyable wrapper of the value type
static_assert(sizeof(S) == sizeof(std::uint16_t), "");
static_assert(std::is_trivially_copyable<S>::value, "");
static_assert(std::is_standard_layout<S>::value, "");

// Compile time use
constexpr S make()
{
  S s;
  s.assign<mode, 2>().assign<prescaler>(0x40).set<en>();
  return s;
}

static_assert(make().value() == 0x4009, "");
static_assert(make().value_of<mode>() == 2, "");
static_assert(make().value_of<prescaler>() == 0x40, "");
static_assert(make().test<en>() && !make().test<irq>(), "");
static_assert(make().test_all<en, mode, prescaler>(), "");
static_assert(!make().test_all<en, irq>(), "");
static_assert(make().test_any<irq, mode>(), "");
static_assert(!make().test_any<irq>(), "");
static_assert(make() == S(0x4009) && make() != S(0), "");
static_assert(S::get_mask<en, mode>() == 0x000D, "");

// Other value types
struct high32 : bit::field<32, 63> {};
struct all64  : bit::field<0, 63> {};
struct all8   : bit::field<0, 7> {};

static_assert(bit::storage<bit::pack<std::uint64_t, high32>>()
                .assign<high32>(0xFFFFFFFFu).value() == 0xFFFFFFFF00000000u, "");
static_assert(bit::storage<bit::pack<std::uint64_t, all64>>()
                .set<all64>().value() == ~std::uint64_t(0), "");
static_assert(bit::storage<bit::pack<std::uint8_t, all8>>()
                .assign<all8, 0xFF>().value() == 0xFF, "");

S returned_by_value()
{
  S s;
  s.set<irq>();
  return s;
}

int main()
{
  S s;
  BIT_CHECK(s.value() == 0);

  // assign leaves the other fields untouched
  s = 0xFFFF;
  s.assign<mode>(1);
  BIT_CHECK(s.value() == 0xFFF7);
  BIT_CHECK(s.value_of<mode>() == 1);

  // set / clear / toggle on several fields
  s = 0;
  s.set<en, mode>();
  BIT_CHECK(s.value() == 0x000D);
  s.clear<en>();
  BIT_CHECK(s.value() == 0x000C);
  s.toggle<en, irq>();
  BIT_CHECK(s.value() == 0x000F);
  s.toggle<mode>();
  BIT_CHECK(s.value() == 0x0003);

  // checked_assign writes only values that fit
  S c(0x0100);
  BIT_CHECK(c.checked_assign<mode>(3));
  BIT_CHECK(c.value() == 0x010C);
  BIT_CHECK(!c.checked_assign<mode>(4));
  BIT_CHECK(c.value() == 0x010C);
  BIT_CHECK(c.checked_assign<prescaler>(255));
  BIT_CHECK(c.value() == 0xFF0C);
  BIT_CHECK(!c.checked_assign<prescaler>(256));
  BIT_CHECK(c.value() == 0xFF0C);

  // Compile time constant
  c.assign<prescaler, 0x12>();
  BIT_CHECK(c.value_of<prescaler>() == 0x12);

  // test_all / test_any at run time
  BIT_CHECK(c.test_all<mode, prescaler>());
  BIT_CHECK(!c.test_all<en, mode>());
  BIT_CHECK(c.test_any<en, mode>());
  BIT_CHECK(!c.test_any<en, irq>());

  // Copy, compare and return by value
  S d = c;
  BIT_CHECK(d == c);
  d.toggle<en>();
  BIT_CHECK(d != c);
  BIT_CHECK(returned_by_value().value() == 0x0002);

#ifdef NDEBUG
  // Without assertions an oversized value is truncated to the field width
  S t;
  t.assign<mode>(5);
  BIT_CHECK(t.value_of<mode>() == 1);
  BIT_CHECK(t.value() == 0x0004);
#endif

  return bit_test::report("storage");
}
