//
// TAKO:ipv6地址类的实现
// ip/impl/address_v6.ipp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_IMPL_ADDRESS_V6_IPP
#define ASIO_IP_IMPL_ADDRESS_V6_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <cstring>
#include <stdexcept>
#include <typeinfo>
#include "asio/detail/socket_ops.hpp"
#include "asio/detail/throw_error.hpp"
#include "asio/detail/throw_exception.hpp"
#include "asio/error.hpp"
#include "asio/ip/address_v6.hpp"
#include "asio/ip/bad_address_cast.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

address_v6::address_v6() ASIO_NOEXCEPT
  : addr_(),
    scope_id_(0)
{   //默认构造函数scope_id设置为0
}

address_v6::address_v6(const address_v6::bytes_type& bytes,
    scope_id_type scope)
  : scope_id_(scope)
{
#if UCHAR_MAX > 0xFF
  for (std::size_t i = 0; i < bytes.size(); ++i)
  {
    if (bytes[i] > 0xFF)
    {
      std::out_of_range ex("address_v6 from bytes_type");
      asio::detail::throw_exception(ex);
    }
  }
#endif // UCHAR_MAX > 0xFF

  using namespace std; // For memcpy.
  memcpy(addr_.s6_addr, bytes.data(), 16); //memcpy拷贝16个字节
}

address_v6::address_v6(const address_v6& other) ASIO_NOEXCEPT
  : addr_(other.addr_),
    scope_id_(other.scope_id_)
{
}

#if defined(ASIO_HAS_MOVE)
address_v6::address_v6(address_v6&& other) ASIO_NOEXCEPT
  : addr_(other.addr_),
    scope_id_(other.scope_id_)
{
}
#endif // defined(ASIO_HAS_MOVE)

address_v6& address_v6::operator=(const address_v6& other) ASIO_NOEXCEPT
{
  addr_ = other.addr_;
  scope_id_ = other.scope_id_;
  return *this;
}

#if defined(ASIO_HAS_MOVE)
address_v6& address_v6::operator=(address_v6&& other) ASIO_NOEXCEPT
{
  addr_ = other.addr_;
  scope_id_ = other.scope_id_;
  return *this;
}
#endif // defined(ASIO_HAS_MOVE)

address_v6::bytes_type address_v6::to_bytes() const ASIO_NOEXCEPT
{
  using namespace std; // For memcpy.
  bytes_type bytes;
#if defined(ASIO_HAS_STD_ARRAY)
  memcpy(bytes.data(), addr_.s6_addr, 16);  //反向memcpy16字节
#else // defined(ASIO_HAS_STD_ARRAY)
  memcpy(bytes.elems, addr_.s6_addr, 16);
#endif // defined(ASIO_HAS_STD_ARRAY)
  return bytes;
}

std::string address_v6::to_string() const
{
  asio::error_code ec;
  char addr_str[asio::detail::max_addr_v6_str_len]; //申请大小256的char数组，
  const char* addr =
    asio::detail::socket_ops::inet_ntop(
        ASIO_OS_DEF(AF_INET6), &addr_, addr_str,
        asio::detail::max_addr_v6_str_len, scope_id_, ec); // 这个调用后addr_str有数据，要是ipv6还有网卡名
  if (addr == 0)
    asio::detail::throw_error(ec);  //要抛异常的
  return addr;
}

#if !defined(ASIO_NO_DEPRECATED)
std::string address_v6::to_string(asio::error_code& ec) const
{
  char addr_str[asio::detail::max_addr_v6_str_len];
  const char* addr =
    asio::detail::socket_ops::inet_ntop(
        ASIO_OS_DEF(AF_INET6), &addr_, addr_str,
        asio::detail::max_addr_v6_str_len, scope_id_, ec);
  if (addr == 0)
    return std::string();
  return addr;
}

address_v4 address_v6::to_v4() const
{
  if (!is_v4_mapped() && !is_v4_compatible())   //不能转换的情况
  {
    bad_address_cast ex;
    asio::detail::throw_exception(ex);
  }

  address_v4::bytes_type v4_bytes = { { addr_.s6_addr[12],
    addr_.s6_addr[13], addr_.s6_addr[14], addr_.s6_addr[15] } };
  return address_v4(v4_bytes);
}
#endif // !defined(ASIO_NO_DEPRECATED)

