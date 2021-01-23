/// Basic usage demonstration

#include <cstdint>
#include <iostream>

#include "bit/field.hpp"
#include "bit/pack.hpp"
#include "bit/storage.hpp"

struct bit_1   : bit::field<0> {};
struct bit_2   : bit::field<1> {};
struct bit_3   : bit::field<2> {};
struct field_1 : bit::field<8, 15> {};

bit::storage
<
  bit::pack
  <
    uint16_t,
    bit_1,
    bit_2,
    bit_3,
    field_1
  >
> s0;

int main()
{
  std::cout << "Value of s0 -> " << std::hex << unsigned(s0.value()) << std::dec << "\n\n";
  std::cout << "Assign 1 to bit_1\n";
  std::cout << "Assign 15 to field_1\n";

  s0.assign<bit_1>(1)
    .assign<field_1>(15);

  std::cout << "Value of s0 -> " << std::hex << unsigned(s0.value()) << std::dec << "\n\n";
  std::cout << "Clear bit_1 and field_1\n";

  s0.clear<bit_1, field_1>();

  std::cout << "Value of s0 -> :" << std::hex << unsigned(s0.value()) << std::dec << "\n\n";
  std::cout << "Set bit_2 and bit_3\n";

  s0.set<bit_2, bit_3>();

  std::cout << "Value of s0 -> " << std::hex << unsigned(s0.value()) << std::dec << "\n\n";

  std::cout << "Increase current value of field_1 by one\n";
  s0.assign<field_1>(s0.get<field_1>() + 1);

  std::cout << "Value of s0 -> " << std::hex << unsigned(s0.value()) << std::dec << "\n\n";

  std::cout << "Test bit_1 : "    << s0.test<bit_1>()   << "\n";
  std::cout << "Test bit_2 : "    << s0.test<bit_2>()   << "\n";
  std::cout << "Test bit_3 : "    << s0.test<bit_3>()   << "\n";
  std::cout << "Test field_1 : "  << s0.test<field_1>() << "\n";

  return 0;
}
