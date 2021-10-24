/// Basic usage demonstration of differect pack value types 

#include <cstdint>
#include <iostream>

#include <atomic>

#include "bit/field.hpp"
#include "bit/pack.hpp"
#include "bit/storage.hpp"

// Define your fields with first and last bit indices. Note that last bit is included in range. 
// If the last bit index is omitted, the field is assumed to be in one bit size.

struct a   : bit::field<0> {};
struct b   : bit::field<1> {};

// valid
using test_storage_1 =
  bit::storage
  <
    bit::pack
    <
      uint32_t,
      a,
      b
    >
  >;
  
// valid
using test_storage_2 =
  bit::storage
  <
    bit::pack
    <
      std::atomic<uint32_t>,
      a,
      b
    >
  >;  

// invalid
using test_storage_3 =
  bit::storage
  <
    bit::pack
    <
      int32_t,
      a,
      b
    >
  >;

// invalid
using test_storage_4 =
  bit::storage
  <
    bit::pack
    <
      std::atomic<int32_t>,
      a,
      b
    >
  >;  

int main()
{
  std::cout << 
    std::is_same
      <
        test_storage_1::value_type, 
        uint32_t
      >::value << "\n";

  std::cout << 
    std::is_same
      <
        test_storage_2::value_type, 
        uint32_t
      >::value << "\n";
  
  test_storage_1 ts_1;
  
  test_storage_2 ts_2;
  
  ts_1.set<a>();
  
  ts_2.set<a>();

//  std::cout <<  // compile error
//    std::is_same
//      <
//        test_storage_3::value_type, 
//        int32_t
//      >::value << "\n";

//  std::cout << // compile error
//    std::is_same
//      <
//        test_storage_4::value_type, 
//        int32_t
//      >::value << "\n";

  return 0;
}
