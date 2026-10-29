/// \file test_atomic_storage.cpp
/// Tests of atomic_storage, including concurrent updates from several threads.
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

#include <atomic>
#include <cstdint>
#include <thread>
#include <type_traits>
#include <vector>

#include "bit/atomic_storage.hpp"

#include "check.hpp"

struct f0      : bit::field<0> {};
struct f1      : bit::field<1> {};
struct f2      : bit::field<2> {};
struct f3      : bit::field<3> {};
struct counter : bit::field<8, 23> {};

using P = bit::pack<std::uint32_t, f0, f1, f2, f3, counter>;
using A = bit::atomic_storage<P>;
using S = bit::storage<P>;

static_assert(!std::is_copy_constructible<A>::value, "");
static_assert(!std::is_copy_assignable<A>::value, "");
static_assert(std::is_same<A::storage_type, S>::value, "");

template<typename Flag>
void worker(A& a, int increments)
{
  for (int i = 0; i < increments; i++)
  {
    a.modify([](S& v)
    {
      v.assign<counter>(v.value_of<counter>() + 1);
    });
  }

  a.set<Flag>();
}

int main()
{
  A a;
  BIT_CHECK(a.value() == 0);

  // Modifying operations return the previous contents
  BIT_CHECK(a.set<f0, f2>().value() == 0x0);
  BIT_CHECK(a.value() == 0x5);
  BIT_CHECK(a.toggle<f0, f1>().value() == 0x5);
  BIT_CHECK(a.value() == 0x6);
  BIT_CHECK(a.clear<f1>().value() == 0x6);
  BIT_CHECK(a.value() == 0x4);
  BIT_CHECK(a.assign<counter, 7>().value() == 0x4);
  BIT_CHECK(a.value() == 0x704);
  BIT_CHECK(a.assign<counter>(9).value_of<counter>() == 7);
  BIT_CHECK(a.value_of<counter>() == 9);

  // checked_assign
  BIT_CHECK(a.checked_assign<counter>(0xFFFF));
  BIT_CHECK(!a.checked_assign<counter>(0x10000));
  BIT_CHECK(a.value_of<counter>() == 0xFFFF);

  // modify returns the previous contents and applies all changes at once
  S before = a.modify([](S& v) { v.clear<counter>().set<f3>(); });
  BIT_CHECK(before.value_of<counter>() == 0xFFFF);
  BIT_CHECK(a.value() == 0xC);

  // load / store, tests and explicit memory orders
  a.store(S(0));
  BIT_CHECK(a.load() == S(0));
  a.set<f1>(std::memory_order_relaxed);
  BIT_CHECK(a.test<f1>(std::memory_order_acquire));
  BIT_CHECK(a.test_any<f0, f1>());
  BIT_CHECK(!a.test_all<f0, f1>());

  // Concurrent updates: no increment and no flag may be lost
  a.store(S(0));

  const int increments = 10000;
  std::vector<std::thread> threads;
  threads.emplace_back(worker<f0>, std::ref(a), increments);
  threads.emplace_back(worker<f1>, std::ref(a), increments);
  threads.emplace_back(worker<f2>, std::ref(a), increments);
  threads.emplace_back(worker<f3>, std::ref(a), increments);

  for (auto& t : threads)
  {
    t.join();
  }

  BIT_CHECK(a.value_of<counter>() == 4u * increments);
  BIT_CHECK((a.test_all<f0, f1, f2, f3>()));

  return bit_test::report("atomic_storage");
}
