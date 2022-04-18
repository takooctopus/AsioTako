//
// TAKO:openssl 的错误处理相关
// ssl/error.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_SSL_ERROR_HPP
#define ASIO_SSL_ERROR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/error_code.hpp"
#include "asio/ssl/detail/openssl_types.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace error {

enum ssl_errors
{
  // Error numbers are those produced by openssl.
};

extern ASIO_DECL
const asio::error_category& get_ssl_category(); //这个函数在asio::error

static const asio::error_category&
  ssl_category ASIO_UNUSED_VARIABLE
  = asio::error::get_ssl_category();

} // namespace error
namespace ssl {
namespace error {

enum stream_errors  //流错误
{
#if defined(GENERATING_DOCUMENTATION)
  /// The underlying stream closed before the ssl stream gracefully shut down.
  stream_truncated,

  /// The underlying SSL library returned a system error without providing
  /// further information.
  unspecified_system_error,

  /// The underlying SSL library generated an unexpected result from a function
  /// call.
  unexpected_result
#else // defined(GENERATING_DOCUMENTATION)
# if (OPENSSL_VERSION_NUMBER < 0x10100000L) \
    && !defined(OPENSSL_IS_BORINGSSL) \
    && !defined(ASIO_USE_WOLFSSL)
  stream_truncated = ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ),
# else
  stream_truncated = 1, //流被截断
# endif
  unspecified_system_error = 2, //未指定系统错误
  unexpected_result = 3 //未预期的错误
#endif // defined(GENERATING_DOCUMENTATION)
};

extern ASIO_DECL
const asio::error_category& get_stream_category();  //这个函数在asio::ssl::error里面

static const asio::error_category&
  stream_category ASIO_UNUSED_VARIABLE
  = asio::ssl::error::get_stream_category();

} // namespace error
} // namespace ssl
} // namespace asio

#if defined(ASIO_HAS_STD_SYSTEM_ERROR)
namespace std {

template<> struct is_error_code_enum<asio::error::ssl_errors>   //使用模板类，判断是不是error_code_enum
{
  static const bool value = true;
};

template<> struct is_error_code_enum<asio::ssl::error::stream_errors>   //使用模板特化类， 判断是不是error_code_enum
{
  static const bool value = true;
};

} // namespace std
#endif // defined(ASIO_HAS_STD_SYSTEM_ERROR)

namespace asio {
namespace error {

inline asio::error_code make_error_code(ssl_errors e)
{
  return asio::error_code(
      static_cast<int>(e), get_ssl_category()); //拿到这个error的static实例
  //对std::error_code的构造函数来说 (value, category) 用参数填充对应的字段
}

} // namespace error
namespace ssl {
namespace error {

inline asio::error_code make_error_code(stream_errors e)
{
  return asio::error_code(
      static_cast<int>(e), get_stream_category());  //创建错误码【错误类的定义在ipp里面】
}

} // namespace error
} // namespace ssl
} // namespace asio

#include "asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "asio/ssl/impl/error.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_SSL_ERROR_HPP
