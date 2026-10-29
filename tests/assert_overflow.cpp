/// \file assert_overflow.cpp
/// Run time test: in debug builds assign asserts when a value does not fit in
/// the field. The test passes when the assertion message is printed.
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

#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "bit/storage.hpp"

struct mode : bit::field<1, 2> {};

// assert prints its message and calls abort. CTest counts a process killed by
// a signal as failed whatever its output, so turn abort into a normal exit;
// the test then passes on the assertion message alone.
extern "C" void exit_on_abort(int)
{
  std::_Exit(3);
}

int main()
{
#ifdef NDEBUG
  std::puts("BIT_TEST_SKIPPED_NDEBUG: assign is not checked when NDEBUG is defined");
  return 0;
#else
  std::signal(SIGABRT, exit_on_abort);

  // volatile keeps the compiler from seeing the value
  volatile std::uint8_t value = 4;

  bit::storage<bit::pack<std::uint8_t, mode>> s;
  s.assign<mode>(value);

  std::puts("assign did not assert");
  return 1;
#endif
}
