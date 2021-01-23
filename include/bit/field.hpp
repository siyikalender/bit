/// \file field.hpp
/// Basic definition of field struct and some helper functions
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

#ifndef BIT_FIELD_HPP
#define BIT_FIELD_HPP

#include <cstddef>

namespace bit
{

template<std::size_t First, std::size_t Last = First>
struct field
{
  static constexpr std::size_t first   = First;
  static constexpr std::size_t last    = Last;
  static constexpr std::size_t size    = Last - First + 1;
  
  static_assert(last >= first, "First index cannot be greater than Last index" );
};

/// helper function to create bit mask (all one's) in specified range. For internal use only
template<typename T>
constexpr T 
mask_range(std::size_t First,
           std::size_t Last)
{
  T result(0);
  
  for (std::size_t i = First; i <= Last; i++)
  {
    result |= T(T(1) << i);
  }
  
  return result;
}

/// Sets the value in field region. For internal use only
template<typename Field, typename ValueType>
constexpr ValueType set(ValueType v)
{
  return 
    (v << Field::first) & 
      mask_range<ValueType>(Field::first, Field::last);
}

/// Gets the value in field region. For internal use only 
template<typename Field, typename ValueType>
constexpr ValueType get(ValueType v)
{
  return 
    (v & mask_range<ValueType>(Field::first, Field::last)) 
      >> Field::first;
}

} // namespace bit

// BIT_FIELD_HPP
#endif

