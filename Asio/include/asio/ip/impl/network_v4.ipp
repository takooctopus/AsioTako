//
// TAKO:ipv4网络相关的类和方法
// ip/impl/network_v4.ipp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2014 Oliver Kowalke (oliver dot kowalke at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_IMPL_NETWORK_V4_IPP
#define ASIO_IP_IMPL_NETWORK_V4_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include "asio/error.hpp"
#include "asio/detail/throw_error.hpp"
#include "asio/detail/throw_exception.hpp"
#include "asio/ip/network_v4.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

network_v4::network_v4(const address_v4& addr, unsigned short prefix_len)
  : address_(addr),
    prefix_length_(prefix_len)
{
  if (prefix_len > 32)  //掩码不能超过32位
  {
    std::out_of_range ex("prefix length too large");
    asio::detail::throw_exception(ex);
  }
}

network_v4::network_v4(const address_v4& addr, const address_v4& mask)
  : address_(addr),
    prefix_length_(0)
{
  address_v4::bytes_type mask_bytes = mask.to_bytes(); //将mask转化成一个u8*4数组
  bool finished = false;
  for (std::size_t i = 0; i < mask_bytes.size(); ++i)
  {
    if (finished)
    {
      if (mask_bytes[i])
      {
        std::invalid_argument ex("non-contiguous netmask");
        asio::detail::throw_exception(ex);
      }
      continue;
    }
    else
    {
      switch (mask_bytes[i])
      {
      case 255:
        prefix_length_ += 8;
        break;
      case 254: // prefix_length_ += 7
        prefix_length_ += 1;
      case 252: // prefix_length_ += 6
        prefix_length_ += 1;
      case 248: // prefix_length_ += 5
        prefix_length_ += 1;
      case 240: // prefix_length_ += 4
        prefix_length_ += 1;
      case 224: // prefix_length_ += 3
        prefix_length_ += 1;
      case 192: // prefix_length_ += 2
        prefix_length_ += 1;
      case 128: // prefix_length_ += 1
        prefix_length_ += 1;
      case 0:   // nbits += 0
        finished = true;
        break;
      default:
        std::out_of_range ex("non-contiguous netmask");
        asio::detail::throw_exception(ex);
      }
    }
  }
}

address_v4 network_v4::netmask() const ASIO_NOEXCEPT
{
  uint32_t nmbits = 0xffffffff;
  if (prefix_length_ == 0)
    nmbits = 0;
  else
    nmbits = nmbits << (32 - prefix_length_); //向左移位主机号的长度
  return address_v4(nmbits);
}

address_v4_range network_v4::hosts() const ASIO_NOEXCEPT
{
  return is_host()
    ? address_v4_range(address_, address_v4(address_.to_uint() + 1)) //如果传入的是主机，那么range就只有1
    : address_v4_range(address_v4(network().to_uint() + 1), broadcast()); //否则主机号从000..01开始到111...11
}

bool network_v4::is_subnet_of(const network_v4& other) const 
{
  if (other.prefix_length_ >= prefix_length_)  //要是当前的网络号长度大于等于另外一个，那肯定不可能是子网啊
    return false; // Only real subsets are allowed.
  const network_v4 me(address_, other.prefix_length_);
  return other.canonical() == me.canonical();
}

std::string network_v4::to_string() const   //转化成string
{
  asio::error_code ec;
  std::string addr = to_string(ec);
  asio::detail::throw_error(ec);
  return addr;
}

std::string network_v4::to_string(asio::error_code& ec) const   //不抛异常的输出成string
{
  using namespace std; // For sprintf.
  ec = asio::error_code();
  char prefix_len[16];  //长度16的buffer用来装掩码长度
#if defined(ASIO_HAS_SECURE_RTL)
  sprintf_s(prefix_len, sizeof(prefix_len), "/%u", prefix_length_); //使用sprintf_s进行格式化
#else // defined(ASIO_HAS_SECURE_RTL)
  sprintf(prefix_len, "/%u", prefix_length_);
#endif // defined(ASIO_HAS_SECURE_RTL)
  return address_.to_string() + prefix_len;
}

network_v4 make_network_v4(const char* str)
{
  return make_network_v4(std::string(str)); //char* 转化成str去算
}

network_v4 make_network_v4(const char* str, asio::error_code& ec)
{
  return make_network_v4(std::string(str), ec);
}

network_v4 make_network_v4(const std::string& str)
{
  asio::error_code ec;
  network_v4 net = make_network_v4(str, ec);
  asio::detail::throw_error(ec);
  return net;
}

network_v4 make_network_v4(const std::string& str,
    asio::error_code& ec)
{
  std::string::size_type pos = str.find_first_of("/");  //找"/"

  if (pos == std::string::npos) //找不到，抛异常啊
  {
    ec = asio::error::invalid_argument;
    return network_v4();
  }

  if (pos == str.size() - 1)    // "/"后面没了？抛异常啊
  {
    ec = asio::error::invalid_argument;
    return network_v4();
  }

  std::string::size_type end = str.find_first_not_of("0123456789", pos + 1);    //后面找康康有没有不是数字的
  if (end != std::string::npos)
  {
    ec = asio::error::invalid_argument;
    return network_v4();
  }

  const address_v4 addr = make_address_v4(str.substr(0, pos), ec);  //用substr创建address_v4
  if (ec)
    return network_v4();

  const int prefix_len = std::atoi(str.substr(pos + 1).c_str());    //用atoi转换成int
  if (prefix_len < 0 || prefix_len > 32)
  {
    ec = asio::error::invalid_argument;
    return network_v4();
  }

  return network_v4(addr, static_cast<unsigned short>(prefix_len)); //记得转换成ushort
}

#if defined(ASIO_HAS_STRING_VIEW)

network_v4 make_network_v4(string_view str)
{
  return make_network_v4(static_cast<std::string>(str));
}

network_v4 make_network_v4(string_view str,
    asio::error_code& ec)
{
  return make_network_v4(static_cast<std::string>(str), ec);
}

#endif // defined(ASIO_HAS_STRING_VIEW)

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_IMPL_NETWORK_V4_IPP
