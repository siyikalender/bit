/// Basic usage demonstration of volatile register access

#include <cstdint>
#include <iostream>

#include "bit/register.hpp"

// Fields of a hypothetical peripheral control register
struct enable : bit::field<0> {};
struct mode   : bit::field<1, 2> {};
struct irq_en : bit::field<4> {};
struct prescaler : bit::field<8, 15> {};

using control_pack = bit::pack<uint32_t, enable, mode, irq_en, prescaler>;

// On a microcontroller the register is bound to its address, e.g.
//   const bit::register_ref<control_pack> control(0x40001000);
// Here a volatile variable stands in for the hardware register.
volatile uint32_t fake_register = 0;

int main()
{
  const bit::register_ref<control_pack> control(&fake_register);

  // Several fields with a single read and a single write
  control.modify([](auto& s)
  {
    s.template assign<mode>(2)
     .template assign<prescaler>(64)
     .template set<enable>();
  });

  std::cout << "Register -> " << std::hex << control.value() << std::dec << "\n";

  // Single field read-modify-write
  control.set<irq_en>();

  std::cout << "Register -> " << std::hex << control.value() << std::dec << "\n";
  std::cout << "mode      : " << control.value_of<mode>() << "\n";
  std::cout << "prescaler : " << control.value_of<prescaler>() << "\n";
  std::cout << "irq_en    : " << control.test<irq_en>() << "\n";

//  control.set<bit::field<3>>(); // invalid: field not in pack. Should not compile

  return 0;
}