// 【::1/128】
bool address_v6::is_loopback() const ASIO_NOEXCEPT  
{
  return ((addr_.s6_addr[0] == 0) && (addr_.s6_addr[1] == 0)
      && (addr_.s6_addr[2] == 0) && (addr_.s6_addr[3] == 0)
      && (addr_.s6_addr[4] == 0) && (addr_.s6_addr[5] == 0)
      && (addr_.s6_addr[6] == 0) && (addr_.s6_addr[7] == 0)
      && (addr_.s6_addr[8] == 0) && (addr_.s6_addr[9] == 0)
      && (addr_.s6_addr[10] == 0) && (addr_.s6_addr[11] == 0)
      && (addr_.s6_addr[12] == 0) && (addr_.s6_addr[13] == 0)
      && (addr_.s6_addr[14] == 0) && (addr_.s6_addr[15] == 1));
}

// 【::128】
bool address_v6::is_unspecified() const ASIO_NOEXCEPT
{
  return ((addr_.s6_addr[0] == 0) && (addr_.s6_addr[1] == 0)
      && (addr_.s6_addr[2] == 0) && (addr_.s6_addr[3] == 0)
      && (addr_.s6_addr[4] == 0) && (addr_.s6_addr[5] == 0)
      && (addr_.s6_addr[6] == 0) && (addr_.s6_addr[7] == 0)
      && (addr_.s6_addr[8] == 0) && (addr_.s6_addr[9] == 0)
      && (addr_.s6_addr[10] == 0) && (addr_.s6_addr[11] == 0)
      && (addr_.s6_addr[12] == 0) && (addr_.s6_addr[13] == 0)
      && (addr_.s6_addr[14] == 0) && (addr_.s6_addr[15] == 0));
}

// fe80::/10
bool address_v6::is_link_local() const ASIO_NOEXCEPT
{
  return ((addr_.s6_addr[0] == 0xfe) && ((addr_.s6_addr[1] & 0xc0) == 0x80));
}

bool address_v6::is_site_local() const ASIO_NOEXCEPT
{
  return ((addr_.s6_addr[0] == 0xfe) && ((addr_.s6_addr[1] & 0xc0) == 0xc0));
}

bool address_v6::is_v4_mapped() const ASIO_NOEXCEPT
{
  return ((addr_.s6_addr[0] == 0) && (addr_.s6_addr[1] == 0)
      && (addr_.s6_addr[2] == 0) && (addr_.s6_addr[3] == 0)
      && (addr_.s6_addr[4] == 0) && (addr_.s6_addr[5] == 0)
      && (addr_.s6_addr[6] == 0) && (addr_.s6_addr[7] == 0)
      && (addr_.s6_addr[8] == 0) && (addr_.s6_addr[9] == 0)
      && (addr_.s6_addr[10] == 0xff) && (addr_.s6_addr[11] == 0xff)); //判断是不是v4地址转换的 0000:0000:0000:0000:0000:ffff:xxxx:xxxx /96
}

#if !defined(ASIO_NO_DEPRECATED)
bool address_v6::is_v4_compatible() const
{
  return ((addr_.s6_addr[0] == 0) && (addr_.s6_addr[1] == 0)
      && (addr_.s6_addr[2] == 0) && (addr_.s6_addr[3] == 0)
      && (addr_.s6_addr[4] == 0) && (addr_.s6_addr[5] == 0)
      && (addr_.s6_addr[6] == 0) && (addr_.s6_addr[7] == 0)
      && (addr_.s6_addr[8] == 0) && (addr_.s6_addr[9] == 0)
      && (addr_.s6_addr[10] == 0) && (addr_.s6_addr[11] == 0)
      && !((addr_.s6_addr[12] == 0)
        && (addr_.s6_addr[13] == 0)
        && (addr_.s6_addr[14] == 0)
        && ((addr_.s6_addr[15] == 0) || (addr_.s6_addr[15] == 1))));
}
#endif // !defined(ASIO_NO_DEPRECATED)

bool address_v6::is_multicast() const ASIO_NOEXCEPT // ff00::/8
{
  return (addr_.s6_addr[0] == 0xff);    //ipv6的多播，首八位为11111111
}

bool address_v6::is_multicast_global() const ASIO_NOEXCEPT
{
  return ((addr_.s6_addr[0] == 0xff) && ((addr_.s6_addr[1] & 0x0f) == 0x0e));   // ff0e::/8
}

bool address_v6::is_multicast_link_local() const ASIO_NOEXCEPT
{
  return ((addr_.s6_addr[0] == 0xff) && ((addr_.s6_addr[1] & 0x0f) == 0x02)); // ff02::/8
}

bool address_v6::is_multicast_node_local() const ASIO_NOEXCEPT
{
  return ((addr_.s6_addr[0] == 0xff) && ((addr_.s6_addr[1] & 0x0f) == 0x01)); // ff01::/8
}

bool address_v6::is_multicast_org_local() const ASIO_NOEXCEPT
{
    //所有区域的NTP服务器
  return ((addr_.s6_addr[0] == 0xff) && ((addr_.s6_addr[1] & 0x0f) == 0x08)); // ff08::/8
}

