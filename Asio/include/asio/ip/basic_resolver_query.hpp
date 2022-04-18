//
// TAKO:基本解析类的查询方法
// ip/basic_resolver_query.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_BASIC_RESOLVER_QUERY_HPP
#define ASIO_IP_BASIC_RESOLVER_QUERY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <string>
#include "asio/detail/socket_ops.hpp"
#include "asio/ip/resolver_query_base.hpp"  //继承这个吧，主要是有一个protected的析构方法

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

/// An query to be passed to a resolver.
/**
 * The asio::ip::basic_resolver_query class template describes a query
 * that can be passed to a resolver.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
template <typename InternetProtocol>
class basic_resolver_query
  : public resolver_query_base
{
public:
  /// The protocol type associated with the endpoint query. 
  typedef InternetProtocol protocol_type;   //InternetProtocol是特例化的protocol类，具体的话就是那几个=>datagram_protocol/raw_protocol/seq_protocol/stream_protocol

  /// Construct with specified service name for any protocol.
  /**
   * This constructor is typically used to perform name resolution for local
   * service binding.
   *
   * @param service A string identifying the requested service. This may be a
   * descriptive name or a numeric string corresponding to a port number.
   * 【service：字符串=>指明了需要的服务】
   *
   * @param resolve_flags A set of flags that determine how name resolution
   * should be performed. The default flags are suitable for local service
   * binding.
   * 【resolve_flags: 标志，决定这个解析器怎么工作[回去看resolver_base.hpp里面的定义]】
   *
   * @note On POSIX systems, service names are typically defined in the file
   * <tt>/etc/services</tt>. On Windows, service names may be found in the file
   * <tt>c:\\windows\\system32\\drivers\\etc\\services</tt>. Operating systems
   * may use additional locations when resolving service names.
   */
  basic_resolver_query(const std::string& service,
      resolver_query_base::flags resolve_flags = passive | address_configured)
    : hints_(),
      host_name_(),
      service_name_(service)    //默认resolve_flags[socket地址用于监听绑定 | 查询配置的地址类型(IPv4或IPv6)]
  {
    typename InternetProtocol::endpoint endpoint; //具体协议下面特例化模板的endpoint类
    hints_.ai_flags = static_cast<int>(resolve_flags);  
    hints_.ai_family = PF_UNSPEC;   //默认构造，协议族未指定
    hints_.ai_socktype = endpoint.protocol().type(); //【绕:这个endpoint类在InternetProtocol中定义，它又传入一个InternetProtocol=>最后其实还是调用的这个InternetProtocol的type()方法】 【具体返回：SOCK_STREAM/SOCK_DGRAM/...】
    hints_.ai_protocol = endpoint.protocol().protocol(); //【具体返回: IPPROTO_TCP/IPPROTO_UDP...】
    hints_.ai_addrlen = 0;  //must be zero or a null pointer
    hints_.ai_canonname = 0;    //must be zero or a null pointer
    hints_.ai_addr = 0; //must be zero or a null pointer
    hints_.ai_next = 0; //must be zero or a null pointer
  }

  /// Construct with specified service name for a given protocol.
  /**
   * This constructor is typically used to perform name resolution for local
   * service binding with a specific protocol version.
   *
   * @param protocol A protocol object, normally representing either the IPv4 or
   * IPv6 version of an internet protocol.
   *
   * @param service A string identifying the requested service. This may be a
   * descriptive name or a numeric string corresponding to a port number.
   *
   * @param resolve_flags A set of flags that determine how name resolution
   * should be performed. The default flags are suitable for local service
   * binding.
   *
   * @note On POSIX systems, service names are typically defined in the file
   * <tt>/etc/services</tt>. On Windows, service names may be found in the file
   * <tt>c:\\windows\\system32\\drivers\\etc\\services</tt>. Operating systems
   * may use additional locations when resolving service names.
   */
  basic_resolver_query(const protocol_type& protocol,   //多传一个proto_type
      const std::string& service,
      resolver_query_base::flags resolve_flags = passive | address_configured)
    : hints_(),
      host_name_(),
      service_name_(service)
  {
    hints_.ai_flags = static_cast<int>(resolve_flags);
    hints_.ai_family = protocol.family();   //地址族
    hints_.ai_socktype = protocol.type();   //套接字类型
    hints_.ai_protocol = protocol.protocol();   //套接字协议
    hints_.ai_addrlen = 0;
    hints_.ai_canonname = 0;
    hints_.ai_addr = 0;
    hints_.ai_next = 0;
  }

  /// Construct with specified host name and service name for any protocol.
  /**
   * This constructor is typically used to perform name resolution for
   * communication with remote hosts.
   *
   * @param host A string identifying a location. May be a descriptive name or
   * a numeric address string. If an empty string and the passive flag has been
   * specified, the resolved endpoints are suitable for local service binding.
   * If an empty string and passive is not specified, the resolved endpoints
   * will use the loopback address.
   *
   * @param service A string identifying the requested service. This may be a
   * descriptive name or a numeric string corresponding to a port number. May
   * be an empty string, in which case all resolved endpoints will have a port
   * number of 0.
   *
   * @param resolve_flags A set of flags that determine how name resolution
   * should be performed. The default flags are suitable for communication with
   * remote hosts.
   *
   * @note On POSIX systems, host names may be locally defined in the file
   * <tt>/etc/hosts</tt>. On Windows, host names may be defined in the file
   * <tt>c:\\windows\\system32\\drivers\\etc\\hosts</tt>. Remote host name
   * resolution is performed using DNS. Operating systems may use additional
   * locations when resolving host names (such as NETBIOS names on Windows).
   *
   * On POSIX systems, service names are typically defined in the file
   * <tt>/etc/services</tt>. On Windows, service names may be found in the file
   * <tt>c:\\windows\\system32\\drivers\\etc\\services</tt>. Operating systems
   * may use additional locations when resolving service names.
   */
  basic_resolver_query(const std::string& host, const std::string& service,
      resolver_query_base::flags resolve_flags = address_configured)    //默认address_configured=>查询配置的地址类型(IPv4或IPv6)
    : hints_(),
      host_name_(host),
      service_name_(service)
  {
    typename InternetProtocol::endpoint endpoint;
    hints_.ai_flags = static_cast<int>(resolve_flags);
    hints_.ai_family = ASIO_OS_DEF(AF_UNSPEC);  //地址族未指定
    hints_.ai_socktype = endpoint.protocol().type();    //套接字类型
    hints_.ai_protocol = endpoint.protocol().protocol();    //套接字协议
    hints_.ai_addrlen = 0;
    hints_.ai_canonname = 0;
    hints_.ai_addr = 0;
    hints_.ai_next = 0;
  }

  /// Construct with specified host name and service name for a given protocol.
  /**
   * This constructor is typically used to perform name resolution for
   * communication with remote hosts.
   *
   * @param protocol A protocol object, normally representing either the IPv4 or
   * IPv6 version of an internet protocol.
   *
   * @param host A string identifying a location. May be a descriptive name or
   * a numeric address string. If an empty string and the passive flag has been
   * specified, the resolved endpoints are suitable for local service binding.
   * If an empty string and passive is not specified, the resolved endpoints
   * will use the loopback address.
   *
   * @param service A string identifying the requested service. This may be a
   * descriptive name or a numeric string corresponding to a port number. May
   * be an empty string, in which case all resolved endpoints will have a port
   * number of 0.
   *
   * @param resolve_flags A set of flags that determine how name resolution
   * should be performed. The default flags are suitable for communication with
   * remote hosts.
   *
   * @note On POSIX systems, host names may be locally defined in the file
   * <tt>/etc/hosts</tt>. On Windows, host names may be defined in the file
   * <tt>c:\\windows\\system32\\drivers\\etc\\hosts</tt>. Remote host name
   * resolution is performed using DNS. Operating systems may use additional
   * locations when resolving host names (such as NETBIOS names on Windows).
   *
   * On POSIX systems, service names are typically defined in the file
   * <tt>/etc/services</tt>. On Windows, service names may be found in the file
   * <tt>c:\\windows\\system32\\drivers\\etc\\services</tt>. Operating systems
   * may use additional locations when resolving service names.
   */
  basic_resolver_query(const protocol_type& protocol,   //制定了套接字协议
      const std::string& host, const std::string& service,
      resolver_query_base::flags resolve_flags = address_configured)
    : hints_(),
      host_name_(host),
      service_name_(service)
  {
    hints_.ai_flags = static_cast<int>(resolve_flags);
    hints_.ai_family = protocol.family();   
    hints_.ai_socktype = protocol.type();
    hints_.ai_protocol = protocol.protocol();
    hints_.ai_addrlen = 0;
    hints_.ai_canonname = 0;
    hints_.ai_addr = 0;
    hints_.ai_next = 0;
  }

  /// Get the hints associated with the query.  获取addrinfo结构体
  const asio::detail::addrinfo_type& hints() const
  {
    return hints_;
  }

  /// Get the host name associated with the query. 【获取主机名】
  std::string host_name() const
  {
    return host_name_;
  }

  /// Get the service name associated with the query. 【获取服务名】
  std::string service_name() const
  {
    return service_name_;
  }

private:
  asio::detail::addrinfo_type hints_;       //<netdb.h> addrinfo结构主要在网络编程解析hostname时使用 getaddrinfo(const char, const char, const struct addrinfo, struct addrinfo*) => freeaddrinfo(struct addrinfo*) 记得拿了要释放掉
  std::string host_name_;                   //主机名
  std::string service_name_;                //服务名
};

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_BASIC_RESOLVER_QUERY_HPP
