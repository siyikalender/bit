/// Basic usage demonstration for compile time checking features

#include <cstdint>
#include <iostream>

#include "bit/field.hpp"
#include "bit/pack.hpp"
#include "bit/storage.hpp"

// Define your fields with first and last bit indices. Note that last bit is included in range. 
// If the last bit index is omitted, the field is assumed to be in one bit size.
struct a   : bit::field<0> {};
struct a2  : bit::field<0> {};
struct b   : bit::field<1> {};

struct c   : bit::field<0> {};
struct d   : bit::field<1> {};

bit::storage
<
  bit::pack
  <
    uint32_t,
    a,
    // a2, // invalid since a2 overlaps with a. Should not compile
    b
  >
> s1;

bit::storage
<
  bit::pack
  <
    uint32_t,
    c,
    d
  >
> s2;

int main()
{
  s1.set<a, b>(); // valid
  
//  s1.set<c>(); // invalid field "c" is not defined in the pack of s1. Should not compile
  
  s2.clear<c, d>(); // valid

//   s2.set<b>(); // invalid field "b" is not defined in the pack of s2. Should not compile

  return 0;
}
