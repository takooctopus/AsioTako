//
// TAKO: 端点的实现
// ip/detail/impl/endpoint.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_DETAIL_IMPL_ENDPOINT_IPP
#define ASIO_IP_DETAIL_IMPL_ENDPOINT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <cstring>
#if !defined(ASIO_NO_IOSTREAM)
# include <sstream>
#endif // !defined(ASIO_NO_IOSTREAM)
#include "asio/detail/socket_ops.hpp"
#include "asio/detail/throw_error.hpp"
#include "asio/error.hpp"
#include "asio/ip/detail/endpoint.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {
namespace detail {

endpoint::endpoint() ASIO_NOEXCEPT
  : data_()
{
  data_.v4.sin_family = ASIO_OS_DEF(AF_INET);   //默认构造设置成ipv4
  data_.v4.sin_port = 0;
  data_.v4.sin_addr.s_addr = ASIO_OS_DEF(INADDR_ANY);
}

endpoint::endpoint(int family, unsigned short port_num) ASIO_NOEXCEPT
  : data_()
{
  using namespace std; // For memcpy.
  if (family == ASIO_OS_DEF(AF_INET))
  {
    data_.v4.sin_family = ASIO_OS_DEF(AF_INET); //要是ipv4设置为v4.sin_family => AF_INET
    data_.v4.sin_port =
      asio::detail::socket_ops::host_to_network_short(port_num);    //设置v4.sin_port => port_num[记得换成网络顺序]
    data_.v4.sin_addr.s_addr = ASIO_OS_DEF(INADDR_ANY); //没传地址，地址默认为0.0.0.0
  }
  else
  {
    data_.v6.sin6_family = ASIO_OS_DEF(AF_INET6);   //要是ipv6设置为v6.sin6_family => AF_INET6
    data_.v6.sin6_port =
      asio::detail::socket_ops::host_to_network_short(port_num);    //设置v6.sin6_port => port_num[记得换成网络字节序]
    data_.v6.sin6_flowinfo = 0; // 流信息设置为0
    data_.v6.sin6_addr.s6_addr[0] = 0; data_.v6.sin6_addr.s6_addr[1] = 0;
    data_.v6.sin6_addr.s6_addr[2] = 0; data_.v6.sin6_addr.s6_addr[3] = 0;
    data_.v6.sin6_addr.s6_addr[4] = 0; data_.v6.sin6_addr.s6_addr[5] = 0;
    data_.v6.sin6_addr.s6_addr[6] = 0; data_.v6.sin6_addr.s6_addr[7] = 0;
    data_.v6.sin6_addr.s6_addr[8] = 0; data_.v6.sin6_addr.s6_addr[9] = 0;
    data_.v6.sin6_addr.s6_addr[10] = 0; data_.v6.sin6_addr.s6_addr[11] = 0;
    data_.v6.sin6_addr.s6_addr[12] = 0; data_.v6.sin6_addr.s6_addr[13] = 0;
    data_.v6.sin6_addr.s6_addr[14] = 0; data_.v6.sin6_addr.s6_addr[15] = 0; //没传地址进来，设置为::
    data_.v6.sin6_scope_id = 0; //没传地址进来，scope_id设置为0
  }
}

endpoint::endpoint(const asio::ip::address& addr,
    unsigned short port_num) ASIO_NOEXCEPT
  : data_()
{
  using namespace std; // For memcpy.
  if (addr.is_v4())
  {
    data_.v4.sin_family = ASIO_OS_DEF(AF_INET);
    data_.v4.sin_port =
      asio::detail::socket_ops::host_to_network_short(port_num);
    data_.v4.sin_addr.s_addr =
      asio::detail::socket_ops::host_to_network_long(
        addr.to_v4().to_uint());    //传了地址进来，先转成uint，再转换成网络字节序
  }
  else
  {
    data_.v6.sin6_family = ASIO_OS_DEF(AF_INET6);
    data_.v6.sin6_port =
      asio::detail::socket_ops::host_to_network_short(port_num);
    data_.v6.sin6_flowinfo = 0;
    asio::ip::address_v6 v6_addr = addr.to_v6();    //先转成address_v6的对象
    asio::ip::address_v6::bytes_type bytes = v6_addr.to_bytes();    //转换成u8*16数组
    memcpy(data_.v6.sin6_addr.s6_addr, bytes.data(), 16);   //直接memcpy拷贝【本身顺序就是网络字节序了】
    data_.v6.sin6_scope_id =
      static_cast<asio::detail::u_long_type>(
        v6_addr.scope_id());
  }
}

void endpoint::resize(std::size_t new_size) //设置端点大小【其实没啥用】
{
  if (new_size > sizeof(asio::detail::sockaddr_storage_type))   //艹了，安慰剂是吧，这里面就一个长度异常判断？
  {
    asio::error_code ec(asio::error::invalid_argument);
    asio::detail::throw_error(ec);
  }
}

unsigned short endpoint::port() const ASIO_NOEXCEPT
{
  if (is_v4())
  {
    return asio::detail::socket_ops::network_to_host_short(
        data_.v4.sin_port); //【获取端点[得转换成主机字节序]】
  }
  else
  {
    return asio::detail::socket_ops::network_to_host_short(
        data_.v6.sin6_port);
  }
}

void endpoint::port(unsigned short port_num) ASIO_NOEXCEPT
{
  if (is_v4())  
  {
    data_.v4.sin_port
      = asio::detail::socket_ops::host_to_network_short(port_num); //【设置端口[得转换成网络字节序]】
  }
  else
  {
    data_.v6.sin6_port
      = asio::detail::socket_ops::host_to_network_short(port_num);
  }
}

asio::ip::address endpoint::address() const ASIO_NOEXCEPT
{
  using namespace std; // For memcpy.
  if (is_v4())
  {
    return asio::ip::address_v4(
        asio::detail::socket_ops::network_to_host_long(
          data_.v4.sin_addr.s_addr));   //【新建一个address_v4对象，用的自动对象生成=>转化成address对象返回去】
  }
  else
  {
    asio::ip::address_v6::bytes_type bytes;
#if defined(ASIO_HAS_STD_ARRAY)
    memcpy(bytes.data(), data_.v6.sin6_addr.s6_addr, 16);   //标准std使用的array操作是data()获取首地址的
#else // defined(ASIO_HAS_STD_ARRAY)
    memcpy(bytes.elems, data_.v6.sin6_addr.s6_addr, 16);
#endif // defined(ASIO_HAS_STD_ARRAY)
    return asio::ip::address_v6(bytes, data_.v6.sin6_scope_id);
  }
}

void endpoint::address(const asio::ip::address& addr) ASIO_NOEXCEPT
{
  endpoint tmp_endpoint(addr, port());  //设置端点的ip，新建一个临时对象[使用传入的地址和现在的端口]
  data_ = tmp_endpoint.data_;   //将前面赋值就好
}

bool operator==(const endpoint& e1, const endpoint& e2) ASIO_NOEXCEPT
{
  return e1.address() == e2.address() && e1.port() == e2.port();    //判断相等，地址和端口都得相等
}

bool operator<(const endpoint& e1, const endpoint& e2) ASIO_NOEXCEPT
{
  if (e1.address() < e2.address())  
    return true;
  if (e1.address() != e2.address())
    return false;
  return e1.port() < e2.port(); //地址相等还得判断端口
}

#if !defined(ASIO_NO_IOSTREAM)
std::string endpoint::to_string() const
{
  std::ostringstream tmp_os;    //使用ostringstream进行格式化
  // 之所以可以直接用是因为我们重写了地址类的<<操作
  tmp_os.imbue(std::locale::classic());
  if (is_v4())
    tmp_os << address();
  else
    tmp_os << '[' << address() << ']';
  tmp_os << ':' << port();  

  return tmp_os.str();  //返回str
}
#endif // !defined(ASIO_NO_IOSTREAM)

} // namespace detail
} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_DETAIL_IMPL_ENDPOINT_IPP
