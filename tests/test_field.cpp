/// \file test_field.cpp
/// Tests of field, field::max, fits and the internal mask helpers.
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

#include "bit/field.hpp"

#include "check.hpp"

struct one    : bit::field<0> {};
struct nibble : bit::field<4, 7> {};
struct full8  : bit::field<0, 7> {};
struct full64 : bit::field<0, 63> {};
struct top64  : bit::field<63> {};

// Geometry
static_assert(one::first == 0 && one::last == 0 && one::size == 1, "");
static_assert(nibble::first == 4 && nibble::last == 7 && nibble::size == 4, "");
static_assert(full64::size == 64, "");

// field::max
static_assert(one::max == 1, "");
static_assert(nibble::max == 15, "");
static_assert(full8::max == 255, "");
static_assert(full64::max == ~std::uint64_t(0), "");
static_assert(top64::max == 1, "");

// fits
static_assert(bit::fits<nibble>(std::uint8_t(15)), "");
static_assert(!bit::fits<nibble>(std::uint8_t(16)), "");
static_assert(bit::fits<full8>(std::uint8_t(255)), "");
static_assert(bit::fits<full64>(~std::uint64_t(0)), "");
static_assert(!bit::fits<one>(std::uint32_t(2)), "");
static_assert(bit::fits<one>(std::uint64_t(0)), "");

// mask_range: bits outside the type are ignored
static_assert(bit::mask_range<std::uint8_t>(4, 7) == 0xF0, "");
static_assert(bit::mask_range<std::uint64_t>(0, 63) == ~std::uint64_t(0), "");
static_assert(bit::mask_range<std::uint8_t>(6, 9) == 0xC0, "");

// set / get of a field region
static_assert(bit::set<nibble>(std::uint8_t(0x0A)) == 0xA0, "");
static_assert(bit::set<nibble>(std::uint8_t(0x1A)) == 0xA0, "");
static_assert(bit::get<nibble>(std::uint8_t(0xA5)) == 0x0A, "");
static_assert(bit::get<top64>(~std::uint64_t(0)) == 1, "");

int main()
{
  // Same checks at run time, with values the compiler cannot fold
  volatile std::uint8_t sixteen = 16;
  volatile std::uint8_t fifteen = 15;

  BIT_CHECK(!bit::fits<nibble>(std::uint8_t(sixteen)));
  BIT_CHECK(bit::fits<nibble>(std::uint8_t(fifteen)));
  BIT_CHECK(bit::get<nibble>(bit::set<nibble>(std::uint8_t(fifteen))) == 15);

  return bit_test::report("field");
}
