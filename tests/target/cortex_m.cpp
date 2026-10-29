/// \file cortex_m.cpp
/// Compile only check for bare metal ARM Cortex-M targets (arm-none-eabi).
/// Exercises every header without exceptions, RTTI or an operating system.
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

#include "bit/atomic_storage.hpp"
#include "bit/register.hpp"
#include "bit/storage.hpp"

// A typical peripheral control register
struct enable    : bit::field<0> {};
struct mode      : bit::field<1, 2> {};
struct irq_en    : bit::field<4> {};
struct prescaler : bit::field<8, 15> {};

using control_pack = bit::pack<std::uint32_t, enable, mode, irq_en, prescaler>;

// Reset value computed at compile time
constexpr bit::storage<control_pack> control_reset =
  bit::storage<control_pack>().assign<mode, 1>().assign<prescaler, 64>();

static_assert(control_reset.value() == 0x4002, "");

#if defined(__ARM_FEATURE_LDREX)
// Cores with exclusive access instructions (Cortex-M3 and up) need no lock
static_assert(bit::atomic_storage<control_pack>::is_always_lock_free, "");
#endif

extern "C" void control_init()
{
  const bit::register_ref<control_pack> control(0x40001000u);

  control.store(control_reset);
  control.modify([](bit::storage<control_pack>& s)
  {
    s.set<enable, irq_en>();
  });
}

extern "C" bool control_set_prescaler(std::uint32_t value)
{
  const bit::register_ref<control_pack> control(0x40001000u);

  return control.checked_assign<prescaler>(value);
}

extern "C" bool control_running()
{
  const bit::register_ref<control_pack> control(0x40001000u);

  return control.test_all<enable, irq_en>();
}

// Flags shared between an interrupt handler and the main loop
bit::atomic_storage<control_pack> pending;

extern "C" void irq_handler()
{
  pending.set<irq_en>(std::memory_order_release);
}

extern "C" bool take_pending()
{
  return pending.clear<irq_en>(std::memory_order_acquire).test<irq_en>();
}
