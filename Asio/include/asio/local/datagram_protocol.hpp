//
// TAKO:本地数据报协议
// local/datagram_protocol.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_LOCAL_DATAGRAM_PROTOCOL_HPP
#define ASIO_LOCAL_DATAGRAM_PROTOCOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_LOCAL_SOCKETS) \
  || defined(GENERATING_DOCUMENTATION)

#include "asio/basic_datagram_socket.hpp"   //基本数据报模板
#include "asio/detail/socket_types.hpp"     //socket相关常量
#include "asio/local/basic_endpoint.hpp"    //基本端点模板

#include "asio/detail/push_options.hpp"

namespace asio {
namespace local {

/// Encapsulates the flags needed for datagram-oriented UNIX sockets.
/**
 * The asio::local::datagram_protocol class contains flags necessary for
 * datagram-oriented UNIX domain sockets.
 * 本地数据报类型
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
  /// Obtain an identifier for the type of the protocol. 【类型SOCK_DGRAM】
  int type() const ASIO_NOEXCEPT
  {
    return SOCK_DGRAM;
  }

  /// Obtain an identifier for the protocol. 【协议本地 =>0】
  int protocol() const ASIO_NOEXCEPT
  {
    return 0;
  }

  /// Obtain an identifier for the protocol family. 【地址族 AF_UNIX】
  int family() const ASIO_NOEXCEPT
  {
    return AF_UNIX;
  }

  /// The type of a UNIX domain endpoint.  【从端点模板定义本地数据报端点】
  typedef basic_endpoint<datagram_protocol> endpoint;

  /// The UNIX domain socket type. 【根据数据报socket模板定义本地数据报socket】
  typedef basic_datagram_socket<datagram_protocol> socket;
};

} // namespace local
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_LOCAL_SOCKETS)
       //   || defined(GENERATING_DOCUMENTATION)

#endif // ASIO_LOCAL_DATAGRAM_PROTOCOL_HPP
