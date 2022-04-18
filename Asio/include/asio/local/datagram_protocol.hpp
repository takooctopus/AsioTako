//
// TAKO:�������ݱ�Э��
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

#include "asio/basic_datagram_socket.hpp"   //�������ݱ�ģ��
#include "asio/detail/socket_types.hpp"     //socket��س���
#include "asio/local/basic_endpoint.hpp"    //�����˵�ģ��

#include "asio/detail/push_options.hpp"

namespace asio {
namespace local {

/// Encapsulates the flags needed for datagram-oriented UNIX sockets.
/**
 * The asio::local::datagram_protocol class contains flags necessary for
 * datagram-oriented UNIX domain sockets.
 * �������ݱ�����
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
  /// Obtain an identifier for the type of the protocol. ������SOCK_DGRAM��
  int type() const ASIO_NOEXCEPT
  {
    return SOCK_DGRAM;
  }

  /// Obtain an identifier for the protocol. ��Э�鱾�� =>0��
  int protocol() const ASIO_NOEXCEPT
  {
    return 0;
  }

  /// Obtain an identifier for the protocol family. ����ַ�� AF_UNIX��
  int family() const ASIO_NOEXCEPT
  {
    return AF_UNIX;
  }

  /// The type of a UNIX domain endpoint.  ���Ӷ˵�ģ�嶨�屾�����ݱ��˵㡿
  typedef basic_endpoint<datagram_protocol> endpoint;

  /// The UNIX domain socket type. ���������ݱ�socketģ�嶨�屾�����ݱ�socket��
  typedef basic_datagram_socket<datagram_protocol> socket;
};

} // namespace local
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_LOCAL_SOCKETS)
       //   || defined(GENERATING_DOCUMENTATION)

#endif // ASIO_LOCAL_DATAGRAM_PROTOCOL_HPP
