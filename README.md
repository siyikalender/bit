# bit
## Description

This is a header only library for defining bit fields in more C++ way. It features strong type check of bit fields and clean interface for bit field manipulation and testing. 
A C++17 compliant compiler is required.

See sources in "examples" folder for usage.

## Quick start

```cpp
#include <cstdint>
#include "bit/storage.hpp"

struct enable : bit::field<0> {};      // bit 0
struct mode   : bit::field<1, 2> {};   // bits 1..2 (last bit included)

using control = bit::storage<bit::pack<uint32_t, enable, mode>>;

constexpr control c = control().assign<mode>(2).set<enable>();
static_assert(c.value() == 0x5, "");
```

Compile time checks:

- a field that is not in the pack cannot be used with its storage,
- fields of a pack must not overlap,
- every field must fit in the pack value type,
- the value type must be `uint8_t`, `uint16_t`, `uint32_t` or `uint64_t`.

## Values that do not fit in a field

`assign` asserts in debug builds when a value is larger than the field can
hold, and is free of checks when `NDEBUG` is defined (the value is then
truncated to the field width). In a constant expression an oversized value is
a compile error in debug builds.

`checked_assign` checks in every build mode. It writes nothing and returns
`false` when the value does not fit:

```cpp
control c;

if (!c.checked_assign<mode>(value_from_protocol))
{
  // value_from_protocol > mode::max
}
```

`field::max` and `bit::fits<Field>(v)` are available for custom checks.
`atomic_storage` and `register_ref` provide the same `assign` and
`checked_assign`; `register_ref::checked_assign` does not access the
register at all when the value does not fit.

## Hardware registers

`bit/register.hpp` provides `bit::register_ref`, which applies a pack to a
memory mapped (volatile) register. Each operation is a single volatile read
and/or write; `modify` updates several fields with one write:

```cpp
#include "bit/register.hpp"

const bit::register_ref<bit::pack<uint32_t, enable, mode>> ctrl(0x40001000);

ctrl.set<enable>();
ctrl.modify([](auto& s) { s.template assign<mode>(3).template clear<enable>(); });
```

Read-modify-write sequences are not atomic; protect them when a register is
shared with an interrupt handler.

## Thread safe storage

`bit/atomic_storage.hpp` provides `bit::atomic_storage`, which keeps the value
in a `std::atomic`. `set`, `clear` and `toggle` are single `fetch_or`,
`fetch_and` and `fetch_xor` operations; `assign` and `modify` use a
compare-exchange loop. Modifying operations return the previous contents, and
every operation takes an optional `std::memory_order`:

```cpp
#include "bit/atomic_storage.hpp"

bit::atomic_storage<bit::pack<uint32_t, enable, mode>> flags;

flags.set<enable>();                                  // fetch_or
auto before = flags.clear<enable>();                  // previous contents
flags.modify([](auto& s) { s.template assign<mode>(s.template value_of<mode>() + 1); });
```

`modify` may call its function more than once under contention, so the
function must not have side effects.
