//
// TAKO: 基本解析类
// ip/resolver_base.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_RESOLVER_BASE_HPP
#define ASIO_IP_RESOLVER_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/socket_types.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

/// The resolver_base class is used as a base for the basic_resolver class
/// templates to provide a common place to define the flag constants.
class resolver_base
{
public:
#if defined(GENERATING_DOCUMENTATION)
  /// A bitmask type (C++ Std [lib.bitmask.types]).
  typedef unspecified flags;

  /// Determine the canonical name of the host specified in the query.
  static const flags canonical_name = implementation_defined;

  /// Indicate that returned endpoint is intended for use as a locally bound
  /// socket endpoint.
  static const flags passive = implementation_defined;

  /// Host name should be treated as a numeric string defining an IPv4 or IPv6
  /// address and no name resolution should be attempted.
  static const flags numeric_host = implementation_defined;

  /// Service name should be treated as a numeric string defining a port number
  /// and no name resolution should be attempted.
  static const flags numeric_service = implementation_defined;

  /// If the query protocol family is specified as IPv6, return IPv4-mapped
  /// IPv6 addresses on finding no IPv6 addresses.
  static const flags v4_mapped = implementation_defined;

  /// If used with v4_mapped, return all matching IPv6 and IPv4 addresses.
  static const flags all_matching = implementation_defined;

  /// Only return IPv4 addresses if a non-loopback IPv4 address is configured
  /// for the system. Only return IPv6 addresses if a non-loopback IPv6 address
  /// is configured for the system.
  static const flags address_configured = implementation_defined;
#else
  //【addrinfo结构体ai_flags的取值】
  enum flags 
  {
    canonical_name = ASIO_OS_DEF(AI_CANONNAME),         //需要一个规范名(而不是别名)
    passive = ASIO_OS_DEF(AI_PASSIVE),                  //socket地址用于监听绑定
    numeric_host = ASIO_OS_DEF(AI_NUMERICHOST),         //以数字格式返回主机地址
    numeric_service = ASIO_OS_DEF(AI_NUMERICSERV),      //以端口号返回服务
    v4_mapped = ASIO_OS_DEF(AI_V4MAPPED),               //如果没有找到IPv6地址, 则返回映射到IPv6格式的IPv6地址
    all_matching = ASIO_OS_DEF(AI_ALL),                 //查找IPv4和IPv6地址(仅用于AI_V4MAPPED)
    address_configured = ASIO_OS_DEF(AI_ADDRCONFIG)     //查询配置的地址类型(IPv4或IPv6)
  }; 

  // Implement bitmask operations as shown in C++ Std [lib.bitmask.types].

  friend flags operator&(flags x, flags y)  //定义flag之间与运算
  {
    return static_cast<flags>(
        static_cast<unsigned int>(x) & static_cast<unsigned int>(y));
  }

  friend flags operator|(flags x, flags y)  //定义flag之间或运算
  {
    return static_cast<flags>(
        static_cast<unsigned int>(x) | static_cast<unsigned int>(y));
  }

  friend flags operator^(flags x, flags y)  //定义flag之间异或运算
  {
    return static_cast<flags>(
        static_cast<unsigned int>(x) ^ static_cast<unsigned int>(y));
  }

  friend flags operator~(flags x)   //定义flag之间非运算
  {
    return static_cast<flags>(~static_cast<unsigned int>(x));
  }

  friend flags& operator&=(flags& x, flags y)   
  {
    x = x & y;
    return x;
  }

  friend flags& operator|=(flags& x, flags y)
  {
    x = x | y;
    return x;
  }

  friend flags& operator^=(flags& x, flags y)
  {
    x = x ^ y;
    return x;
  }
#endif

protected:
  /// Protected destructor to prevent deletion through this type.
  ~resolver_base()
  {
  }
};

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_RESOLVER_BASE_HPP
