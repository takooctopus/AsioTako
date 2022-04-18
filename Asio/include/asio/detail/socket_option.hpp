//
// TAKO：设置socket option的帮助类
// detail/socket_option.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_SOCKET_OPTION_HPP
#define ASIO_DETAIL_SOCKET_OPTION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <cstddef>
#include <stdexcept>
#include "asio/detail/socket_types.hpp"
#include "asio/detail/throw_exception.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {
namespace socket_option {

// Helper template for implementing boolean-based options.【设置socket上bool类型操作的帮助类】【模板是level和optName】
template <int Level, int Name>
class boolean
{
public:
  // Default constructor.【默认构造为0】
  boolean()
    : value_(0)
  {
  }

  // Construct with a specific option value.【有值构造】
  explicit boolean(bool v)
    : value_(v ? 1 : 0)
  {
  }

  // Set the current value of the boolean.
  boolean& operator=(bool v)
  {
    value_ = v ? 1 : 0;
    return *this;
  }

  // Get the current value of the boolean.
  bool value() const
  {
    return !!value_;
  }

  // Convert to bool.
  operator bool() const
  {
    return !!value_;
  }

  // Test for false.
  bool operator!() const
  {
    return !value_;
  }

  // Get the level of the socket option. 【返回套接字选项的级别level】
  template <typename Protocol>
  int level(const Protocol&) const
  {
    return Level;
  }

  // Get the name of the socket option. 【选项名称】
  template <typename Protocol>
  int name(const Protocol&) const
  {
    return Name;
  }

  // Get the address of the boolean data. 【获取设置的值的指针】
  template <typename Protocol>
  int* data(const Protocol&)
  {
    return &value_;
  }

  // Get the address of the boolean data. 【获取设置的值的常指针】
  template <typename Protocol>
  const int* data(const Protocol&) const
  {
    return &value_;
  }

  // Get the size of the boolean data. 【获取设置的值的长度[二元值也是用的int存储]】
  template <typename Protocol>
  std::size_t size(const Protocol&) const
  {
    return sizeof(value_);
  }

  // Set the size of the boolean data.  【设置二元值的长度】
  template <typename Protocol>
  void resize(const Protocol&, std::size_t s)
  {
    // On some platforms (e.g. Windows Vista), the getsockopt function will
    // return the size of a boolean socket option as one byte, even though a
    // four byte integer was passed in.
    switch (s)
    {
    case sizeof(char):
      value_ = *reinterpret_cast<char*>(&value_) ? 1 : 0;  
      break;
    case sizeof(value_):
      break;
    default:
      {
        std::length_error ex("boolean socket option resize");
        asio::detail::throw_exception(ex);
      }
    }
  }

private:
  int value_;   //二元值用int存储
};

// Helper template for implementing integer options.【setsocket/getsocket int值的模板类】
template <int Level, int Name>
class integer
{
public:
  // Default constructor.   【默认构造使用0】
  integer()
    : value_(0)
  {
  }

  // Construct with a specific option value. 【特别值】
  explicit integer(int v)
    : value_(v)
  {
  }

  // Set the value of the int option.
  integer& operator=(int v)
  {
    value_ = v;
    return *this;
  }

  // Get the current value of the int option.
  int value() const
  {
    return value_;
  }

  // Get the level of the socket option. 【返回socket设置的级别】
  template <typename Protocol>
  int level(const Protocol&) const
  {
    return Level;
  }

  // Get the name of the socket option. 【返回socket设置的选项名】
  template <typename Protocol>
  int name(const Protocol&) const
  {
    return Name;
  }

  // Get the address of the int data.   【返回设置值的指针】
  template <typename Protocol>
  int* data(const Protocol&)
  {
    return &value_;
  }

  // Get the address of the int data. 【返回设置值的常指针】
  template <typename Protocol>
  const int* data(const Protocol&) const
  {
    return &value_;
  }

  // Get the size of the int data. 【返回数据大小int => 32】
  template <typename Protocol>
  std::size_t size(const Protocol&) const
  {
    return sizeof(value_);
  }

  // Set the size of the int data.  【设置数据大小 只能是32】
  template <typename Protocol>
  void resize(const Protocol&, std::size_t s)
  {
    if (s != sizeof(value_))
    {
      std::length_error ex("integer socket option resize");
      asio::detail::throw_exception(ex);
    }
  }

private:
  int value_;   //int的就是int
};

// Helper template for implementing linger options. 【设置linger操作】
template <int Level, int Name>
class linger
{
public:
  // Default constructor. 【默认操作关掉linger，延时设为0】
  linger()
  {
    value_.l_onoff = 0;
    value_.l_linger = 0;
  }

  // Construct with specific option values. 【设置值】
  linger(bool e, int t)
  {
    enabled(e);
    timeout ASIO_PREVENT_MACRO_SUBSTITUTION(t);
  }

  // Set the value for whether linger is enabled.   【单独设置linger开启关闭】
  void enabled(bool value)
  {
    value_.l_onoff = value ? 1 : 0;
  }

  // Get the value for whether linger is enabled. 【单独获取linger开启状态】
  bool enabled() const
  {
    return value_.l_onoff != 0;
  }

  // Set the value for the linger timeout.  【设置超时时间】
  void timeout ASIO_PREVENT_MACRO_SUBSTITUTION(int value) 
  {
#if defined(WIN32)
    value_.l_linger = static_cast<u_short>(value);  //win32是u16
#else
    value_.l_linger = value;    //linux下是u32
#endif
  }

  // Get the value for the linger timeout. 【获取超时时间(转换成int)】
  int timeout ASIO_PREVENT_MACRO_SUBSTITUTION() const
  {
    return static_cast<int>(value_.l_linger);
  }

  // Get the level of the socket option. 【获取socketopt级别】
  template <typename Protocol>
  int level(const Protocol&) const
  {
    return Level;
  }

  // Get the name of the socket option. 【获取socket optName】
  template <typename Protocol>
  int name(const Protocol&) const
  {
    return Name;
  }

  // Get the address of the linger data. 【设置的值指针】
  template <typename Protocol>
  detail::linger_type* data(const Protocol&)
  {
    return &value_;
  }

  // Get the address of the linger data. 【设置的值常指针】
  template <typename Protocol>
  const detail::linger_type* data(const Protocol&) const
  {
    return &value_;
  }

  // Get the size of the linger data. 【linger结构体大小 win下u32, linuxu64】
  template <typename Protocol>
  std::size_t size(const Protocol&) const
  {
    return sizeof(value_);
  }

  // Set the size of the int data. 【设置结构体大小 u32/u64】
  template <typename Protocol>
  void resize(const Protocol&, std::size_t s)
  {
    if (s != sizeof(value_))
    {
      std::length_error ex("linger socket option resize");
      asio::detail::throw_exception(ex);
    }
  }

private:
  detail::linger_type value_;   //【linger的设置结构体】
};

} // namespace socket_option
} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_SOCKET_OPTION_HPP
