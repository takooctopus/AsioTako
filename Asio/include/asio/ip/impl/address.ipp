//
// TAKO address.hpp的实现
// ip/impl/address.ipp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_IMPL_ADDRESS_IPP
#define ASIO_IP_IMPL_ADDRESS_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <typeinfo>
#include "asio/detail/throw_error.hpp"
#include "asio/detail/throw_exception.hpp"
#include "asio/error.hpp"
#include "asio/ip/address.hpp"
#include "asio/ip/bad_address_cast.hpp"
#include "asio/system_error.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

// 默认构造函数使用ipv4类型初始化
address::address() ASIO_NOEXCEPT
  : type_(ipv4),
    ipv4_address_(),
    ipv6_address_()
{
}

// ipv4的初始化
address::address(
    const asio::ip::address_v4& ipv4_address) ASIO_NOEXCEPT
  : type_(ipv4),
    ipv4_address_(ipv4_address),
    ipv6_address_()
{
}

// ipv6的初始化
address::address(
    const asio::ip::address_v6& ipv6_address) ASIO_NOEXCEPT
  : type_(ipv6),
    ipv4_address_(),
    ipv6_address_(ipv6_address)
{
}

// 拷贝构造
address::address(const address& other) ASIO_NOEXCEPT
  : type_(other.type_),
    ipv4_address_(other.ipv4_address_),
    ipv6_address_(other.ipv6_address_)
{
}

#if defined(ASIO_HAS_MOVE)
// 支持移动时 移动构造【没有new分配堆，所以可以直接初始化】
address::address(address&& other) ASIO_NOEXCEPT
  : type_(other.type_),
    ipv4_address_(other.ipv4_address_),
    ipv6_address_(other.ipv6_address_)
{
}
#endif // defined(ASIO_HAS_MOVE)

// =拷贝
address& address::operator=(const address& other) ASIO_NOEXCEPT
{
  type_ = other.type_;
  ipv4_address_ = other.ipv4_address_;
  ipv6_address_ = other.ipv6_address_;
  return *this;
}

#if defined(ASIO_HAS_MOVE)
// 支持移动时 =移动
address& address::operator=(address&& other) ASIO_NOEXCEPT
{
  type_ = other.type_;
  ipv4_address_ = other.ipv4_address_;
  ipv6_address_ = other.ipv6_address_;
  return *this;
}
#endif // defined(ASIO_HAS_MOVE)

address& address::operator=(
    const asio::ip::address_v4& ipv4_address) ASIO_NOEXCEPT
{
  type_ = ipv4;
  ipv4_address_ = ipv4_address;
  ipv6_address_ = asio::ip::address_v6();
  return *this;
}

address& address::operator=(
    const asio::ip::address_v6& ipv6_address) ASIO_NOEXCEPT
{
  type_ = ipv6;
  ipv4_address_ = asio::ip::address_v4();
  ipv6_address_ = ipv6_address;
  return *this;
}

// 会抛出异常的创建地址方法
address make_address(const char* str)
{
  asio::error_code ec;
  address addr = make_address(str, ec);
  asio::detail::throw_error(ec);    // 创建失败会抛出异常
  return addr;
}

// 只会返回异常引用的创建地址方法
address make_address(const char* str,
    asio::error_code& ec) ASIO_NOEXCEPT
{
  asio::ip::address_v6 ipv6_address =
    asio::ip::make_address_v6(str, ec);
  if (!ec)
    return address(ipv6_address);

  asio::ip::address_v4 ipv4_address =
    asio::ip::make_address_v4(str, ec);
  if (!ec)
    return address(ipv4_address);

  return address(); //创建失败就返回默认构造，并将异常引用赋值
}

address make_address(const std::string& str)
{
  return make_address(str.c_str()); //string就换成c_str()
}

address make_address(const std::string& str,
    asio::error_code& ec) ASIO_NOEXCEPT
{
  return make_address(str.c_str(), ec); //string就换成c_str()
}

#if defined(ASIO_HAS_STRING_VIEW)

address make_address(string_view str)
{
  return make_address(static_cast<std::string>(str));   //string_view使用static_cast
}

address make_address(string_view str,
    asio::error_code& ec) ASIO_NOEXCEPT
{
  return make_address(static_cast<std::string>(str), ec);  //string_view使用static_cast
}

#endif // defined(ASIO_HAS_STRING_VIEW)

asio::ip::address_v4 address::to_v4() const
{
  if (type_ != ipv4)
  {
    bad_address_cast ex;
    asio::detail::throw_exception(ex);  // 当前是v6地址输出v4抛异常
  }
  return ipv4_address_;
}

asio::ip::address_v6 address::to_v6() const
{
  if (type_ != ipv6)
  {
    bad_address_cast ex;
    asio::detail::throw_exception(ex);  // 当前是v4地址输出v6抛异常
  }
  return ipv6_address_;
}

std::string address::to_string() const
{
  if (type_ == ipv6)                // 根据地址类型返回string【错误会抛异常】
    return ipv6_address_.to_string();
  return ipv4_address_.to_string();
}

#if !defined(ASIO_NO_DEPRECATED)
std::string address::to_string(asio::error_code& ec) const
{
  if (type_ == ipv6)    // 不抛异常的函数被弃用了
    return ipv6_address_.to_string(ec);
  return ipv4_address_.to_string(ec);
}
#endif // !defined(ASIO_NO_DEPRECATED)

bool address::is_loopback() const ASIO_NOEXCEPT
{
  return (type_ == ipv4)    // 根据类型判断是否回环
    ? ipv4_address_.is_loopback()
    : ipv6_address_.is_loopback();
}

bool address::is_unspecified() const ASIO_NOEXCEPT
{
  return (type_ == ipv4)    //根据类型判断是否未指定
    ? ipv4_address_.is_unspecified()
    : ipv6_address_.is_unspecified();
}

bool address::is_multicast() const ASIO_NOEXCEPT
{
  return (type_ == ipv4)    //根据类型判断是否多播
    ? ipv4_address_.is_multicast()
    : ipv6_address_.is_multicast();
}

bool operator==(const address& a1, const address& a2) ASIO_NOEXCEPT
{
  if (a1.type_ != a2.type_) //类型不同肯定不同
    return false;
  if (a1.type_ == address::ipv6)    //根据类型比较
    return a1.ipv6_address_ == a2.ipv6_address_;
  return a1.ipv4_address_ == a2.ipv4_address_;
}

bool operator<(const address& a1, const address& a2) ASIO_NOEXCEPT
{
  if (a1.type_ < a2.type_)  // 类型ipv4必定小于ipv6
    return true;
  if (a1.type_ > a2.type_)
    return false;
  if (a1.type_ == address::ipv6)
    return a1.ipv6_address_ < a2.ipv6_address_;
  return a1.ipv4_address_ < a2.ipv4_address_;
}

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_IMPL_ADDRESS_IPP
