//
// TAKO:网络原始套接字
// generic/raw_protocol.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_GENERIC_RAW_PROTOCOL_HPP
#define ASIO_GENERIC_RAW_PROTOCOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#include <typeinfo>
#include "asio/basic_raw_socket.hpp"
#include "asio/detail/socket_types.hpp"
#include "asio/detail/throw_exception.hpp"
#include "asio/generic/basic_endpoint.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace generic {

/// Encapsulates the flags needed for a generic raw socket.
/**
 * The asio::generic::raw_protocol class contains flags necessary for
 * raw sockets of any address family and protocol.
 * 对于网络原始套接字类，包含了必要的flags
 *
 * @par Examples
 * Constructing using a native address family and socket protocol:
 * @code raw_protocol p(AF_INET, IPPROTO_ICMP); @endcode
 * Constructing from a specific protocol type:
 * @code raw_protocol p(asio::ip::icmp::v4()); @endcode
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol.
 */
class raw_protocol
{
public:
  /// Construct a protocol object for a specific address family and protocol.【构造函数得传入 地址族和套接字协议】
  raw_protocol(int address_family, int socket_protocol)
    : family_(address_family),
      protocol_(socket_protocol)
  {
  }

  /// Construct a generic protocol object from a specific protocol. 【拷贝构造函数】
  /**
   * @throws @c bad_cast Thrown if the source protocol is not raw-oriented.
   */
  template <typename Protocol>
  raw_protocol(const Protocol& source_protocol)
    : family_(source_protocol.family()),
      protocol_(source_protocol.protocol())
  {
    if (source_protocol.type() != type())   //要保证套接字协议相同[tcp <> udp]
    {
      std::bad_cast ex;
      asio::detail::throw_exception(ex);
    }
  }

  /// Obtain an identifier for the type of the protocol. 【套接字类型就是原始套接字SOCK_RAW】
  int type() const ASIO_NOEXCEPT
  {
    return ASIO_OS_DEF(SOCK_RAW);
  }

  /// Obtain an identifier for the protocol.    【协议IPPROTO_TCP/IPPROTO_UDP】
  int protocol() const ASIO_NOEXCEPT
  {
    return protocol_;
  }

  /// Obtain an identifier for the protocol family. 【地址族】
  int family() const ASIO_NOEXCEPT
  {
    return family_;
  }

  /// Compare two protocols for equality.
  friend bool operator==(const raw_protocol& p1, const raw_protocol& p2)
  {
    return p1.family_ == p2.family_ && p1.protocol_ == p2.protocol_;
  }

  /// Compare two protocols for inequality.
  friend bool operator!=(const raw_protocol& p1, const raw_protocol& p2)
  {
    return !(p1 == p2);
  }

  /// The type of an endpoint. 【特例化端点类模板】
  typedef basic_endpoint<raw_protocol> endpoint;

  /// The generic socket type.  【特例化基本原始套接字类模板】
  typedef basic_raw_socket<raw_protocol> socket;

private:
  int family_;  //地址族
  int protocol_;    //套接字协议
};

} // namespace generic
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_GENERIC_RAW_PROTOCOL_HPP
