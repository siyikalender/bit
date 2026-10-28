/// Basic usage demonstration of atomic_storage across threads

#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>

#include "bit/atomic_storage.hpp"

// Each worker owns one flag bit; all workers share a counter field
struct flag_0  : bit::field<0> {};
struct flag_1  : bit::field<1> {};
struct flag_2  : bit::field<2> {};
struct flag_3  : bit::field<3> {};
struct counter : bit::field<8, 23> {};

using status = bit::atomic_storage
<
  bit::pack<uint32_t, flag_0, flag_1, flag_2, flag_3, counter>
>;

status s;

template<typename Flag>
void worker()
{
  for (int i = 0; i < 1000; i++)
  {
    // Read-modify-write of a multi bit field: done atomically with modify()
    s.modify([](auto& v)
    {
      v.template assign<counter>(v.template value_of<counter>() + 1);
    });
  }

  // Single atomic fetch_or
  s.set<Flag>();
}

int main()
{
  std::vector<std::thread> threads;
  threads.emplace_back(worker<flag_0>);
  threads.emplace_back(worker<flag_1>);
  threads.emplace_back(worker<flag_2>);
  threads.emplace_back(worker<flag_3>);

  for (auto& t : threads)
  {
    t.join();
  }

  std::cout << "counter   : " << s.value_of<counter>() << " (expected 4000)\n";
  std::cout << "all flags : " << (s.test<flag_0>() && s.test<flag_1>() &&
                                  s.test<flag_2>() && s.test<flag_3>()) << "\n";
  std::cout << "lock free : " << status::is_always_lock_free << "\n";

  // set/clear/toggle return the previous contents
  auto previous = s.clear<flag_0, flag_1>();
  std::cout << "flag_0 before clear : " << previous.test<flag_0>() << "\n";
  std::cout << "flag_0 after clear  : " << s.test<flag_0>() << "\n";

  return 0;
}
