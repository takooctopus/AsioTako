//
// TAKO�������˵�
// ip/detail/endpoint.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_DETAIL_ENDPOINT_HPP
#define ASIO_IP_DETAIL_ENDPOINT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <string>
#include "asio/detail/socket_types.hpp"
#include "asio/detail/winsock_init.hpp"
#include "asio/error_code.hpp"
#include "asio/ip/address.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {
namespace detail {

// Helper class for implementating an IP endpoint.���˵�����ࡿ
class endpoint
{
public:
  // Default constructor. ��Ĭ�Ϲ��캯����
  ASIO_DECL endpoint() ASIO_NOEXCEPT;

  // Construct an endpoint using a family and port number. ��ʹ�õ�ַ��Ͷ˿�[u16=>max65535]�����г�ʼ����
  ASIO_DECL endpoint(int family,
      unsigned short port_num) ASIO_NOEXCEPT;

  // Construct an endpoint using an address and port number. ��ʹ�õ�ַ�Ͷ˿�[u16=>max65535]�����г�ʼ����
  ASIO_DECL endpoint(const asio::ip::address& addr,
      unsigned short port_num) ASIO_NOEXCEPT;

  // Copy constructor.���������졿
  endpoint(const endpoint& other) ASIO_NOEXCEPT
    : data_(other.data_)
  {
  }

  // Assign from another endpoint.����ֵ���㡿
  endpoint& operator=(const endpoint& other) ASIO_NOEXCEPT
  {
    data_ = other.data_;
    return *this;
  }

  // Get the underlying endpoint in the native type.����ȡsock_addr�ĵ�ַָ��[ʹ�õ�������]��
  asio::detail::socket_addr_type* data() ASIO_NOEXCEPT
  {
    return &data_.base;
  }

  // Get the underlying endpoint in the native type. ����ȡsock_addr�ĵ�ַ��ָ�롿
  const asio::detail::socket_addr_type* data() const ASIO_NOEXCEPT
  {
    return &data_.base;
  }

  // Get the underlying size of the endpoint in the native type. ��v4=>16, v6=>28��
  std::size_t size() const ASIO_NOEXCEPT
  {
    if (is_v4())
      return sizeof(asio::detail::sockaddr_in4_type);
    else
      return sizeof(asio::detail::sockaddr_in6_type);
  }

  // Set the underlying size of the endpoint in the native type. �����ö˿ڵĴ�С[����˵ֻ����16/28]��
  ASIO_DECL void resize(std::size_t new_size);

  // Get the capacity of the endpoint in the native type. ����ȡdata_�������С=>28��
  std::size_t capacity() const ASIO_NOEXCEPT
  {
    return sizeof(data_);
  }

  // Get the port associated with the endpoint. ����ȡ�˵�ӵ�еĶ˿ڡ�
  ASIO_DECL unsigned short port() const ASIO_NOEXCEPT;

  // Set the port associated with the endpoint. �����ö˿ڡ�
  ASIO_DECL void port(unsigned short port_num) ASIO_NOEXCEPT;

  // Get the IP address associated with the endpoint. ����ȡ�˵��ip��
  ASIO_DECL asio::ip::address address() const ASIO_NOEXCEPT;

  // Set the IP address associated with the endpoint. �����ö˵��ip��
  ASIO_DECL void address(
      const asio::ip::address& addr) ASIO_NOEXCEPT;

  // Compare two endpoints for equality. ���Ƚ������˵�Ĵ�С��
  ASIO_DECL friend bool operator==(const endpoint& e1,
      const endpoint& e2) ASIO_NOEXCEPT;

  // Compare endpoints for ordering. ���Ƚ�˳��
  ASIO_DECL friend bool operator<(const endpoint& e1,
      const endpoint& e2) ASIO_NOEXCEPT;

  // Determine whether the endpoint is IPv4. ���ж��ǲ���ipv4 => ȡdata_=>����Ǹ������壬����ֱ��ȡbase=>�����sockaddr=>��ȡsa_family��
  bool is_v4() const ASIO_NOEXCEPT
  {
    return data_.base.sa_family == ASIO_OS_DEF(AF_INET);
  }

#if !defined(ASIO_NO_IOSTREAM)
  // Convert to a string.   ��ת����һ��string��
  ASIO_DECL std::string to_string() const;
#endif // !defined(ASIO_NO_IOSTREAM)

private:
  // The underlying IP socket address.
  union data_union
  {
    asio::detail::socket_addr_type base; //�����ṹ16byte
    asio::detail::sockaddr_in4_type v4; // v4�ṹ��sin_family��sin_port, sin_addr, sin_zero => u16, u16, u32, u8*8 => 16��
    asio::detail::sockaddr_in6_type v6; // v6�ṹ��sin6_family, sin6_port, sin6_flowinfo, sin6_addr, union[sin6_scope_id, sin6_scope_struct] => u16, u16, u32, u8*16, u32[ע��������windows���涨����ʹ�õ�ULONG����Ȼ������64λ���������unsigned long��ʵ��32bit��] => һ��28byte��
  } data_; //���ݽṹ���������塾16/16/28��
};

} // namespace detail
} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "asio/ip/detail/impl/endpoint.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_IP_DETAIL_ENDPOINT_HPP
