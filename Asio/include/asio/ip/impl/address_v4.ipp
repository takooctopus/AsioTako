//
// TAKO: 
// ip/impl/address_v4.ipp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_IMPL_ADDRESS_V4_IPP
#define ASIO_IP_IMPL_ADDRESS_V4_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <climits>
#include <limits>
#include <stdexcept>
#include "asio/error.hpp"
#include "asio/detail/socket_ops.hpp"
#include "asio/detail/throw_error.hpp"
#include "asio/detail/throw_exception.hpp"
#include "asio/ip/address_v4.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

address_v4::address_v4(const address_v4::bytes_type& bytes)
{
#if UCHAR_MAX > 0xFF
  if (bytes[0] > 0xFF || bytes[1] > 0xFF
      || bytes[2] > 0xFF || bytes[3] > 0xFF)
  {
    std::out_of_range ex("address_v4 from bytes_type");
    asio::detail::throw_exception(ex);
  }
#endif // UCHAR_MAX > 0xFF

  using namespace std; // For memcpy.
  memcpy(&addr_.s_addr, bytes.data(), 4);   //要知道bytes_type[]是紧凑的u8*4数组，直接拷贝4个字节就好
}

address_v4::address_v4(address_v4::uint_type addr)
{
  if ((std::numeric_limits<uint_type>::max)() > 0xFFFFFFFF)
  {
    std::out_of_range ex("address_v4 from unsigned integer");
    asio::detail::throw_exception(ex);
  }

  addr_.s_addr = asio::detail::socket_ops::host_to_network_long(   
      static_cast<asio::detail::u_long_type>(addr));    // 一般的话就直接调用htonl就好
}

address_v4::bytes_type address_v4::to_bytes() const ASIO_NOEXCEPT
{
  using namespace std; // For memcpy.
  bytes_type bytes; // bytes数组
#if defined(ASIO_HAS_STD_ARRAY)
  memcpy(bytes.data(), &addr_.s_addr, 4);   //用memcpy拷贝过去
#else // defined(ASIO_HAS_STD_ARRAY)
  memcpy(bytes.elems, &addr_.s_addr, 4);
#endif // defined(ASIO_HAS_STD_ARRAY)
  return bytes;
}

address_v4::uint_type address_v4::to_uint() const ASIO_NOEXCEPT
{
  return asio::detail::socket_ops::network_to_host_long(addr_.s_addr);  //一样ntohl()
}

#if !defined(ASIO_NO_DEPRECATED)
unsigned long address_v4::to_ulong() const
{
  return asio::detail::socket_ops::network_to_host_long(addr_.s_addr);
}
#endif // !defined(ASIO_NO_DEPRECATED)

std::string address_v4::to_string() const
{
  asio::error_code ec;
  char addr_str[asio::detail::max_addr_v4_str_len]; //拿一个字符数组 char a[256]
  const char* addr =
    asio::detail::socket_ops::inet_ntop(
        ASIO_OS_DEF(AF_INET), &addr_, addr_str,
        asio::detail::max_addr_v4_str_len, 0, ec);  //用ASIO_OS_DEF() 宏进行转换
  if (addr == 0)
    asio::detail::throw_error(ec);  //判断一下转换是否成功
  return addr;
}

#if !defined(ASIO_NO_DEPRECATED)
std::string address_v4::to_string(asio::error_code& ec) const
{
  char addr_str[asio::detail::max_addr_v4_str_len];
  const char* addr =
    asio::detail::socket_ops::inet_ntop(
        ASIO_OS_DEF(AF_INET), &addr_, addr_str,
        asio::detail::max_addr_v4_str_len, 0, ec);
  if (addr == 0)
    return std::string();
  return addr;
}
#endif // !defined(ASIO_NO_DEPRECATED)

bool address_v4::is_loopback() const ASIO_NOEXCEPT
{
  return (to_uint() & 0xFF000000) == 0x7F000000;        //回环127.x.x.x
}

bool address_v4::is_unspecified() const ASIO_NOEXCEPT
{
  return to_uint() == 0;    // 未指定
}

#if !defined(ASIO_NO_DEPRECATED)
bool address_v4::is_class_a() const
{
  return (to_uint() & 0x80000000) == 0;     // 1-126 0xxx.x.x.x
}

bool address_v4::is_class_b() const
{
  return (to_uint() & 0xC0000000) == 0x80000000;    // 128-191 10xx.x.x.x
}

bool address_v4::is_class_c() const
{
  return (to_uint() & 0xE0000000) == 0xC0000000;    // 192-223 110x.x.x.x
}
#endif // !defined(ASIO_NO_DEPRECATED)

bool address_v4::is_multicast() const ASIO_NOEXCEPT
{
  return (to_uint() & 0xF0000000) == 0xE0000000;    //多播1110.x.x.x
}

#if !defined(ASIO_NO_DEPRECATED)
address_v4 address_v4::broadcast(const address_v4& addr, const address_v4& mask)   
{
  return address_v4(addr.to_uint() | (mask.to_uint() ^ 0xFFFFFFFF));    //广播地址， 其掩码得是0
}

address_v4 address_v4::netmask(const address_v4& addr)
{
  if (addr.is_class_a())
    return address_v4(0xFF000000);
  if (addr.is_class_b())
    return address_v4(0xFFFF0000);
  if (addr.is_class_c())
    return address_v4(0xFFFFFF00);
  return address_v4(0xFFFFFFFF);
}
#endif // !defined(ASIO_NO_DEPRECATED)

address_v4 make_address_v4(const char* str)
{
  asio::error_code ec;
  address_v4 addr = make_address_v4(str, ec);
  asio::detail::throw_error(ec);
  return addr;
}

address_v4 make_address_v4(const char* str,
    asio::error_code& ec) ASIO_NOEXCEPT
{
  address_v4::bytes_type bytes;
  if (asio::detail::socket_ops::inet_pton(
        ASIO_OS_DEF(AF_INET), str, &bytes, 0, ec) <= 0)
    return address_v4();    // 调用pton()
  return address_v4(bytes);     //【创建ipv4地址实例】
}

address_v4 make_address_v4(const std::string& str)
{
  return make_address_v4(str.c_str());  //【创建ipv4地址实例】
}

address_v4 make_address_v4(const std::string& str,
    asio::error_code& ec) ASIO_NOEXCEPT
{
  return make_address_v4(str.c_str(), ec);//【创建ipv4地址实例】
}

#if defined(ASIO_HAS_STRING_VIEW)

address_v4 make_address_v4(string_view str)
{
  return make_address_v4(static_cast<std::string>(str));//【创建ipv4地址实例】
}

address_v4 make_address_v4(string_view str,
    asio::error_code& ec) ASIO_NOEXCEPT
{
  return make_address_v4(static_cast<std::string>(str), ec);//【创建ipv4地址实例】
}

#endif // defined(ASIO_HAS_STRING_VIEW)

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_IMPL_ADDRESS_V4_IPP
