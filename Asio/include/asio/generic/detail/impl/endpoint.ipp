//
// generic/detail/impl/endpoint.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_GENERIC_DETAIL_IMPL_ENDPOINT_IPP
#define ASIO_GENERIC_DETAIL_IMPL_ENDPOINT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#include <cstring>
#include <typeinfo>
#include "asio/detail/socket_ops.hpp"
#include "asio/detail/throw_error.hpp"
#include "asio/detail/throw_exception.hpp"
#include "asio/error.hpp"
#include "asio/generic/detail/endpoint.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace generic {
namespace detail {

endpoint::endpoint()
{
  init(0, 0, 0);    //清零
}

endpoint::endpoint(const void* sock_addr,
    std::size_t sock_addr_size, int sock_protocol)
{
  init(sock_addr, sock_addr_size, sock_protocol);
}

void endpoint::resize(std::size_t new_size)
{
  if (new_size > sizeof(asio::detail::sockaddr_storage_type)) //大于128byte
  {
    asio::error_code ec(asio::error::invalid_argument);
    asio::detail::throw_error(ec);  //抛异常
  }
  else
  {
    size_ = new_size;
    protocol_ = 0;
  }
}

bool operator==(const endpoint& e1, const endpoint& e2)
{
  using namespace std; // For memcmp.
  return e1.size() == e2.size() && memcmp(e1.data(), e2.data(), e1.size()) == 0; //这种直接使用memcmp进行比较
}

bool operator<(const endpoint& e1, const endpoint& e2)
{
  if (e1.protocol() < e2.protocol())    //先比较协议
    return true;

  if (e1.protocol() > e2.protocol())
    return false;

  using namespace std; // For memcmp.
  std::size_t compare_size = e1.size() < e2.size() ? e1.size() : e2.size(); //再比较大小 v4的肯定比v6的小
  int compare_result = memcmp(e1.data(), e2.data(), compare_size);  //再直接比较内存数据

  if (compare_result < 0)
    return true;

  if (compare_result > 0)
    return false;

  return e1.size() < e2.size();
}

void endpoint::init(const void* sock_addr,
    std::size_t sock_addr_size, int sock_protocol)
{
  if (sock_addr_size > sizeof(asio::detail::sockaddr_storage_type)) //大小不能大于128byte，要不就超限了
  {
    asio::error_code ec(asio::error::invalid_argument);
    asio::detail::throw_error(ec);
  }

  using namespace std; // For memset and memcpy.
  memset(&data_.generic, 0, sizeof(asio::detail::sockaddr_storage_type));   //将地址上数据置0
  if (sock_addr_size > 0)
    memcpy(&data_.generic, sock_addr, sock_addr_size); //将对应的拷贝过去

  size_ = sock_addr_size;   //记录下sockaddr_in/sockaddr_in6的大小
  protocol_ = sock_protocol;    //记录下协议
}

} // namespace detail
} // namespace generic
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_GENERIC_DETAIL_IMPL_ENDPOINT_IPP
