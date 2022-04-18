//
// TAKO: ipv4网络
// ip/network_v4.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2014 Oliver Kowalke (oliver dot kowalke at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_NETWORK_V4_HPP
#define ASIO_IP_NETWORK_V4_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <string>
#include "asio/detail/string_view.hpp"
#include "asio/error_code.hpp"
#include "asio/ip/address_v4_range.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

/// Represents an IPv4 network.
/**
 * The asio::ip::network_v4 class provides the ability to use and
 * manipulate IP version 4 networks.
 * 【asio::ip::network_v4 这个类提供了操作v4网络的方法】
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
class network_v4
{
public:
  /// Default constructor. 【默认构造函数，全部置0】
  network_v4() ASIO_NOEXCEPT
    : address_(),
      prefix_length_(0)
  {
  }

  /// Construct a network based on the specified address and prefix length.【使用address&和网络号长度[u8就可以了，毕竟最长也不过16]初始化】
  ASIO_DECL network_v4(const address_v4& addr,
      unsigned short prefix_len);

  /// Construct network based on the specified address and netmask.【掩码mask实际上也是一个address_v4】
  ASIO_DECL network_v4(const address_v4& addr,
      const address_v4& mask);

  /// Copy constructor.
  network_v4(const network_v4& other) ASIO_NOEXCEPT
    : address_(other.address_),
      prefix_length_(other.prefix_length_)
  {
  }

#if defined(ASIO_HAS_MOVE)
  /// Move constructor.【注意prefix_length_作为u8，内置的built in type是没有move实现的，所以直接复制就好】
  network_v4(network_v4&& other) ASIO_NOEXCEPT
    : address_(ASIO_MOVE_CAST(address_v4)(other.address_)),
      prefix_length_(other.prefix_length_)  
  {
  }
#endif // defined(ASIO_HAS_MOVE)

  /// Assign from another network.
  network_v4& operator=(const network_v4& other) ASIO_NOEXCEPT
  {
    address_ = other.address_;
    prefix_length_ = other.prefix_length_;
    return *this;
  }

#if defined(ASIO_HAS_MOVE)
  /// Move-assign from another network.【同样的移动赋值构造】
  network_v4& operator=(network_v4&& other) ASIO_NOEXCEPT
  {
    address_ = ASIO_MOVE_CAST(address_v4)(other.address_);
    prefix_length_ = other.prefix_length_;
    return *this;
  }
#endif // defined(ASIO_HAS_MOVE)

  /// Obtain the address object specified when the network object was created. 【获取持有的address_v4实例】
  address_v4 address() const ASIO_NOEXCEPT
  {
    return address_;
  }

  /// Obtain the prefix length that was specified when the network object was
  /// created. 【获取主机号的长度u8】
  unsigned short prefix_length() const ASIO_NOEXCEPT
  {
    return prefix_length_;
  }

  /// Obtain the netmask that was specified when the network object was created. 【获取掩码mask】
  ASIO_DECL address_v4 netmask() const ASIO_NOEXCEPT;

  /// Obtain an address object that represents the network address. 【获取网络号 => 将地址与掩码想与就好[注意要全部转换成主机字节序]】
  address_v4 network() const ASIO_NOEXCEPT
  {
    return address_v4(address_.to_uint() & netmask().to_uint());
  }

  /// Obtain an address object that represents the network's broadcast address. 【获取广播地址 => 主机号全部置1】
  address_v4 broadcast() const ASIO_NOEXCEPT
  {
    return address_v4(network().to_uint() | (netmask().to_uint() ^ 0xFFFFFFFF));
  }

  /// Obtain an address range corresponding to the hosts in the network. 【获取地址范围】
  ASIO_DECL address_v4_range hosts() const ASIO_NOEXCEPT;

  /// Obtain the true network address, omitting any host bits. 【新建一个只有网络号和网络号长度的network_v4对象返回】
  network_v4 canonical() const ASIO_NOEXCEPT
  {
    return network_v4(network(), prefix_length());
  }

  /// Test if network is a valid host address. 【只有掩码长度为32时，才是一个正确的主机地址】
  bool is_host() const ASIO_NOEXCEPT
  {
    return prefix_length_ == 32;
  }

  /// Test if a network is a real subnet of another network. 【当前网络是不是另一个网络的子网】
  ASIO_DECL bool is_subnet_of(const network_v4& other) const;

  /// Get the network as an address in dotted decimal format. 【返回点分十进制】
  ASIO_DECL std::string to_string() const;

  /// Get the network as an address in dotted decimal format. 【无异常返回点分十进制】
  ASIO_DECL std::string to_string(asio::error_code& ec) const;

  /// Compare two networks for equality. 【==相等比较，地址和掩码都要相同】
  friend bool operator==(const network_v4& a, const network_v4& b)
  {
    return a.address_ == b.address_ && a.prefix_length_ == b.prefix_length_;
  }

  /// Compare two networks for inequality. 【==的反面】
  friend bool operator!=(const network_v4& a, const network_v4& b)
  {
    return !(a == b);
  }

private:
  address_v4 address_;  //保有一个address_v4实例
  unsigned short prefix_length_; //一个掩码长度
};

/// Create an IPv4 network from an address and prefix length.
/**
* 【使用address_v4和掩码长度创建并返回network_v4的方法】
 * @relates address_v4
 */
