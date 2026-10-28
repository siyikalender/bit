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
