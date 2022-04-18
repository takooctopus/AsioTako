//
// TAKO:SSL错误相关函数
// ssl/impl/error.ipp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_SSL_IMPL_ERROR_IPP
#define ASIO_SSL_IMPL_ERROR_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/ssl/error.hpp"
#include "asio/ssl/detail/openssl_init.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace error {
namespace detail {

class ssl_category : public asio::error_category    //【错误类继承了error_category，我们先看std::error_category】
{
public:
  const char* name() const ASIO_ERROR_CATEGORY_NOEXCEPT //错误名称
  {
    return "asio.ssl";
  } 

  std::string message(int value) const
  {
    const char* reason = ::ERR_reason_error_string(value);  //通过ssl库的函数获取错误原因
    if (reason) //如果是ssl能发现错误
    {
      const char* lib = ::ERR_lib_error_string(value);  //通过ssl库的函数获取出错库
#if (OPENSSL_VERSION_NUMBER < 0x30000000L)
      const char* func = ::ERR_func_error_string(value);
#else // (OPENSSL_VERSION_NUMBER < 0x30000000L)
      const char* func = 0;
#endif // (OPENSSL_VERSION_NUMBER < 0x30000000L)
      std::string result(reason);   // 将错误原因转化成string
      if (lib || func)  //当版本openssl版本过低【要去格式化】
      {
        result += " (";
        if (lib)
          result += lib;
        if (lib && func)
          result += ", ";
        if (func)
          result += func;
        result += ")";
      }
      return result;    //能通过ssl内置错误函数获取错误就返回内置的错误
    }
    return "asio.ssl error";    //要不然就只给一个笼统的错误了
  }
};

} // namespace detail

const asio::error_category& get_ssl_category()
{
  static detail::ssl_category instance; //生成一个static的错误对象
  return instance;
}

} // namespace error
namespace ssl {
namespace error {

#if (OPENSSL_VERSION_NUMBER < 0x10100000L) && !defined(OPENSSL_IS_BORINGSSL)

const asio::error_category& get_stream_category()
{
  return asio::error::get_ssl_category();
}

#else

namespace detail {

class stream_category : public asio::error_category
{
public:
  const char* name() const ASIO_ERROR_CATEGORY_NOEXCEPT
  {
    return "asio.ssl.stream";   //错误名：写的是出错位置
  }

  std::string message(int value) const
  {
    switch (value)  //根据错误value返回出错原因的字符串
    {
    case stream_truncated: return "stream truncated";
    case unspecified_system_error: return "unspecified system error";
    case unexpected_result: return "unexpected result";
    default: return "asio.ssl.stream error";
    }
  }
};

} // namespace detail

const asio::error_category& get_stream_category()
{
  static detail::stream_category instance;  //一样的，static对象
  return instance;
}

#endif

} // namespace error
} // namespace ssl
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_SSL_IMPL_ERROR_IPP