bool address_v6::is_multicast_site_local() const ASIO_NOEXCEPT
{
  return ((addr_.s6_addr[0] == 0xff) && ((addr_.s6_addr[1] & 0x0f) == 0x05)); //ff05::/8
}

bool operator==(const address_v6& a1, const address_v6& a2) ASIO_NOEXCEPT
{
  using namespace std; // For memcmp. 使用memcmp去比较长度的字符
  return memcmp(&a1.addr_, &a2.addr_,
      sizeof(asio::detail::in6_addr_type)) == 0
    && a1.scope_id_ == a2.scope_id_;    //v6的相等还要比较scope_id
}

bool operator<(const address_v6& a1, const address_v6& a2) ASIO_NOEXCEPT
{
  using namespace std; // For memcmp.
  int memcmp_result = memcmp(&a1.addr_, &a2.addr_,
      sizeof(asio::detail::in6_addr_type)); 
  if (memcmp_result < 0)
    return true;
  if (memcmp_result > 0)
    return false;
  return a1.scope_id_ < a2.scope_id_;   //地址相等还要比较scope_id的大小
}

address_v6 address_v6::loopback() ASIO_NOEXCEPT
{
  address_v6 tmp;
  tmp.addr_.s6_addr[15] = 1;    //生成一个本地回环地址 ::1/128
  return tmp;
}

#if !defined(ASIO_NO_DEPRECATED)
address_v6 address_v6::v4_mapped(const address_v4& addr)
{
  address_v4::bytes_type v4_bytes = addr.to_bytes();
  bytes_type v6_bytes = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF,
    v4_bytes[0], v4_bytes[1], v4_bytes[2], v4_bytes[3] } };
  return address_v6(v6_bytes);
}

address_v6 address_v6::v4_compatible(const address_v4& addr)
{
  address_v4::bytes_type v4_bytes = addr.to_bytes();
  bytes_type v6_bytes = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    v4_bytes[0], v4_bytes[1], v4_bytes[2], v4_bytes[3] } };
  return address_v6(v6_bytes);
}
#endif // !defined(ASIO_NO_DEPRECATED)

address_v6 make_address_v6(const char* str)
{
  asio::error_code ec;
  address_v6 addr = make_address_v6(str, ec); //调用下面的函数
  asio::detail::throw_error(ec);
  return addr;
}

address_v6 make_address_v6(const char* str,
    asio::error_code& ec) ASIO_NOEXCEPT
{
  address_v6::bytes_type bytes;
  unsigned long scope_id = 0;   
  if (asio::detail::socket_ops::inet_pton(
        ASIO_OS_DEF(AF_INET6), str, &bytes[0], &scope_id, ec) <= 0)
    return address_v6(); 
  return address_v6(bytes, scope_id); //调用helper方法构造，下去看helper方法
}

address_v6 make_address_v6(const std::string& str)
{
  return make_address_v6(str.c_str()); //调用上面函数
}

address_v6 make_address_v6(const std::string& str,
    asio::error_code& ec) ASIO_NOEXCEPT
{
  return make_address_v6(str.c_str(), ec); //调用上面函数
}

#if defined(ASIO_HAS_STRING_VIEW)

address_v6 make_address_v6(string_view str)
{
  return make_address_v6(static_cast<std::string>(str)); //调用上面函数
}

address_v6 make_address_v6(string_view str,
    asio::error_code& ec) ASIO_NOEXCEPT
{
  return make_address_v6(static_cast<std::string>(str), ec); //调用上面函数
}

#endif // defined(ASIO_HAS_STRING_VIEW)

address_v4 make_address_v4(
    v4_mapped_t, const address_v6& v6_addr)
{
  if (!v6_addr.is_v4_mapped())
  {
    bad_address_cast ex; 
    asio::detail::throw_exception(ex); //不能转就抛异常
  }

  address_v6::bytes_type v6_bytes = v6_addr.to_bytes(); //先转换成u8*16数组
  address_v4::bytes_type v4_bytes = { { v6_bytes[12], //用上面的后4段进行初始化
    v6_bytes[13], v6_bytes[14], v6_bytes[15] } };
  return address_v4(v4_bytes); //返回新建的对象
}

address_v6 make_address_v6(
    v4_mapped_t, const address_v4& v4_addr)
{
  address_v4::bytes_type v4_bytes = v4_addr.to_bytes();  //先转换成u8*4数组
  address_v6::bytes_type v6_bytes = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0xFF, 0xFF, v4_bytes[0], v4_bytes[1], v4_bytes[2], v4_bytes[3] } }; //根据map规则转换成 0000:0000:0000:0000:0000:ffff:xxxx:xxxx
  return address_v6(v6_bytes); //构造实例
}

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_IMPL_ADDRESS_V6_IPP
