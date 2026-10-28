/// \file storage.hpp
/// Implements memory place holder for specific pack.
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

#ifndef BIT_STORAGE_HPP
#define BIT_STORAGE_HPP

#include "field.hpp"
#include "pack.hpp"

namespace bit
{

template
<
  typename Pack 
>
struct storage
{
  typedef Pack                          pack;
  typedef typename Pack::value_type     value_type;
  
  constexpr storage(const value_type v = 0)
  : m_value(v)
  {}

  // Copy and move operations are implicitly defined, so that storage
  // stays trivially copyable and can be returned by value.

  /// Assigns the value in field region.
  template<typename Field>
  constexpr storage& assign(value_type v)
  {
    const value_type  mask = pack::template get_field_mask<Field>();
    m_value = value_type((m_value & value_type(~mask)) | bit::set<Field>(v));
    return *this;
  }

  /// Returns the combined mask of multiple field regions
  template<typename... Fields>
  static constexpr value_type get_mask()
  {
    return value_type((pack::template get_field_mask<Fields>() | ...));
  }

  /// Sets all values in multiple field regions
  template<typename... Fields>
  constexpr storage& set()
  {
    m_value = value_type(m_value | get_mask<Fields...>());
    return *this;
  }

  /// Clears all values in multiple field regions
  template<typename... Fields>
  constexpr storage& clear()
  {
    m_value = value_type(m_value & value_type(~get_mask<Fields...>()));
    return *this;
  }

  /// Toggles all values in multiple field regions
  template<typename... Fields>
  constexpr storage& toggle()
  {
    m_value = value_type(m_value ^ get_mask<Fields...>());
    return *this;
  }

  /// Returns the current value of specified field in storage
  template<typename Field>
  constexpr value_type value_of() const
  {
    pack::template assert_if_not_exists<Field>();

    return 
      bit::get<Field>(m_value);
  }
  
  /// Tests the value in field region. Returns true if value is greater than zero.
  template<typename Field>
  constexpr bool test() const
  {
    return (value_of<Field>() > 0); 
  }

  /// Returns the current value of storage
  constexpr value_type value() const
  {
    return m_value;
  }

  value_type   m_value;
};

} // namespace bit

// BIT_STORAGE_HPP
#endif
