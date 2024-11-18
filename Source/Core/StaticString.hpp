/*
Compile-time string manipulation library for modern C++
version 1.0.1
https://github.com/snw1/static-string-cpp

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2017-2018 Andrew Sheetov <andrew.sheetov@gmail.com>

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#pragma once
#include <algorithm>
#include <array>
#include <cctype>
#include <limits>
#include <string>
#include <string_view>

#include "Utility.hpp"

namespace Core
{

template<typename TChar, size_t Size>
class basic_static_string
{
  std::array<TChar, Size> data;

public:
  /// @brief default constructor
  constexpr basic_static_string() noexcept { data.fill(static_cast<TChar>('\0')); }
  /// @brief constructor from static string literal
  constexpr basic_static_string(const TChar (&str)[Size]) noexcept
  {
    std::copy(std::begin(str), std::end(str), begin());
  }

  static constexpr size_t npos = static_cast<size_t>(-1);

  constexpr auto begin() noexcept { return data.begin(); }
  constexpr auto end() noexcept { return data.end(); }
  constexpr auto begin() const noexcept { return data.begin(); }
  constexpr auto end() const noexcept { return data.end(); }
  constexpr auto cbegin() const noexcept { return data.cbegin(); }
  constexpr auto cend() const noexcept { return data.cend(); }
  constexpr auto rbegin() noexcept { return data.rbegin(); }
  constexpr auto rend() noexcept { return data.rend(); }
  constexpr auto rbegin() const noexcept { return data.rbegin(); }
  constexpr auto rend() const noexcept { return data.rend(); }
  constexpr auto crbegin() const noexcept { return data.crbegin(); }
  constexpr auto crend() const noexcept { return data.crend(); }

  constexpr TChar & at(size_t idx) { return data[idx]; }
  constexpr const TChar & at(size_t idx) const { return data[idx]; }
  constexpr TChar & operator[](std::size_t idx) { return data[idx]; }
  constexpr const TChar & operator[](std::size_t idx) const { return data[idx]; }

  constexpr bool empty() const noexcept { return length() == 0; }
  constexpr size_t length() const noexcept { return Size - 1; }
  constexpr TChar * c_str() noexcept { return data.data(); }
  constexpr const TChar * c_str() const noexcept { return data.data(); }
  operator std::basic_string<TChar>() const noexcept
  {
    return std::basic_string<TChar>(data.data());
  }

  constexpr void assign(const basic_static_string<TChar, Size> & other) noexcept
  {
    data = other.data;
  }

  template<std::size_t Size2>
  constexpr auto append(const basic_static_string<TChar, Size2> & other) const noexcept
  {
    basic_static_string<TChar, Size + Size2 - 1> result;
    std::copy(begin(), end(), result.begin());
    std::copy(other.begin(), other.end(), result.begin() + Size - 1);
    return result;
  }

  template<std::size_t Size2>
  constexpr auto append(const TChar (&str)[Size2]) const noexcept
  {
    basic_static_string<TChar, Size + Size2 - 1> result;
    std::copy(begin(), end(), result.begin());
    std::copy(std::begin(str), std::end(str), result.begin() + Size - 1);
    return result;
  }

  template<std::size_t Size2>
  constexpr auto operator+(const basic_static_string<TChar, Size2> & other) const noexcept
  {
    return append(other);
  }

  template<std::size_t Size2>
  constexpr auto operator+(const TChar (&str)[Size2]) const noexcept
  {
    return append(str);
  }

  /// @brief reverse a string (creates new string)
  constexpr auto reverse() const
  {
    basic_static_string<TChar, Size> result = *this;
    result.reverse();
    return result;
  }

  /// @brief reverse a string (change exist string)
  constexpr void reverse() { std::reverse(begin(), end()); }

  /// @brief get substring of static string
  template<size_t Begin, size_t Count>
  constexpr auto substring() const
  {
    basic_static_string<TChar, Count + 1> result;
    static_assert(Begin + Count <= Size - 1, "Out of range of string");
    auto offset = begin() + Begin;
    std::copy(offset, offset + Count, result.begin());
    result[Count] = static_cast<TChar>('\0');
    return result;
  }

  /// @brief get prefix of string (Take N first symbols)
  template<size_t N>
  constexpr auto prefix() const
  {
    return substring<0, N>();
  }

  /// @brief get a suffix of string from Begin position
  template<size_t Begin>
  constexpr auto suffix() const
  {
    return substring<Begin, Size - 1 - Begin>();
  }

  //template<size_t Size2>
  //constexpr int compare(const basic_static_string<TChar, Size2>& other)
  //{
  //  int res = Size - Size2;
  //  if (res == 0)
  //  {
  //    res = std::;

  //  }
  //  return res;
  //}

  // TODO: make compare, find, rfind, starts_with, ends_with, etc
};

template<size_t Size>
using static_string = basic_static_string<char, Size>;
template<size_t Size>
using static_wstring = basic_static_string<wchar_t, Size>;


template<typename TChar, size_t Size>
std::basic_ostream<TChar> & operator<<(std::basic_ostream<TChar> & bos,
                                       const basic_static_string<TChar, Size> & str)
{
  bos << str.c_str();
  return bos;
}

} // namespace Core
