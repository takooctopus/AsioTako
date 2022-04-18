//
// TAKO:IP端点
// local/detail/impl/endpoint.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Derived from a public domain implementation written by Daniel Casimiro.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_LOCAL_DETAIL_IMPL_ENDPOINT_IPP
#define ASIO_LOCAL_DETAIL_IMPL_ENDPOINT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_LOCAL_SOCKETS)

#include <cstring>
#include "asio/detail/socket_ops.hpp"
#include "asio/detail/throw_error.hpp"
#include "asio/error.hpp"
#include "asio/local/detail/endpoint.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace local {
namespace detail {

endpoint::endpoint()
{
  init("", 0); //默认构造函数就传一个空字符串进去
}

endpoint::endpoint(const char* path_name)
{
  using namespace std; // For strlen.
  init(path_name, strlen(path_name));
}

endpoint::endpoint(const std::string& path_name)
{
  init(path_name.data(), path_name.length());
}

#if defined(ASIO_HAS_STRING_VIEW)
endpoint::endpoint(string_view path_name)
{
  init(path_name.data(), path_name.length());
}
#endif // defined(ASIO_HAS_STRING_VIEW)

void endpoint::resize(std::size_t new_size)
{
    //变换大小别说你大小都超出110byte了【本地作用域】
  if (new_size > sizeof(asio::detail::sockaddr_un_type))
  {
    asio::error_code ec(asio::error::invalid_argument);
    asio::detail::throw_error(ec);
  }
  else if (new_size == 0)
  {
      //或者把地址全部变成0
    path_length_ = 0;
  }
  else
  {
    path_length_ = new_size
      - offsetof(asio::detail::sockaddr_un_type, sun_path); //减去前面的sun_family[ushort即2byte]

    // The path returned by the operating system may be NUL-terminated. 操作系统返回的可能会有NUL['\0']这个时候我们算长度要-1
    if (path_length_ > 0 && data_.local.sun_path[path_length_ - 1] == 0)
      --path_length_;
  }
}

std::string endpoint::path() const
{
  return std::string(data_.local.sun_path, path_length_);   //生成一个string使用特定的构造函数返回string
}

void endpoint::path(const char* p)
{
  using namespace std; // For strlen.
  init(p, strlen(p));   //传入path进行初始化
}

void endpoint::path(const std::string& p)
{
  init(p.data(), p.length());   //传入path，进行初始化
}

bool operator==(const endpoint& e1, const endpoint& e2)
{
  return e1.path() == e2.path(); //只比较path
}

bool operator<(const endpoint& e1, const endpoint& e2)
{
  return e1.path() < e2.path(); //只比较path
}

void endpoint::init(const char* path_name, std::size_t path_length)
{
    //path长度超过>107时，sockaddr_un_type结构的缓冲区就放不下了，报错吧
  if (path_length > sizeof(data_.local.sun_path) - 1)
  {
    // The buffer is not large enough to store this address.
    asio::error_code ec(asio::error::name_too_long);
    asio::detail::throw_error(ec);
  }

  using namespace std; // For memset and memcpy.
  memset(&data_.local, 0, sizeof(asio::detail::sockaddr_un_type));  //将整个的data_一共110byte全部清零
  data_.local.sun_family = AF_UNIX; //设置协议[本地]
  if (path_length > 0)
    memcpy(data_.local.sun_path, path_name, path_length);   //将path中的信息拷贝过去
  path_length_ = path_length;  
}

} // namespace detail
} // namespace local
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_LOCAL_SOCKETS)

#endif // ASIO_LOCAL_DETAIL_IMPL_ENDPOINT_IPP
