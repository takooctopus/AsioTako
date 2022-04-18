//
// TAKO: socket�����������涨��ĵ�mulicast.hpp��ʹ�á�
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

// Helper template for implementing multicast enable loopback options.��Ӧ����int setsockopt(int socket, int level, int option_name, const void *option_value, size_t option_len);�������Ӧ�İ��������ɣ����ģ����ֻ����ʵ�ֶಥ������رձ��ػ���=>IP_MULTICAST_LOOP/IPV6_MULTICAST_LOOP��
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
  typedef int ipv4_value_type;  //����ipv4��ֵ����int����Ȼ�ҿ��ĵ���д����uint����Ȼ������õ�ֵ��ֻ��0��1��
  typedef int ipv6_value_type;  //����ipv6��ֵ����int����Ȼ�ҿ��ĵ���д����uint����Ȼ������õ�ֵ��ֻ��0��1��
#endif

  // Default constructor. ��Ĭ�϶���0��
  multicast_enable_loopback()
    : ipv4_value_(0),
      ipv6_value_(0)
  {
  }

  // Construct with a specific option value. ������v��1������Ϊ0��
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

  // Get the level of the socket option. ��IPPROTO_IP/IPPROTO_IPV6������level
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option. ��IP_MULTICAST_LOOP/IPV6_MULTICAST_LOOP����������
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the boolean data. ����Ȼ������ֵ���Ͳ���bool���Ƿ���ֵ�ĵ�ַ��
  template <typename Protocol>
  void* data(const Protocol& protocol)
  {
    if (protocol.family() == PF_INET6) 
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the address of the boolean data. ������ipv4/ipv6 value=>���bool�ĵ�ַ�����ֵ�ǳ�ʼ��ʱ����Ĳ�����
  template <typename Protocol>
  const void* data(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the size of the boolean data. ����ȡ��ǰ����ֵ�Ĵ�С sizeof(uint) ����������������uint����֪��Ϊʲô������int û����������32��
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

// Helper template for implementing unicast hops options. �����õ���������TTL IP_TTL/IPV6_UNICAST_HOPS��
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class unicast_hops
{
public:
  // Default constructor.
  unicast_hops()
    : value_(0)
  {
  }

  // Construct with a specific option value. �����ʱ��IP_TTL => (int)��
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

  // Get the current value of the option. �����ص�ǰoption��ֵ=>ttl��
  int value() const
  {
    return value_;
  }

  // Get the level of the socket option. ����ȡ��ǰ���õ�socket level => IPPROTO_IP��
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option. ����ȡ��ǰ��option name => IP_TTL��
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the data. ����ȡ��ǰ������ֵ => һ����˵Ĭ�ϵĻ�Linux(64/255) win(128)��
  template <typename Protocol>
  int* data(const Protocol&)
  {
    return &value_;
  }

  // Get the address of the data. ����ȡ����ֵ�ĵ�ַ��
  template <typename Protocol>
  const int* data(const Protocol&) const
  {
    return &value_;
  }

  // Get the size of the data. ����ȡ����ֵ�Ĵ�С sizeof(int)��
  template <typename Protocol>
  std::size_t size(const Protocol&) const
  {
    return sizeof(value_);
  }

  // Set the size of the data. ������ֵ�Ĵ�С(�����˵İ�int => 32)��
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

// Helper template for implementing multicast hops options. ��IP_MULTICAST_TTL/IPV6_MULTICAST_HOPS => uchar/uint��
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class multicast_hops
{
public:
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  typedef int ipv4_value_type;
#else
  typedef unsigned char ipv4_value_type;    //���ĵ���ipv4 => uchar
#endif
  typedef int ipv6_value_type; //���ĵ� ipv6 => uint

  // Default constructor.
  multicast_hops()
    : ipv4_value_(0),
      ipv6_value_(0)
  {
  }

  // Construct with a specific option value. �����öಥ����0-255��
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

  // Get the current value of the option. ����Ϊ��������ʱͬʱ��ʼ����v4�ĺ�v6�ģ�ֵ����һ���ģ�Ϊ�˷����ֻ����v6�ġ�
  int value() const
  {
    return ipv6_value_;
  }

  // Get the level of the socket option. ��IPPROTO_IP/IPPROTO_IPV6��
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option. ��IP_MULTICAST_TTL/IPV6_MULTICAST_HOPS��
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the data. ����ȡ����ֵ��ָ�롿
  template <typename Protocol>
  void* data(const Protocol& protocol)
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the address of the data. ����ȡ����ֵ�ĳ�ָ�롿
  template <typename Protocol>
  const void* data(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the size of the data. ��size�� u8/u32 => 8/32��
  template <typename Protocol>
  std::size_t size(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return sizeof(ipv6_value_);
    return sizeof(ipv4_value_);
  }

  // Set the size of the data. ���������ݴ�С��
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

// Helper template for implementing ip_mreq-based options. �����ü���/�뿪�ಥ��������� IP_ADD_MEMBERSHIP/IP_DROP_MEMBERSHIP/IPV6_ADD_MEMBERSHIP/IPV6_DROP_MEMBERSHIP��
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class multicast_request
{
public:
  // Default constructor. ��Ĭ�ϴ���ɶ��û�пյ�Ҳ�С�
  multicast_request()
    : ipv4_value_(), // Zero-initialisation gives the "any" address.
      ipv6_value_() // Zero-initialisation gives the "any" address.
  {
  }

  // Construct with multicast address only. ������һ��adress�������ã����Ǹ���adress.type_�������жϡ�
  explicit multicast_request(const address& multicast_address)
    : ipv4_value_(), // Zero-initialisation gives the "any" address.
      ipv6_value_() // Zero-initialisation gives the "any" address.
  {
    if (multicast_address.is_v6())  //Ҫ��ipv6
    {
      using namespace std; // For memcpy.
      address_v6 ipv6_address = multicast_address.to_v6();  //ת����address_v6����
      address_v6::bytes_type bytes = ipv6_address.to_bytes(); //address_v6��������u8*16����
      memcpy(ipv6_value_.ipv6mr_multiaddr.s6_addr, bytes.data(), 16);   //ʹ��memcpy����128bit�� ipv6_value_.ipv6mr_multiaddr.s6_addr ��
      ipv6_value_.ipv6mr_interface = ipv6_address.scope_id(); // ������ipv6����scoped_id
    }
    else
    {
      ipv4_value_.imr_multiaddr.s_addr =
        asio::detail::socket_ops::host_to_network_long(
            multicast_address.to_v4().to_uint());   // ͬ����ipv4�Ͳ�����ô�鷳��ֱ��ת����u32���С���Ȼ�����u32�������ֽ������ǻ���ת���������ֽ���
      ipv4_value_.imr_interface.s_addr =
        asio::detail::socket_ops::host_to_network_long(
            address_v4::any().to_uint());   //imr_interface���ó�0.0.0.0
    }
  }

  // Construct with multicast address and IPv4 address specifying an interface.������address_v4�Ķ������ã��������ǵ�����������
  explicit multicast_request(const address_v4& multicast_address,
      const address_v4& network_interface = address_v4::any())
    : ipv6_value_() // Zero-initialisation gives the "any" address.
  {
    ipv4_value_.imr_multiaddr.s_addr =
      asio::detail::socket_ops::host_to_network_long(
          multicast_address.to_uint()); // =>��ת��uint��Ϊ�����ֽ���u32 => ������ת���������ֽ���[���]
    ipv4_value_.imr_interface.s_addr =
      asio::detail::socket_ops::host_to_network_long(
          network_interface.to_uint());
  }

  // Construct with multicast address and IPv6 network interface index. ������address_v6�������á�
  explicit multicast_request(
      const address_v6& multicast_address,
      unsigned long network_interface = 0)
    : ipv4_value_() // Zero-initialisation gives the "any" address.
  {
    using namespace std; // For memcpy.
    address_v6::bytes_type bytes = multicast_address.to_bytes();    //ת����u8*16���飬��memcpy
    memcpy(ipv6_value_.ipv6mr_multiaddr.s6_addr, bytes.data(), 16);
    if (network_interface)
      ipv6_value_.ipv6mr_interface = network_interface;
    else
      ipv6_value_.ipv6mr_interface = multicast_address.scope_id(); //ipv6��������ʹ�õ���scope_id
  }

  // Get the level of the socket option. ��IPPROTO_IP/IPPROTO_IPV6��
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option. ��IP_ADD_MEMBERSHIP/IP_DROP_MEMBERSHIP/IPV6_ADD_MEMBERSHIP/IPV6_DROP_MEMBERSHIP��
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the option data. ����ȡ���õ�ֵ���ã����õĶ�������һ���ṹ��ip_mreq{}/ipv6_mreq{}��
  template <typename Protocol>
  const void* data(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the size of the option data.����ȡ���õ�ֵ�����ã����õĶ�������һ���ṹ��ip_mreq{}/ipv6_mreq{}��
  template <typename Protocol>
  std::size_t size(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return sizeof(ipv6_value_);
    return sizeof(ipv4_value_);
  }

private:
  asio::detail::in4_mreq_type ipv4_value_; //ipv4�鲥�ṹ�����
  asio::detail::in6_mreq_type ipv6_value_; //ipv6�鲥�ṹ�����
};

// Helper template for implementing options that specify a network interface. ���ر�ָ���������ӿ�IP_MULTICAST_IF/IPV6_MULTICAST_IF => in_addr{}/in6_addr{}��
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class network_interface
{
public:
  // Default constructor. ��Ĭ�Ϲ��캯��0.0.0.0 / ::��
  network_interface()
  {
    ipv4_value_.s_addr =
      asio::detail::socket_ops::host_to_network_long(
          address_v4::any().to_uint());
    ipv6_value_ = 0;
  }

  // Construct with IPv4 interface. ��ʹ��address_v4���ù��졿
  explicit network_interface(const address_v4& ipv4_interface)
  {
    ipv4_value_.s_addr =
      asio::detail::socket_ops::host_to_network_long(
          ipv4_interface.to_uint()); //����uint������ת���������ֽ���
    ipv6_value_ = 0;
  }

  // Construct with IPv6 interface. ��u32��
  explicit network_interface(unsigned int ipv6_interface)
  {
    ipv4_value_.s_addr =
      asio::detail::socket_ops::host_to_network_long(
          address_v4::any().to_uint()); //ipv4������Ϊ0.0.0.0
    ipv6_value_ = ipv6_interface;
  }

  // Get the level of the socket option. ��IPPROTO_IP/IPPROTO_IPV6��
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option. ��IP_MULTICAST_IF/IPV6_MULTICAST_IF��
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the option data. ����ȡ���õ����ݵ�ַ��
  template <typename Protocol>
  const void* data(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the size of the option data. �����ݴ�С��
  template <typename Protocol>
  std::size_t size(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return sizeof(ipv6_value_);
    return sizeof(ipv4_value_);
  }

private:
  asio::detail::in4_addr_type ipv4_value_; //��in_addr ��u32��
  unsigned int ipv6_value_; //��TOQ: ��֪��Ϊʲô������u32������˵Ӧ����in6_addr{},u8*16�Ŷԡ�
};

} // namespace socket_option
} // namespace detail
} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_DETAIL_SOCKET_OPTION_HPP
