//
// TAKO������socket option�İ�����
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

// Helper template for implementing boolean-based options.������socket��bool���Ͳ����İ����ࡿ��ģ����level��optName��
template <int Level, int Name>
class boolean
{
public:
  // Default constructor.��Ĭ�Ϲ���Ϊ0��
  boolean()
    : value_(0)
  {
  }

  // Construct with a specific option value.����ֵ���졿
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

  // Get the level of the socket option. �������׽���ѡ��ļ���level��
  template <typename Protocol>
  int level(const Protocol&) const
  {
    return Level;
  }

  // Get the name of the socket option. ��ѡ�����ơ�
  template <typename Protocol>
  int name(const Protocol&) const
  {
    return Name;
  }

  // Get the address of the boolean data. ����ȡ���õ�ֵ��ָ�롿
  template <typename Protocol>
  int* data(const Protocol&)
  {
    return &value_;
  }

  // Get the address of the boolean data. ����ȡ���õ�ֵ�ĳ�ָ�롿
  template <typename Protocol>
  const int* data(const Protocol&) const
  {
    return &value_;
  }

  // Get the size of the boolean data. ����ȡ���õ�ֵ�ĳ���[��ԪֵҲ���õ�int�洢]��
  template <typename Protocol>
  std::size_t size(const Protocol&) const
  {
    return sizeof(value_);
  }

  // Set the size of the boolean data.  �����ö�Ԫֵ�ĳ��ȡ�
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
  int value_;   //��Ԫֵ��int�洢
};

// Helper template for implementing integer options.��setsocket/getsocket intֵ��ģ���ࡿ
template <int Level, int Name>
class integer
{
public:
  // Default constructor.   ��Ĭ�Ϲ���ʹ��0��
  integer()
    : value_(0)
  {
  }

  // Construct with a specific option value. ���ر�ֵ��
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

  // Get the level of the socket option. ������socket���õļ���
  template <typename Protocol>
  int level(const Protocol&) const
  {
    return Level;
  }

  // Get the name of the socket option. ������socket���õ�ѡ������
  template <typename Protocol>
  int name(const Protocol&) const
  {
    return Name;
  }

  // Get the address of the int data.   ����������ֵ��ָ�롿
  template <typename Protocol>
  int* data(const Protocol&)
  {
    return &value_;
  }

  // Get the address of the int data. ����������ֵ�ĳ�ָ�롿
  template <typename Protocol>
  const int* data(const Protocol&) const
  {
    return &value_;
  }

  // Get the size of the int data. ���������ݴ�Сint => 32��
  template <typename Protocol>
  std::size_t size(const Protocol&) const
  {
    return sizeof(value_);
  }

  // Set the size of the int data.  ���������ݴ�С ֻ����32��
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
  int value_;   //int�ľ���int
};

// Helper template for implementing linger options. ������linger������
template <int Level, int Name>
class linger
{
public:
  // Default constructor. ��Ĭ�ϲ����ص�linger����ʱ��Ϊ0��
  linger()
  {
    value_.l_onoff = 0;
    value_.l_linger = 0;
  }

  // Construct with specific option values. ������ֵ��
  linger(bool e, int t)
  {
    enabled(e);
    timeout ASIO_PREVENT_MACRO_SUBSTITUTION(t);
  }

  // Set the value for whether linger is enabled.   ����������linger�����رա�
  void enabled(bool value)
  {
    value_.l_onoff = value ? 1 : 0;
  }

  // Get the value for whether linger is enabled. ��������ȡlinger����״̬��
  bool enabled() const
  {
    return value_.l_onoff != 0;
  }

  // Set the value for the linger timeout.  �����ó�ʱʱ�䡿
  void timeout ASIO_PREVENT_MACRO_SUBSTITUTION(int value) 
  {
#if defined(WIN32)
    value_.l_linger = static_cast<u_short>(value);  //win32��u16
#else
    value_.l_linger = value;    //linux����u32
#endif
  }

  // Get the value for the linger timeout. ����ȡ��ʱʱ��(ת����int)��
  int timeout ASIO_PREVENT_MACRO_SUBSTITUTION() const
  {
    return static_cast<int>(value_.l_linger);
  }

  // Get the level of the socket option. ����ȡsocketopt����
  template <typename Protocol>
  int level(const Protocol&) const
  {
    return Level;
  }

  // Get the name of the socket option. ����ȡsocket optName��
  template <typename Protocol>
  int name(const Protocol&) const
  {
    return Name;
  }

  // Get the address of the linger data. �����õ�ֵָ�롿
  template <typename Protocol>
  detail::linger_type* data(const Protocol&)
  {
    return &value_;
  }

  // Get the address of the linger data. �����õ�ֵ��ָ�롿
  template <typename Protocol>
  const detail::linger_type* data(const Protocol&) const
  {
    return &value_;
  }

  // Get the size of the linger data. ��linger�ṹ���С win��u32, linuxu64��
  template <typename Protocol>
  std::size_t size(const Protocol&) const
  {
    return sizeof(value_);
  }

  // Set the size of the int data. �����ýṹ���С u32/u64��
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
  detail::linger_type value_;   //��linger�����ýṹ�塿
};

} // namespace socket_option
} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_SOCKET_OPTION_HPP
