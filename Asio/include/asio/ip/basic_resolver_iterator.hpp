//
// TAKO:基本解析器的迭代器
// ip/basic_resolver_iterator.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_BASIC_RESOLVER_ITERATOR_HPP
#define ASIO_IP_BASIC_RESOLVER_ITERATOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <cstddef>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>
#include "asio/detail/memory.hpp"   //标准就是那几个只能指针吧
#include "asio/detail/socket_ops.hpp"   //socket操作方法
#include "asio/detail/socket_types.hpp" //socket相关常量
#include "asio/ip/basic_resolver_entry.hpp" //基本解析进入

#if defined(ASIO_WINDOWS_RUNTIME)
# include "asio/detail/winrt_utils.hpp"
#endif // defined(ASIO_WINDOWS_RUNTIME)

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

/// An iterator over the entries produced by a resolver.
/**
 * The asio::ip::basic_resolver_iterator class template is used to define
 * iterators over the results returned by a resolver.
 * 【定义了解析器返回的结果上面的迭代器】
 *
 * The iterator's value_type, obtained when the iterator is dereferenced, is:
 * @code const basic_resolver_entry<InternetProtocol> @endcode
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
template <typename InternetProtocol>
class basic_resolver_iterator
{
public:
  /// The type used for the distance between two iterators.【int64 我不要跑32位啊】
  typedef std::ptrdiff_t difference_type;  

  /// The type of the value pointed to by the iterator. 【值类型】
  typedef basic_resolver_entry<InternetProtocol> value_type;

  /// The type of the result of applying operator->() to the iterator.  【指针类型】
  typedef const basic_resolver_entry<InternetProtocol>* pointer;

  /// The type of the result of applying operator*() to the iterator.   【引用类型】
  typedef const basic_resolver_entry<InternetProtocol>& reference;

  /// The iterator category. 【迭代器类型】
  typedef std::forward_iterator_tag iterator_category;

  /// Default constructor creates an end iterator. 【默认构造函数，index=>0】
  basic_resolver_iterator()
    : index_(0)
  {
  }

  /// Copy constructor. 【拷贝构造，所以下面values_用的shared_ptr】
  basic_resolver_iterator(const basic_resolver_iterator& other)
    : values_(other.values_),
      index_(other.index_)
  {
  }

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move constructor.
  basic_resolver_iterator(basic_resolver_iterator&& other)
    : values_(ASIO_MOVE_CAST(values_ptr_type)(other.values_)),  //得用static_cast<type&&>
      index_(other.index_)  //built-in不用move
  {
    other.index_ = 0;
  }
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Assignment operator.
  basic_resolver_iterator& operator=(const basic_resolver_iterator& other)
  {
    values_ = other.values_;
    index_ = other.index_;
    return *this;
  }

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move-assignment operator.
  basic_resolver_iterator& operator=(basic_resolver_iterator&& other)
  {
    if (this != &other)
    {
      values_ = ASIO_MOVE_CAST(values_ptr_type)(other.values_);
      index_ = other.index_;
      other.index_ = 0; //移动构造后要将另外的清空
    }

    return *this;
  }
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Dereference an iterator.  【解引用，返回下面元素引用】
  const basic_resolver_entry<InternetProtocol>& operator*() const
  {
    return dereference();   
  }

  /// Dereference an iterator. 【解引用，返回下面元素地址】
  const basic_resolver_entry<InternetProtocol>* operator->() const
  {
    return &dereference();
  }

  /// Increment operator (prefix).
  basic_resolver_iterator& operator++()
  {
    increment();
    return *this;
  }

  /// Increment operator (postfix).
  basic_resolver_iterator operator++(int)
  {
    basic_resolver_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  /// Test two iterators for equality.
  friend bool operator==(const basic_resolver_iterator& a,
      const basic_resolver_iterator& b)
  {
    return a.equal(b);
  }

  /// Test two iterators for inequality.
  friend bool operator!=(const basic_resolver_iterator& a,
      const basic_resolver_iterator& b)
  {
    return !a.equal(b);
  }

protected:
    //计数器加1，要是到了结尾，就重置对vector的引用，并将index_重置为0
  void increment()
  {
    if (++index_ == values_->size())
    {
      // Reset state to match a default constructed end iterator.
      values_.reset();
      index_ = 0;
    }
  }

  bool equal(const basic_resolver_iterator& other) const
  {
    if (!values_ && !other.values_) 
      return true;  //都为nullptr相等
    if (values_ != other.values_)
      return false; //一个为nullptr，不等
    return index_ == other.index_;  //判断是不是index_相同
  }

  const basic_resolver_entry<InternetProtocol>& dereference() const
  {
    return (*values_)[index_];  //返回下面的向量元素引用
  }

  typedef std::vector<basic_resolver_entry<InternetProtocol> > values_type; //定义下面的向量值类型
  typedef asio::detail::shared_ptr<values_type> values_ptr_type;    //对这个向量的指针
  values_ptr_type values_;  //这个向量指针
  std::size_t index_;   //index
};

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_BASIC_RESOLVER_ITERATOR_HPP
