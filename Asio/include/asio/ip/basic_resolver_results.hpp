//
// TAKO:有查询类就有解析的结果类
// ip/basic_resolver_results.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_BASIC_RESOLVER_RESULTS_HPP
#define ASIO_IP_BASIC_RESOLVER_RESULTS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <cstddef>
#include <cstring>
#include "asio/detail/socket_ops.hpp"
#include "asio/detail/socket_types.hpp" //用于套接字的常量定义
#include "asio/ip/basic_resolver_iterator.hpp"  //解析类的迭代器

#if defined(ASIO_WINDOWS_RUNTIME)
# include "asio/detail/winrt_utils.hpp"
#endif // defined(ASIO_WINDOWS_RUNTIME)

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

/// A range of entries produced by a resolver.
/**
 * The asio::ip::basic_resolver_results class template is used to define
 * a range over the results returned by a resolver.
 * 【解析器返回的结果】
 *
 * The iterator's value_type, obtained when a results iterator is dereferenced,
 * is: @code const basic_resolver_entry<InternetProtocol> @endcode
 *
 * @note For backward compatibility, basic_resolver_results is derived from
 * basic_resolver_iterator. This derivation is deprecated.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
template <typename InternetProtocol>
class basic_resolver_results
#if !defined(ASIO_NO_DEPRECATED)
  : public basic_resolver_iterator<InternetProtocol>    //继承于基本迭代器模板类的特定类型
#else // !defined(ASIO_NO_DEPRECATED)
  : private basic_resolver_iterator<InternetProtocol>
#endif // !defined(ASIO_NO_DEPRECATED)
{
public:
  /// The protocol type associated with the results. 【定义其协议类型，方便多态化】
  typedef InternetProtocol protocol_type;

  /// The endpoint type associated with the results. 【定义端点类型，方便多态化】
  typedef typename protocol_type::endpoint endpoint_type;

  /// The type of a value in the results range. 【定义基本解析开始类，方便多态化】
  typedef basic_resolver_entry<protocol_type> value_type;

  /// The type of a const reference to a value in the range.
  typedef const value_type& const_reference;

  /// The type of a non-const reference to a value in the range.
  typedef value_type& reference;

  /// The type of an iterator into the range.【定义基本的迭代器】
  typedef basic_resolver_iterator<protocol_type> const_iterator;

  /// The type of an iterator into the range.
  typedef const_iterator iterator;

  /// Type used to represent the distance between two iterators in the range.
  typedef std::ptrdiff_t difference_type;

  /// Type used to represent a count of the elements in the range.
  typedef std::size_t size_type;

  /// Default constructor creates an empty range. 【默认构造，啥都没有】
  basic_resolver_results()
  {
  }

  /// Copy constructor. 【拷贝构造】
  basic_resolver_results(const basic_resolver_results& other)
    : basic_resolver_iterator<InternetProtocol>(other)  //继承就要初始化基类
  {
  }

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move constructor.
  basic_resolver_results(basic_resolver_results&& other)
    : basic_resolver_iterator<InternetProtocol>(
        ASIO_MOVE_CAST(basic_resolver_results)(other))  //下面都是实现了move_cast的，所以可以放心用
  {
  }
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Assignment operator.
  basic_resolver_results& operator=(const basic_resolver_results& other)
  {
    basic_resolver_iterator<InternetProtocol>::operator=(other);
    return *this;
  }

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move-assignment operator.
  basic_resolver_results& operator=(basic_resolver_results&& other)
  {
    basic_resolver_iterator<InternetProtocol>::operator=(
        ASIO_MOVE_CAST(basic_resolver_results)(other));
    return *this;
  }
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

#if !defined(GENERATING_DOCUMENTATION)
  // Create results from an addrinfo list returned by getaddrinfo. 【[static]使用getaddrinfo返回的值创建一个实例】
  static basic_resolver_results create(
      asio::detail::addrinfo_type* address_info,
      const std::string& host_name, const std::string& service_name)
  {
    basic_resolver_results results;     //初始化一个实例
    if (!address_info)
      return results;

    std::string actual_host_name = host_name;   
    if (address_info->ai_canonname)
      actual_host_name = address_info->ai_canonname;    //有规范名用规范名

    results.values_.reset(new values_type); //我们要记着values_是一个shared_ptr

    while (address_info)
    {
      if (address_info->ai_family == ASIO_OS_DEF(AF_INET)
          || address_info->ai_family == ASIO_OS_DEF(AF_INET6))  //看看地址族是不是AF_INET/AF_INET6
      {
        using namespace std; // For memcpy.
        typename InternetProtocol::endpoint endpoint;
        endpoint.resize(static_cast<std::size_t>(address_info->ai_addrlen)); //设置长度16/28
        memcpy(endpoint.data(), address_info->ai_addr,
            address_info->ai_addrlen);
        results.values_->push_back(
            basic_resolver_entry<InternetProtocol>(endpoint,
              actual_host_name, service_name)); //往vector里面添加一个新的basic_resolver_entry实例
      }
      address_info = address_info->ai_next; //【一个域名可能对应多个IP地址】
    }

    return results;
  }

  // Create results from an endpoint, host name and service name. 【[static]传入端点，主机名，服务名】
  static basic_resolver_results create(const endpoint_type& endpoint,
      const std::string& host_name, const std::string& service_name)
  {
    basic_resolver_results results; //新建实例
    results.values_.reset(new values_type); //shared_ptr重置向量
    results.values_->push_back(
        basic_resolver_entry<InternetProtocol>(
          endpoint, host_name, service_name));  //往向量中添加一个新的basic_resolver_entry解析入口 
    return results;
  }

  // Create results from a sequence of endpoints, host and service name. 【端点，主机名，服务名】
  template <typename EndpointIterator>
  static basic_resolver_results create(
      EndpointIterator begin, EndpointIterator end,
      const std::string& host_name, const std::string& service_name)
  {
    basic_resolver_results results;
    if (begin != end)
    {
      results.values_.reset(new values_type);
      for (EndpointIterator ep_iter = begin; ep_iter != end; ++ep_iter)
      {
        results.values_->push_back(
            basic_resolver_entry<InternetProtocol>(
              *ep_iter, host_name, service_name));
      }
    }
    return results;
  }

# if defined(ASIO_WINDOWS_RUNTIME)
  // Create results from a Windows Runtime list of EndpointPair objects.
  static basic_resolver_results create(
      Windows::Foundation::Collections::IVectorView<
        Windows::Networking::EndpointPair^>^ endpoints,
      const asio::detail::addrinfo_type& hints,
      const std::string& host_name, const std::string& service_name)
  {
    basic_resolver_results results;
    if (endpoints->Size)
    {
      results.values_.reset(new values_type);
      for (unsigned int i = 0; i < endpoints->Size; ++i)
      {
        auto pair = endpoints->GetAt(i);

        if (hints.ai_family == ASIO_OS_DEF(AF_INET)
            && pair->RemoteHostName->Type
              != Windows::Networking::HostNameType::Ipv4)
          continue;

        if (hints.ai_family == ASIO_OS_DEF(AF_INET6)
            && pair->RemoteHostName->Type
              != Windows::Networking::HostNameType::Ipv6)
          continue;

        results.values_->push_back(
            basic_resolver_entry<InternetProtocol>(
              typename InternetProtocol::endpoint(
                ip::make_address(
                  asio::detail::winrt_utils::string(
                    pair->RemoteHostName->CanonicalName)),
                asio::detail::winrt_utils::integer(
                  pair->RemoteServiceName)),
              host_name, service_name));
      }
    }
    return results;
  }
# endif // defined(ASIO_WINDOWS_RUNTIME)
#endif // !defined(GENERATING_DOCUMENTATION)

  /// Get the number of entries in the results range.   【返回容器中解析入口实例的数量】
  size_type size() const ASIO_NOEXCEPT
  {
    return this->values_ ? this->values_->size() : 0;
  }

  /// Get the maximum number of entries permitted in a results range. 【返回容器最大所能容纳的元素个数】
  size_type max_size() const ASIO_NOEXCEPT
  {
    return this->values_ ? this->values_->max_size() : values_type().max_size();
  }

  /// Determine whether the results range is empty. 【看看shared_ptr保有的vector是不是空的】
  bool empty() const ASIO_NOEXCEPT
  {
    return this->values_ ? this->values_->empty() : true;
  }

  /// Obtain a begin iterator for the results range. 【返回新的results实例，但是其index置为0】
  const_iterator begin() const
  {
    basic_resolver_results tmp(*this);
    tmp.index_ = 0;
    return ASIO_MOVE_CAST(basic_resolver_results)(tmp);
  }

  /// Obtain an end iterator for the results range. 【返回一个空的result实例】
  const_iterator end() const
  {
    return const_iterator();
  }

  /// Obtain a begin iterator for the results range. 【和上面函数相同】
  const_iterator cbegin() const
  {
    return begin();
  }

  /// Obtain an end iterator for the results range. 【和上面函数相同】
  const_iterator cend() const
  {
    return end();
  }

  /// Swap the results range with another. 【交换两个result，数据交换，其index也要交换】
  void swap(basic_resolver_results& that) ASIO_NOEXCEPT 
  {
    if (this != &that)
    {
      this->values_.swap(that.values_);
      std::size_t index = this->index_;
      this->index_ = that.index_;
      that.index_ = index;
    }
  }

  /// Test two iterators for equality. 【相等比较】
  friend bool operator==(const basic_resolver_results& a,
      const basic_resolver_results& b)
  {
    return a.equal(b);
  }

  /// Test two iterators for inequality. 【不等比较】
  friend bool operator!=(const basic_resolver_results& a,
      const basic_resolver_results& b)
  {
    return !a.equal(b);
  }

private:
  typedef std::vector<basic_resolver_entry<InternetProtocol> > values_type; //定义别名，反正是一个向量
};

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_BASIC_RESOLVER_RESULTS_HPP
