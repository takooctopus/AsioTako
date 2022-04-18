//
// TAKO: socket参数【这里面定义的到mulicast.hpp中使用】
// detail/socket_option.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_DETAIL_SOCKET_OPTION_HPP
#define ASIO_IP_DETAIL_SOCKET_OPTION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include "asio/detail/socket_ops.hpp"
#include "asio/detail/socket_types.hpp"
#include "asio/detail/throw_exception.hpp"
#include "asio/ip/address.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {
namespace detail {
namespace socket_option {

// Helper template for implementing multicast enable loopback options.【应该是int setsockopt(int socket, int level, int option_name, const void *option_value, size_t option_len);这里面对应的帮助方法吧，这个模板类只用来实现多播开启或关闭本地回馈=>IP_MULTICAST_LOOP/IPV6_MULTICAST_LOOP】
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class multicast_enable_loopback
{
public:
#if defined(__sun) || defined(__osf__)
  typedef unsigned char ipv4_value_type;
  typedef unsigned char ipv6_value_type;
#elif defined(_AIX) || defined(__hpux) || defined(__QNXNTO__) 
  typedef unsigned char ipv4_value_type;
  typedef unsigned int ipv6_value_type;
#else
  typedef int ipv4_value_type;  //定义ipv4的值类型int【虽然我看文档上写的是uint，当然最后设置的值是只有0和1】
  typedef int ipv6_value_type;  //定义ipv6的值类型int【虽然我看文档上写的是uint，当然最后设置的值是只有0和1】
#endif

  // Default constructor. 【默认都是0】
  multicast_enable_loopback()
    : ipv4_value_(0),
      ipv6_value_(0)
  {
  }

  // Construct with a specific option value. 【存在v置1，否则为0】
  explicit multicast_enable_loopback(bool v)
    : ipv4_value_(v ? 1 : 0),
      ipv6_value_(v ? 1 : 0)
  {
  }

  // Set the value of the boolean.
  multicast_enable_loopback& operator=(bool v)
  {
    ipv4_value_ = v ? 1 : 0;
    ipv6_value_ = v ? 1 : 0;
    return *this;
  }

  // Get the current value of the boolean.
  bool value() const
  {
    return !!ipv4_value_;
  }

  // Convert to bool.
  operator bool() const
  {
    return !!ipv4_value_;
  }

  // Test for false.
  bool operator!() const
  {
    return !ipv4_value_;
  }

  // Get the level of the socket option. 【IPPROTO_IP/IPPROTO_IPV6】两个level
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option. 【IP_MULTICAST_LOOP/IPV6_MULTICAST_LOOP】两个名字
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the boolean data. 【虽然这两个值类型不是bool但是返回值的地址】
  template <typename Protocol>
  void* data(const Protocol& protocol)
  {
    if (protocol.family() == PF_INET6) 
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the address of the boolean data. 【返回ipv4/ipv6 value=>这个bool的地址，这个值是初始化时传入的参数】
  template <typename Protocol>
  const void* data(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the size of the boolean data. 【获取当前设置值的大小 sizeof(uint) 讲道理啊数据类型是uint，不知道为什么这里是int 没差啦。都是32】
  template <typename Protocol>
  std::size_t size(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return sizeof(ipv6_value_);
    return sizeof(ipv4_value_);
  }

  // Set the size of the boolean data.
  template <typename Protocol>
  void resize(const Protocol& protocol, std::size_t s)
  {
    if (protocol.family() == PF_INET6)
    {
      if (s != sizeof(ipv6_value_))
      {
        std::length_error ex("multicast_enable_loopback socket option resize");
        asio::detail::throw_exception(ex);
      }
      ipv4_value_ = ipv6_value_ ? 1 : 0;
    }
    else
    {
      if (s != sizeof(ipv4_value_))
      {
        std::length_error ex("multicast_enable_loopback socket option resize");
        asio::detail::throw_exception(ex);
      }
      ipv6_value_ = ipv4_value_ ? 1 : 0;
    }
  }

private:
  ipv4_value_type ipv4_value_;
  ipv6_value_type ipv6_value_;
};

// Helper template for implementing unicast hops options. 【设置单播的跳数TTL IP_TTL/IPV6_UNICAST_HOPS】
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class unicast_hops
{
public:
  // Default constructor.
  unicast_hops()
    : value_(0)
  {
  }

  // Construct with a specific option value. 【存活时间IP_TTL => (int)】
  explicit unicast_hops(int v)
    : value_(v)
  {
  }

  // Set the value of the option.
  unicast_hops& operator=(int v)
  {
    value_ = v;
    return *this;
  }

  // Get the current value of the option. 【返回当前option的值=>ttl】
  int value() const
  {
    return value_;
  }

  // Get the level of the socket option. 【获取当前设置的socket level => IPPROTO_IP】
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option. 【获取当前的option name => IP_TTL】
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the data. 【获取当前的设置值 => 一般来说默认的话Linux(64/255) win(128)】
  template <typename Protocol>
  int* data(const Protocol&)
  {
    return &value_;
  }

  // Get the address of the data. 【获取设置值的地址】
  template <typename Protocol>
  const int* data(const Protocol&) const
  {
    return &value_;
  }

  // Get the size of the data. 【获取设置值的大小 sizeof(int)】
  template <typename Protocol>
  std::size_t size(const Protocol&) const
  {
    return sizeof(value_);
  }

  // Set the size of the data. 【设置值的大小(定死了的吧int => 32)】
  template <typename Protocol>
  void resize(const Protocol&, std::size_t s)
  {
    if (s != sizeof(value_))
    {
      std::length_error ex("unicast hops socket option resize");
      asio::detail::throw_exception(ex);
    }
#if defined(__hpux)
    if (value_ < 0)
      value_ = value_ & 0xFF;
#endif
  }

private:
  int value_;
};

// Helper template for implementing multicast hops options. 【IP_MULTICAST_TTL/IPV6_MULTICAST_HOPS => uchar/uint】
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class multicast_hops
{
public:
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  typedef int ipv4_value_type;
#else
  typedef unsigned char ipv4_value_type;    //看文档是ipv4 => uchar
#endif
  typedef int ipv6_value_type; //看文档 ipv6 => uint

  // Default constructor.
  multicast_hops()
    : ipv4_value_(0),
      ipv6_value_(0)
  {
  }

  // Construct with a specific option value. 【设置多播跳数0-255】
  explicit multicast_hops(int v)
  {
    if (v < 0 || v > 255)
    {
      std::out_of_range ex("multicast hops value out of range");
      asio::detail::throw_exception(ex);
    }
    ipv4_value_ = (ipv4_value_type)v;
    ipv6_value_ = v;
  }

  // Set the value of the option.
  multicast_hops& operator=(int v)
  {
    if (v < 0 || v > 255)
    {
      std::out_of_range ex("multicast hops value out of range");
      asio::detail::throw_exception(ex);
    }
    ipv4_value_ = (ipv4_value_type)v;
    ipv6_value_ = v;
    return *this;
  }

  // Get the current value of the option. 【因为跳数设置时同时初始化了v4的和v6的，值都是一样的，为了方便就只返回v6的】
  int value() const
  {
    return ipv6_value_;
  }

  // Get the level of the socket option. 【IPPROTO_IP/IPPROTO_IPV6】
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option. 【IP_MULTICAST_TTL/IPV6_MULTICAST_HOPS】
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the data. 【获取设置值的指针】
  template <typename Protocol>
  void* data(const Protocol& protocol)
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the address of the data. 【获取设置值的常指针】
  template <typename Protocol>
  const void* data(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the size of the data. 【size嘛 u8/u32 => 8/32】
  template <typename Protocol>
  std::size_t size(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return sizeof(ipv6_value_);
    return sizeof(ipv4_value_);
  }

  // Set the size of the data. 【设置数据大小】
  template <typename Protocol>
  void resize(const Protocol& protocol, std::size_t s)
  {
    if (protocol.family() == PF_INET6)
    {
      if (s != sizeof(ipv6_value_))
      {
        std::length_error ex("multicast hops socket option resize");
        asio::detail::throw_exception(ex);
      }
      if (ipv6_value_ < 0)
        ipv4_value_ = 0;
      else if (ipv6_value_ > 255)
        ipv4_value_ = 255;
      else
        ipv4_value_ = (ipv4_value_type)ipv6_value_;
    }
    else
    {
      if (s != sizeof(ipv4_value_))
      {
        std::length_error ex("multicast hops socket option resize");
        asio::detail::throw_exception(ex);
      }
      ipv6_value_ = ipv4_value_;
    }
  }

private:
  ipv4_value_type ipv4_value_;
  ipv6_value_type ipv6_value_;
};

// Helper template for implementing ip_mreq-based options. 【设置加入/离开多播组相关设置 IP_ADD_MEMBERSHIP/IP_DROP_MEMBERSHIP/IPV6_ADD_MEMBERSHIP/IPV6_DROP_MEMBERSHIP】
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class multicast_request
{
public:
  // Default constructor. 【默认创建啥都没有空的也行】
  multicast_request()
    : ipv4_value_(), // Zero-initialisation gives the "any" address.
      ipv6_value_() // Zero-initialisation gives the "any" address.
  {
  }

  // Construct with multicast address only. 【传入一个adress对象引用，我们根据adress.type_来进行判断】
  explicit multicast_request(const address& multicast_address)
    : ipv4_value_(), // Zero-initialisation gives the "any" address.
      ipv6_value_() // Zero-initialisation gives the "any" address.
  {
    if (multicast_address.is_v6())  //要是ipv6
    {
      using namespace std; // For memcpy.
      address_v6 ipv6_address = multicast_address.to_v6();  //转换成address_v6对象
      address_v6::bytes_type bytes = ipv6_address.to_bytes(); //address_v6对象生成u8*16数组
      memcpy(ipv6_value_.ipv6mr_multiaddr.s6_addr, bytes.data(), 16);   //使用memcpy拷贝128bit到 ipv6_value_.ipv6mr_multiaddr.s6_addr 中
      ipv6_value_.ipv6mr_interface = ipv6_address.scope_id(); // 别忘了ipv6还有scoped_id
    }
    else
    {
      ipv4_value_.imr_multiaddr.s_addr =
        asio::detail::socket_ops::host_to_network_long(
            multicast_address.to_v4().to_uint());   // 同样的ipv4就不用那么麻烦，直接转换成u32就行【当然这里的u32是主机字节序，我们还得转换成网络字节序】
      ipv4_value_.imr_interface.s_addr =
        asio::detail::socket_ops::host_to_network_long(
            address_v4::any().to_uint());   //imr_interface设置成0.0.0.0
    }
  }

  // Construct with multicast address and IPv4 address specifying an interface.【传入address_v4的对象引用，这样我们得有俩参数】
  explicit multicast_request(const address_v4& multicast_address,
      const address_v4& network_interface = address_v4::any())
    : ipv6_value_() // Zero-initialisation gives the "any" address.
  {
    ipv4_value_.imr_multiaddr.s_addr =
      asio::detail::socket_ops::host_to_network_long(
          multicast_address.to_uint()); // =>先转成uint成为主机字节序u32 => 别忘了转换成网络字节序[大端]
    ipv4_value_.imr_interface.s_addr =
      asio::detail::socket_ops::host_to_network_long(
          network_interface.to_uint());
  }

  // Construct with multicast address and IPv6 network interface index. 【传入address_v6对象引用】
  explicit multicast_request(
      const address_v6& multicast_address,
      unsigned long network_interface = 0)
    : ipv4_value_() // Zero-initialisation gives the "any" address.
  {
    using namespace std; // For memcpy.
    address_v6::bytes_type bytes = multicast_address.to_bytes();    //转换成u8*16数组，再memcpy
    memcpy(ipv6_value_.ipv6mr_multiaddr.s6_addr, bytes.data(), 16);
    if (network_interface)
      ipv6_value_.ipv6mr_interface = network_interface;
    else
      ipv6_value_.ipv6mr_interface = multicast_address.scope_id(); //ipv6的作用域使用的是scope_id
  }

  // Get the level of the socket option. 【IPPROTO_IP/IPPROTO_IPV6】
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option. 【IP_ADD_MEMBERSHIP/IP_DROP_MEMBERSHIP/IPV6_ADD_MEMBERSHIP/IPV6_DROP_MEMBERSHIP】
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the option data. 【获取设置的值引用，引用的对象本质是一个结构体ip_mreq{}/ipv6_mreq{}】
  template <typename Protocol>
  const void* data(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the size of the option data.【获取设置的值常引用，引用的对象本质是一个结构体ip_mreq{}/ipv6_mreq{}】
  template <typename Protocol>
  std::size_t size(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return sizeof(ipv6_value_);
    return sizeof(ipv4_value_);
  }

private:
  asio::detail::in4_mreq_type ipv4_value_; //ipv4组播结构体对象
  asio::detail::in6_mreq_type ipv6_value_; //ipv6组播结构体对象
};

// Helper template for implementing options that specify a network interface. 【特别指定外出网络接口IP_MULTICAST_IF/IPV6_MULTICAST_IF => in_addr{}/in6_addr{}】
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class network_interface
{
public:
  // Default constructor. 【默认构造函数0.0.0.0 / ::】
  network_interface()
  {
    ipv4_value_.s_addr =
      asio::detail::socket_ops::host_to_network_long(
          address_v4::any().to_uint());
    ipv6_value_ = 0;
  }

  // Construct with IPv4 interface. 【使用address_v4引用构造】
  explicit network_interface(const address_v4& ipv4_interface)
  {
    ipv4_value_.s_addr =
      asio::detail::socket_ops::host_to_network_long(
          ipv4_interface.to_uint()); //用了uint别忘了转换成网络字节序
    ipv6_value_ = 0;
  }

  // Construct with IPv6 interface. 【u32】
  explicit network_interface(unsigned int ipv6_interface)
  {
    ipv4_value_.s_addr =
      asio::detail::socket_ops::host_to_network_long(
          address_v4::any().to_uint()); //ipv4的设置为0.0.0.0
    ipv6_value_ = ipv6_interface;
  }

  // Get the level of the socket option. 【IPPROTO_IP/IPPROTO_IPV6】
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option. 【IP_MULTICAST_IF/IPV6_MULTICAST_IF】
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the option data. 【获取设置的数据地址】
  template <typename Protocol>
  const void* data(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the size of the option data. 【数据大小】
  template <typename Protocol>
  std::size_t size(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return sizeof(ipv6_value_);
    return sizeof(ipv4_value_);
  }

private:
  asio::detail::in4_addr_type ipv4_value_; //【in_addr 共u32】
  unsigned int ipv6_value_; //【TOQ: 不知道为什么这里用u32，按理说应该用in6_addr{},u8*16才对】
};

} // namespace socket_option
} // namespace detail
} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_DETAIL_SOCKET_OPTION_HPP
