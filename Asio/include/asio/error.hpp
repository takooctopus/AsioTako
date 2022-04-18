//
// TAKO: 定义异常代码
// error.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_ERROR_HPP
#define ASIO_ERROR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/error_code.hpp"
#include "asio/system_error.hpp"
#if defined(ASIO_WINDOWS) \
  || defined(__CYGWIN__) \
  || defined(ASIO_WINDOWS_RUNTIME)
# include <winerror.h>
#else
# include <cerrno>
# include <netdb.h>
#endif

#if defined(GENERATING_DOCUMENTATION)
/// INTERNAL ONLY.
# define ASIO_NATIVE_ERROR(e) implementation_defined
/// INTERNAL ONLY.
# define ASIO_SOCKET_ERROR(e) implementation_defined
/// INTERNAL ONLY.
# define ASIO_NETDB_ERROR(e) implementation_defined
/// INTERNAL ONLY.
# define ASIO_GETADDRINFO_ERROR(e) implementation_defined
/// INTERNAL ONLY.
# define ASIO_WIN_OR_POSIX(e_win, e_posix) implementation_defined
#elif defined(ASIO_WINDOWS_RUNTIME)
# define ASIO_NATIVE_ERROR(e) __HRESULT_FROM_WIN32(e)
# define ASIO_SOCKET_ERROR(e) __HRESULT_FROM_WIN32(WSA ## e)
# define ASIO_NETDB_ERROR(e) __HRESULT_FROM_WIN32(WSA ## e)
# define ASIO_GETADDRINFO_ERROR(e) __HRESULT_FROM_WIN32(WSA ## e)
# define ASIO_WIN_OR_POSIX(e_win, e_posix) e_win
#elif defined(ASIO_WINDOWS) || defined(__CYGWIN__)
# define ASIO_NATIVE_ERROR(e) e
# define ASIO_SOCKET_ERROR(e) WSA ## e
# define ASIO_NETDB_ERROR(e) WSA ## e
# define ASIO_GETADDRINFO_ERROR(e) WSA ## e
# define ASIO_WIN_OR_POSIX(e_win, e_posix) e_win
#else
# define ASIO_NATIVE_ERROR(e) e
# define ASIO_SOCKET_ERROR(e) e
# define ASIO_NETDB_ERROR(e) e
# define ASIO_GETADDRINFO_ERROR(e) e
# define ASIO_WIN_OR_POSIX(e_win, e_posix) e_posix
#endif

#include "asio/detail/push_options.hpp"

namespace asio {
namespace error {

enum basic_errors
{
  /// Permission denied. 【拒绝访问】
  access_denied = ASIO_SOCKET_ERROR(EACCES),

  /// Address family not supported by protocol. 【地址族不支持该协议】
  address_family_not_supported = ASIO_SOCKET_ERROR(EAFNOSUPPORT),

  /// Address already in use. 【该地址已经被使用】
  address_in_use = ASIO_SOCKET_ERROR(EADDRINUSE),

  /// Transport endpoint is already connected. 【该端点已经被使用】
  already_connected = ASIO_SOCKET_ERROR(EISCONN),

  /// Operation already in progress. 【操作正在进行中】
  already_started = ASIO_SOCKET_ERROR(EALREADY),

  /// Broken pipe. 【管道损坏】
  broken_pipe = ASIO_WIN_OR_POSIX(
      ASIO_NATIVE_ERROR(ERROR_BROKEN_PIPE),
      ASIO_NATIVE_ERROR(EPIPE)),

  /// A connection has been aborted. 【连接异常退出】
  connection_aborted = ASIO_SOCKET_ERROR(ECONNABORTED),

  /// Connection refused. 【连接被拒绝】
  connection_refused = ASIO_SOCKET_ERROR(ECONNREFUSED),

  /// Connection reset by peer. 【连接被重置[peer]】
  connection_reset = ASIO_SOCKET_ERROR(ECONNRESET),

  /// Bad file descriptor. 【错误的文件描述符】
  bad_descriptor = ASIO_SOCKET_ERROR(EBADF),

  /// Bad address. 【错误的地址】
  fault = ASIO_SOCKET_ERROR(EFAULT),

  /// No route to host. 【没有到主机的路径[无法连接到主机]】
  host_unreachable = ASIO_SOCKET_ERROR(EHOSTUNREACH),