inline network_v4 make_network_v4( 
    const address_v4& addr, unsigned short prefix_len)
{
  return network_v4(addr, prefix_len);
}

/// Create an IPv4 network from an address and netmask.
/**
* 【使用address_v4和掩码创建并返回network_v4的方法】
 * @relates address_v4
 */
inline network_v4 make_network_v4(
    const address_v4& addr, const address_v4& mask)
{
  return network_v4(addr, mask);
}

/// Create an IPv4 network from a string containing IP address and prefix
/// length.
/**
* 【使用字符串创建network_v4方法】
 * @relates network_v4
 */
ASIO_DECL network_v4 make_network_v4(const char* str);

/// Create an IPv4 network from a string containing IP address and prefix
/// length.
/**
* 【使用字符串创建network_v4方法[不抛异常]】
 * @relates network_v4
 */
ASIO_DECL network_v4 make_network_v4(
    const char* str, asio::error_code& ec);

/// Create an IPv4 network from a string containing IP address and prefix
/// length.
/**
* 【使用string创建network_v4方法】
 * @relates network_v4
 */
ASIO_DECL network_v4 make_network_v4(const std::string& str);

/// Create an IPv4 network from a string containing IP address and prefix
/// length.
/**
* 【使用string创建network_v4方法[不带异常]】
 * @relates network_v4
 */
ASIO_DECL network_v4 make_network_v4(
    const std::string& str, asio::error_code& ec);

#if defined(ASIO_HAS_STRING_VIEW) \
  || defined(GENERATING_DOCUMENTATION)

/// Create an IPv4 network from a string containing IP address and prefix
/// length.
/**
* 【使用string_view创建network_v4方法】
 * @relates network_v4
 */
ASIO_DECL network_v4 make_network_v4(string_view str);

/// Create an IPv4 network from a string containing IP address and prefix
/// length.
/**
* 【使用string_view创建network_v4方法[不带异常]】
 * @relates network_v4
 */
ASIO_DECL network_v4 make_network_v4(
    string_view str, asio::error_code& ec);

#endif // defined(ASIO_HAS_STRING_VIEW)
       //  || defined(GENERATING_DOCUMENTATION)

#if !defined(ASIO_NO_IOSTREAM)

/// Output a network as a string.
/**
 * Used to output a human-readable string for a specified network.
 * 重写流输出方法
 *
 * @param os The output stream to which the string will be written.
 *
 * @param net The network to be written.
 *
 * @return The output stream.
 *
 * @relates asio::ip::address_v4
 */
template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(
    std::basic_ostream<Elem, Traits>& os, const network_v4& net);

#endif // !defined(ASIO_NO_IOSTREAM)

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#include "asio/ip/impl/network_v4.hpp"
#if defined(ASIO_HEADER_ONLY)
# include "asio/ip/impl/network_v4.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_IP_NETWORK_V4_HPP
