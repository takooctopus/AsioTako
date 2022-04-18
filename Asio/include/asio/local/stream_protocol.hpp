//
// TAKO: ������Э��
// local/stream_protocol.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_LOCAL_STREAM_PROTOCOL_HPP
#define ASIO_LOCAL_STREAM_PROTOCOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_LOCAL_SOCKETS) \
  || defined(GENERATING_DOCUMENTATION)

#include "asio/basic_socket_acceptor.hpp"
#include "asio/basic_socket_iostream.hpp"
#include "asio/basic_stream_socket.hpp"
#include "asio/detail/socket_types.hpp"
#include "asio/local/basic_endpoint.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace local {

/// Encapsulates the flags needed for stream-oriented UNIX sockets.
/**
 * The asio::local::stream_protocol class contains flags necessary for
 * stream-oriented UNIX domain sockets.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol.
 */
class stream_protocol
{
public:
  /// Obtain an identifier for the type of the protocol. ��������Э��=>����SOCK_STREAM��
  int type() const ASIO_NOEXCEPT
  {
    return SOCK_STREAM; //������ô����ASIO_OS_DEF(SOCK_STREAM)�ˣ���ʼ���հ�
  }

  /// Obtain an identifier for the protocol. ������Э��ı�ʶ��
  int protocol() const ASIO_NOEXCEPT
  {
    return 0;
  }

  /// Obtain an identifier for the protocol family. �����ص�ַ��=>AF_UNIX��
  int family() const ASIO_NOEXCEPT
  {
    return AF_UNIX;
  }

  /// The type of a UNIX domain endpoint. ����������endpoint => ʹ��basic_endpoint<>ģ�塿
  typedef basic_endpoint<stream_protocol> endpoint; 

  /// The UNIX domain socket type. ����������socket => ʹ��basic_stream_socket<>ģ�塿
  typedef basic_stream_socket<stream_protocol> socket;

  /// The UNIX domain acceptor type. ����������acceptor => ʹ��basic_socket_acceptor<>ģ�塿
  typedef basic_socket_acceptor<stream_protocol> acceptor;

#if !defined(ASIO_NO_IOSTREAM)
  /// The UNIX domain iostream type. ����������iostream => ʹ��basic_socket_iostream<>ģ�塿
  typedef basic_socket_iostream<stream_protocol> iostream;
#endif // !defined(ASIO_NO_IOSTREAM)
};

} // namespace local
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_LOCAL_SOCKETS)
       //   || defined(GENERATING_DOCUMENTATION)

#endif // ASIO_LOCAL_STREAM_PROTOCOL_HPP
