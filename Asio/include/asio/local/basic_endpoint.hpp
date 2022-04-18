//
// TAKO:基本的端点信息
// local/basic_endpoint.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Derived from a public domain implementation written by Daniel Casimiro.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_LOCAL_BASIC_ENDPOINT_HPP
#define ASIO_LOCAL_BASIC_ENDPOINT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_LOCAL_SOCKETS) \
  || defined(GENERATING_DOCUMENTATION)

#include "asio/local/detail/endpoint.hpp"

#if !defined(ASIO_NO_IOSTREAM)
# include <iosfwd>
#endif // !defined(ASIO_NO_IOSTREAM)

#include "asio/detail/push_options.hpp"

namespace asio {
namespace local {

/// Describes an endpoint for a UNIX socket.
/**
 * The asio::local::basic_endpoint class template describes an endpoint
 * that may be associated with a particular UNIX socket.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 *
 * @par Concepts:
 * Endpoint.
 */
template <typename Protocol>
class basic_endpoint
{
public:
  /// The protocol type associated with the endpoint.
  typedef Protocol protocol_type;

  /// The type of the endpoint structure. This type is dependent on the
  /// underlying implementation of the socket layer.
#if defined(GENERATING_DOCUMENTATION)
  typedef implementation_defined data_type;
#else
  typedef asio::detail::socket_addr_type data_type; // 【定义sockaddr 16byte】
#endif

  /// Default constructor.【默认构造函数，里面的impl_也调用默认构造函数=>impl_("")】
  basic_endpoint() ASIO_NOEXCEPT
  { 
  }

  /// Construct an endpoint using the specified path name. 【传入const char*】
  basic_endpoint(const char* path_name)
    : impl_(path_name)
  {
  }

  /// Construct an endpoint using the specified path name. 【传入const string& ==> 虽然传入的时string，但最下面还是用的s.data(),s.length()去初始化的】
  basic_endpoint(const std::string& path_name)
    : impl_(path_name)
  {
  }

  #if defined(ASIO_HAS_STRING_VIEW)
  /// Construct an endpoint using the specified path name. 【用stringview】
  basic_endpoint(string_view path_name)
    : impl_(path_name)
  {
  }
  #endif // defined(ASIO_HAS_STRING_VIEW)

  /// Copy constructor. 【拷贝构造】
  basic_endpoint(const basic_endpoint& other)
    : impl_(other.impl_)
  {
  }

#if defined(ASIO_HAS_MOVE)
  /// Move constructor. 【移动构造[其实下面impl_中没有实现移动构造还行]】
  basic_endpoint(basic_endpoint&& other)
    : impl_(other.impl_)
  {
  }
#endif // defined(ASIO_HAS_MOVE)

  /// Assign from another endpoint. 【赋值】
  basic_endpoint& operator=(const basic_endpoint& other)
  {
    impl_ = other.impl_;
    return *this;
  }

#if defined(ASIO_HAS_MOVE)
  /// Move-assign from another endpoint.【移动赋值【还是一样的，主要是impl_的类并没有实现移动构造函数]】
  basic_endpoint& operator=(basic_endpoint&& other)
  {
    impl_ = other.impl_;
    return *this;
  }
#endif // defined(ASIO_HAS_MOVE)

  /// The protocol associated with the endpoint.
  protocol_type protocol() const 
  {
    return protocol_type(); //返回的是构建这个类时<>里面的类型
  }

  /// Get the underlying endpoint in the native type.
  data_type* data()
  {
    return impl_.data(); //data首地址指针
  }

  /// Get the underlying endpoint in the native type.
  const data_type* data() const
  {
    return impl_.data(); //data首地址指针
  }

  /// Get the underlying size of the endpoint in the native type. 【获取到从协议到path非0位置的总大小】
  std::size_t size() const
  {
    return impl_.size();
  }

  /// Set the underlying size of the endpoint in the native type.   【更改大小[不能超过110]】
  void resize(std::size_t new_size)
  {
    impl_.resize(new_size);
  }

  /// Get the capacity of the endpoint in the native type. 【110byte】
  std::size_t capacity() const
  {
    return impl_.capacity();
  }

  /// Get the path associated with the endpoint. 【获取里面的本地路径 string】
  std::string path() const
  {
    return impl_.path();
  }

  /// Set the path associated with the endpoint. 【设定文件路径】
  void path(const char* p)
  {
    impl_.path(p);
  }

  /// Set the path associated with the endpoint. 【设定文件路径】
  void path(const std::string& p)
  {
    impl_.path(p);
  }

  /// Compare two endpoints for equality.
  friend bool operator==(const basic_endpoint<Protocol>& e1,
      const basic_endpoint<Protocol>& e2)
  {
    return e1.impl_ == e2.impl_;
  }

  /// Compare two endpoints for inequality.
  friend bool operator!=(const basic_endpoint<Protocol>& e1,
      const basic_endpoint<Protocol>& e2)
  {
    return !(e1.impl_ == e2.impl_);
  }

  /// Compare endpoints for ordering.
  friend bool operator<(const basic_endpoint<Protocol>& e1,
      const basic_endpoint<Protocol>& e2)
  {
    return e1.impl_ < e2.impl_;
  }

  /// Compare endpoints for ordering.
  friend bool operator>(const basic_endpoint<Protocol>& e1,
      const basic_endpoint<Protocol>& e2)
  {
    return e2.impl_ < e1.impl_;
  }

  /// Compare endpoints for ordering.
  friend bool operator<=(const basic_endpoint<Protocol>& e1,
      const basic_endpoint<Protocol>& e2)
  {
    return !(e2 < e1);
  }

  /// Compare endpoints for ordering.
  friend bool operator>=(const basic_endpoint<Protocol>& e1,
      const basic_endpoint<Protocol>& e2)
  {
    return !(e1 < e2);
  }

private:
  // The underlying UNIX domain endpoint. 【保有下面一共底层的endpoint实例】
  asio::local::detail::endpoint impl_;  
};

/// Output an endpoint as a string.
/**
 * Used to output a human-readable string for a specified endpoint.
 *
 * @param os The output stream to which the string will be written.
 *
 * @param endpoint The endpoint to be written.
 *
 * @return The output stream.
 *
 * @relates asio::local::basic_endpoint
 */
template <typename Elem, typename Traits, typename Protocol>
std::basic_ostream<Elem, Traits>& operator<<(
    std::basic_ostream<Elem, Traits>& os,
    const basic_endpoint<Protocol>& endpoint)
{
  os << endpoint.path();
  return os;
}

} // namespace local
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_LOCAL_SOCKETS)
       //   || defined(GENERATING_DOCUMENTATION)

#endif // ASIO_LOCAL_BASIC_ENDPOINT_HPP
