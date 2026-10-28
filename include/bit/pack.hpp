/// \file pack.hpp
/// Packs fields in specified integral type. Checks field existence and overlapping.
/// It is recommended to use only unsigned integer types for ValueType template argument
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

/// \author Selcuk Iyikalender
/// \date   2021

#ifndef BIT_PACK_HPP
#define BIT_PACK_HPP

#include <cstdint>
#include <type_traits>
#include <utility>

#include "field.hpp"

namespace bit
{

namespace detail
{

template <typename T>
struct always_false : std::false_type {};

// Only uint8_t, uint16_t, uint32_t and uint64_t are valid pack value types.
// std::atomic is intentionally rejected: storage keeps a plain copy of the
// value, so operations on it would not be atomic.
template <typename A>
struct value_type_of
{
  static_assert(always_false<A>::value,
                "Pack value type must be uint8_t, uint16_t, uint32_t or uint64_t");
  typedef A type;
};

template <> struct value_type_of<uint8_t>  { typedef uint8_t  type; };
template <> struct value_type_of<uint16_t> { typedef uint16_t type; };
template <> struct value_type_of<uint32_t> { typedef uint32_t type; };
template <> struct value_type_of<uint64_t> { typedef uint64_t type; };

} // namespace detail

template <typename    ValueType, 
          typename... Fields>
struct pack
{
  typedef 
    typename detail::value_type_of<ValueType>::type
      value_type;

  static constexpr value_type  mask      = 0;

  template<typename OtherField>
  static constexpr bool 
  exists() 
  {
    return false;
  }
  
  template<typename OtherField>
  static constexpr value_type
  get_field_mask_() 
  {
    return 0;
  }
};

template <typename    ValueType, 
          typename    Field,
          typename... Fields>
struct pack<ValueType, Field, Fields...>
{
  typedef 
    typename detail::value_type_of<ValueType>::type
      value_type;

  typedef   pack<ValueType, Fields...>    next;
  typedef   Field                         field;

  static_assert(field::last < bit_width_of<value_type>(),
                "Field exceeds value type width");

  static constexpr value_type   field_mask = 
    mask_range<value_type>(field::first, field::last);

  static constexpr value_type  mask    = 
    field_mask | next::mask;

  static constexpr value_type  check   = 
    field_mask & next::mask;
    
  template<typename OtherField>
  static constexpr bool 
  exists() 
  {
    return std::is_same<field, OtherField>() || 
      next::template exists<OtherField>();
  }

  template<typename OtherField>
  static constexpr void assert_if_not_exists()
  {
    static_assert(exists<OtherField>(), "Field not in pack");
  }

  template<typename OtherField>
  static constexpr value_type
  get_field_mask() 
  {
    assert_if_not_exists<OtherField>();

    return get_field_mask_<OtherField>();
  }

  template<typename OtherField>
  static constexpr value_type
  get_field_mask_() 
  {
    return 
      std::conditional
      <
        std::is_same
        <
          field, 
          OtherField
        >::value,
        std::integral_constant
        <
          value_type, 
          field_mask
        >,
        std::integral_constant
        <
          value_type, 
          next::template get_field_mask_<OtherField>()
        >
      >::type::value;
  }

  static_assert(check == 0, "Fields overlap" );
};

} // namespace bit

// BIT_PACK_HPP
#endif
