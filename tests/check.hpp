/// \file check.hpp
/// Minimal check macro for the tests. Unlike assert, it is active in every
/// build mode, so the tests also run in release (NDEBUG) builds.
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

#ifndef BIT_TESTS_CHECK_HPP
#define BIT_TESTS_CHECK_HPP

#include <cstdio>

namespace bit_test
{

inline int& failures()
{
  static int count = 0;
  return count;
}

inline void check(bool ok, const char* expression, const char* file, int line)
{
  if (!ok)
  {
    ++failures();
    std::fprintf(stderr, "%s:%d: check failed: %s\n", file, line, expression);
  }
}

/// Prints the result and returns the process exit code
inline int report(const char* name)
{
  if (failures() == 0)
  {
    std::printf("%s: all checks passed\n", name);
    return 0;
  }

  std::fprintf(stderr, "%s: %d check(s) failed\n", name, failures());
  return 1;
}

} // namespace bit_test

// Variadic, so that expressions with template argument lists such as
// s.test_all<a, b>() need no extra parentheses
#define BIT_CHECK(...) \
  ::bit_test::check(static_cast<bool>(__VA_ARGS__), #__VA_ARGS__, __FILE__, __LINE__)

// BIT_TESTS_CHECK_HPP
#endif