  /// Operation now in progress. 【操作正在进行中】
  in_progress = ASIO_SOCKET_ERROR(EINPROGRESS),

  /// Interrupted system call. 【系统调用打断】
  interrupted = ASIO_SOCKET_ERROR(EINTR),

  /// Invalid argument. 【参数错误】
  invalid_argument = ASIO_SOCKET_ERROR(EINVAL),

  /// Message too long. 【消息太长】
  message_size = ASIO_SOCKET_ERROR(EMSGSIZE),

  /// The name was too long. 【名称太长】
  name_too_long = ASIO_SOCKET_ERROR(ENAMETOOLONG),

  /// Network is down. 【网络不可用】
  network_down = ASIO_SOCKET_ERROR(ENETDOWN),

  /// Network dropped connection on reset. 【网络已重置】
  network_reset = ASIO_SOCKET_ERROR(ENETRESET),

  /// Network is unreachable. 【网络不可达】
  network_unreachable = ASIO_SOCKET_ERROR(ENETUNREACH),

  /// Too many open files. 【没有文件描述符[打开文件太多]】
  no_descriptors = ASIO_SOCKET_ERROR(EMFILE),

  /// No buffer space available. 【缓冲区空间不足】
  no_buffer_space = ASIO_SOCKET_ERROR(ENOBUFS),

  /// Cannot allocate memory. 【不能分配内存】
  no_memory = ASIO_WIN_OR_POSIX(
      ASIO_NATIVE_ERROR(ERROR_OUTOFMEMORY),
      ASIO_NATIVE_ERROR(ENOMEM)),

  /// Operation not permitted. 【不允许的操作】
  no_permission = ASIO_WIN_OR_POSIX(
      ASIO_NATIVE_ERROR(ERROR_ACCESS_DENIED),
      ASIO_NATIVE_ERROR(EPERM)),

  /// Protocol not available. 【协议不可用】
  no_protocol_option = ASIO_SOCKET_ERROR(ENOPROTOOPT),

  /// No such device. 【找不到设备】
  no_such_device = ASIO_WIN_OR_POSIX(
      ASIO_NATIVE_ERROR(ERROR_BAD_UNIT),
      ASIO_NATIVE_ERROR(ENODEV)),

  /// Transport endpoint is not connected. 【端点未连接】
  not_connected = ASIO_SOCKET_ERROR(ENOTCONN),

  /// Socket operation on non-socket. 【操作并非套接字】
  not_socket = ASIO_SOCKET_ERROR(ENOTSOCK),

  /// Operation cancelled. 【操作终止】
  operation_aborted = ASIO_WIN_OR_POSIX(
      ASIO_NATIVE_ERROR(ERROR_OPERATION_ABORTED),
      ASIO_NATIVE_ERROR(ECANCELED)),

  /// Operation not supported. 【不支持的操作】
  operation_not_supported = ASIO_SOCKET_ERROR(EOPNOTSUPP),

  /// Cannot send after transport endpoint shutdown. 【端点已终止，无法继续发送】
  shut_down = ASIO_SOCKET_ERROR(ESHUTDOWN),

  /// Connection timed out. 【连接超时】
  timed_out = ASIO_SOCKET_ERROR(ETIMEDOUT),

  /// Resource temporarily unavailable. 【资源暂时不可用】
  try_again = ASIO_WIN_OR_POSIX(
      ASIO_NATIVE_ERROR(ERROR_RETRY),
      ASIO_NATIVE_ERROR(EAGAIN)),

  /// The socket is marked non-blocking and the requested operation would block.【套接字设定为非阻塞，但要求阻塞】
  would_block = ASIO_SOCKET_ERROR(EWOULDBLOCK)
};

enum netdb_errors // getaddrinfo的错误
{
  /// Host not found (authoritative). 【未找到主机(HOST_NOT_FOUND)】
  host_not_found = ASIO_NETDB_ERROR(HOST_NOT_FOUND),

  /// Host not found (non-authoritative). 【未找到主机(TRY_AGAIN)】
  host_not_found_try_again = ASIO_NETDB_ERROR(TRY_AGAIN),

  /// The query is valid but does not have associated address data. 【没有数据】
  no_data = ASIO_NETDB_ERROR(NO_DATA),

