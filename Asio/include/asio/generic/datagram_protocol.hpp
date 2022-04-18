//
// TAKO: 【模板实现】网络数据报协议
// generic/datagram_protocol.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_GENERIC_DATAGRAM_PROTOCOL_HPP
#define ASIO_GENERIC_DATAGRAM_PROTOCOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#include <typeinfo>
#include "asio/basic_datagram_socket.hpp"
#include "asio/detail/socket_types.hpp"
#include "asio/detail/throw_exception.hpp"
#include "asio/generic/basic_endpoint.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace generic {

/// Encapsulates the flags needed for a generic datagram-oriented socket.
/**
 * The asio::generic::datagram_protocol class contains flags necessary
 * for datagram-oriented sockets of any address family and protocol.
 *
 * @par Examples
 * Constructing using a native address family and socket protocol:
 * @code datagram_protocol p(AF_INET, IPPROTO_UDP); @endcode
 * Constructing from a specific protocol type:
 * @code datagram_protocol p(asio::ip::udp::v4()); @endcode
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol.
 */
class datagram_protocol
{
public:
  /// Construct a protocol object for a specific address family and protocol. 【构造时需要传入 地址族[AF_INET...]，协议[IPPROTO_TCP/IPPROTO_UDP...]】
  datagram_protocol(int address_family, int socket_protocol)
    : family_(address_family),
      protocol_(socket_protocol)
  {
  }

  /// Construct a generic protocol object from a specific protocol. 
  /**
  * 拷贝构造函数【如果对应的协议类型不一样，抛异常啊[TCP 不能和 UDP互转]】
   * @throws @c bad_cast Thrown if the source protocol is not datagram-oriented.
   */
  template <typename Protocol>
  datagram_protocol(const Protocol& source_protocol)
    : family_(source_protocol.family()),
      protocol_(source_protocol.protocol())
  {
    if (source_protocol.type() != type())
    {
      std::bad_cast ex;
      asio::detail::throw_exception(ex);
    }
  }

  /// Obtain an identifier for the type of the protocol.【类型就是数据报SOCK_DGRAM】
  int type() const ASIO_NOEXCEPT
  {
    return ASIO_OS_DEF(SOCK_DGRAM);
  }

  /// Obtain an identifier for the protocol. 【协议看看吧 一般来说 IPPROTO_TCP/IPPROTO_UDP】
  int protocol() const ASIO_NOEXCEPT
  {
    return protocol_;
  }

  /// Obtain an identifier for the protocol family. 【地址族 AF_INET/AF_INET6】
  int family() const ASIO_NOEXCEPT
  {
    return family_;
  }

  /// Compare two protocols for equality.   【协议是不是一样的，地址族是不是一样的】
  friend bool operator==(const datagram_protocol& p1,
      const datagram_protocol& p2)
  {
    return p1.family_ == p2.family_ && p1.protocol_ == p2.protocol_;
  }

  /// Compare two protocols for inequality. 
  friend bool operator!=(const datagram_protocol& p1,
      const datagram_protocol& p2)
  {
    return !(p1 == p2);
  }

  /// The type of an endpoint. 【特例化basic_endpoint<>模板】
  typedef basic_endpoint<datagram_protocol> endpoint;

  /// The generic socket type. 【特例化basic_datagram_socket<>模板】
  typedef basic_datagram_socket<datagram_protocol> socket;

private:
  int family_;  //地址族
  int protocol_;    //协议
};

} // namespace generic
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_GENERIC_DATAGRAM_PROTOCOL_HPP