  /// A non-recoverable error occurred. 【不可恢复】
  no_recovery = ASIO_NETDB_ERROR(NO_RECOVERY)
};

enum addrinfo_errors
{
  /// The service is not supported for the given socket type.【该服务不支持该套接字类型】
  service_not_found = ASIO_WIN_OR_POSIX(
      ASIO_NATIVE_ERROR(WSATYPE_NOT_FOUND),
      ASIO_GETADDRINFO_ERROR(EAI_SERVICE)),

  /// The socket type is not supported. 【不支持的套接字类型】
  socket_type_not_supported = ASIO_WIN_OR_POSIX(
      ASIO_NATIVE_ERROR(WSAESOCKTNOSUPPORT),
      ASIO_GETADDRINFO_ERROR(EAI_SOCKTYPE))
};

enum misc_errors
{
  /// Already open. 【已打开】
  already_open = 1,

  /// End of file or stream. 【已结束】
  eof,

  /// Element not found. 【未找到】
  not_found,

  /// The descriptor cannot fit into the select system call's fd_set. 【文件描述符设定错误】
  fd_set_failure
};

inline const asio::error_category& get_system_category()
{
  return asio::system_category();
}

#if !defined(ASIO_WINDOWS) && !defined(__CYGWIN__)

extern ASIO_DECL
const asio::error_category& get_netdb_category();

extern ASIO_DECL
const asio::error_category& get_addrinfo_category();

#else // !defined(ASIO_WINDOWS) && !defined(__CYGWIN__)

inline const asio::error_category& get_netdb_category()
{
  return get_system_category();
}

inline const asio::error_category& get_addrinfo_category()
{
  return get_system_category();
}

#endif // !defined(ASIO_WINDOWS) && !defined(__CYGWIN__)

extern ASIO_DECL
const asio::error_category& get_misc_category();

static const asio::error_category&
  system_category ASIO_UNUSED_VARIABLE
  = asio::error::get_system_category();
static const asio::error_category&
  netdb_category ASIO_UNUSED_VARIABLE
  = asio::error::get_netdb_category();
static const asio::error_category&
  addrinfo_category ASIO_UNUSED_VARIABLE
  = asio::error::get_addrinfo_category();
static const asio::error_category&
  misc_category ASIO_UNUSED_VARIABLE
  = asio::error::get_misc_category();

} // namespace error
} // namespace asio

#if defined(ASIO_HAS_STD_SYSTEM_ERROR)
namespace std {

template<> struct is_error_code_enum<asio::error::basic_errors>
{
  static const bool value = true;
};

template<> struct is_error_code_enum<asio::error::netdb_errors>
{
  static const bool value = true;
};

template<> struct is_error_code_enum<asio::error::addrinfo_errors>
{
  static const bool value = true;
};

template<> struct is_error_code_enum<asio::error::misc_errors>
{
  static const bool value = true;
};

} // namespace std
#endif // defined(ASIO_HAS_STD_SYSTEM_ERROR)

namespace asio {
namespace error {

inline asio::error_code make_error_code(basic_errors e)
{
  return asio::error_code(
      static_cast<int>(e), get_system_category());
}

inline asio::error_code make_error_code(netdb_errors e)
{
  return asio::error_code(
      static_cast<int>(e), get_netdb_category());
}

inline asio::error_code make_error_code(addrinfo_errors e)
{
  return asio::error_code(
      static_cast<int>(e), get_addrinfo_category());
}

inline asio::error_code make_error_code(misc_errors e)
{
  return asio::error_code(
      static_cast<int>(e), get_misc_category());
}

} // namespace error
namespace stream_errc {
  // Simulates the proposed stream_errc scoped enum.
  using error::eof;
  using error::not_found;
} // namespace stream_errc
namespace socket_errc {
  // Simulates the proposed socket_errc scoped enum.
  using error::already_open;
  using error::not_found;
} // namespace socket_errc
namespace resolver_errc {
  // Simulates the proposed resolver_errc scoped enum.
  using error::host_not_found;
  const error::netdb_errors try_again = error::host_not_found_try_again;
  using error::service_not_found;
} // namespace resolver_errc
} // namespace asio

#include "asio/detail/pop_options.hpp"

#undef ASIO_NATIVE_ERROR
#undef ASIO_SOCKET_ERROR
#undef ASIO_NETDB_ERROR
#undef ASIO_GETADDRINFO_ERROR
#undef ASIO_WIN_OR_POSIX

#if defined(ASIO_HEADER_ONLY)
# include "asio/impl/error.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_ERROR_HPP
