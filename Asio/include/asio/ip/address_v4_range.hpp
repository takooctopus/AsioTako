//
// TAKO: ipv4�ĵ�ַ��Χ
// ip/address_v4_range.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_ADDRESS_V4_RANGE_HPP
#define ASIO_IP_ADDRESS_V4_RANGE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/ip/address_v4_iterator.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace ip {

template <typename> class basic_address_range;

/// Represents a range of IPv4 addresses.����
/**
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
template <> class basic_address_range<address_v4>
{
public:
  /// The type of an iterator that points into the range.
  typedef basic_address_iterator<address_v4> iterator;

  /// Construct an empty range. ���չ���0-0��
  basic_address_range() ASIO_NOEXCEPT
    : begin_(address_v4()),
      end_(address_v4())
  {
  }

  /// Construct an range that represents the given range of addresses.��������iter��
  explicit basic_address_range(const iterator& first,
      const iterator& last) ASIO_NOEXCEPT
    : begin_(first),
      end_(last)
  {
  }

  /// Copy constructor.
  basic_address_range(const basic_address_range& other) ASIO_NOEXCEPT
    : begin_(other.begin_),
      end_(other.end_)
  {
  }

#if defined(ASIO_HAS_MOVE)
  /// Move constructor.���ƶ�������Ҳ��ֱ��ʹ��static_cast<type&&>���У���Ϊ���涼û�з���ѡ�
  basic_address_range(basic_address_range&& other) ASIO_NOEXCEPT
    : begin_(ASIO_MOVE_CAST(iterator)(other.begin_)),
      end_(ASIO_MOVE_CAST(iterator)(other.end_))
  {
  }
#endif // defined(ASIO_HAS_MOVE)

  /// Assignment operator.
  basic_address_range& operator=(
      const basic_address_range& other) ASIO_NOEXCEPT
  {
    begin_ = other.begin_;
    end_ = other.end_;
    return *this;
  }

#if defined(ASIO_HAS_MOVE)
  /// Move assignment operator.
  basic_address_range& operator=(
      basic_address_range&& other) ASIO_NOEXCEPT
  {
    begin_ = ASIO_MOVE_CAST(iterator)(other.begin_);
    end_ = ASIO_MOVE_CAST(iterator)(other.end_);
    return *this;
  }
#endif // defined(ASIO_HAS_MOVE)

  /// Obtain an iterator that points to the start of the range.��begin������ָ�롿
  iterator begin() const ASIO_NOEXCEPT
  {
    return begin_;
  }

  /// Obtain an iterator that points to the end of the range. ��end����βָ�롿
  iterator end() const ASIO_NOEXCEPT
  {
    return end_;
  }

  /// Determine whether the range is empty. ��sizeΪ0��
  bool empty() const ASIO_NOEXCEPT
  {
    return size() == 0;
  }

  /// Return the size of the range. ������size_t,u64/u32��
  std::size_t size() const ASIO_NOEXCEPT
  {
    return end_->to_uint() - begin_->to_uint(); //��Ȼipv4�����ֻ֧��u32��������󷵻ص���u32/u64
  }

  /// Find an address in the range.���Ƚ����½磬������ͷ��أ�û�оͷ���end_��
  iterator find(const address_v4& addr) const ASIO_NOEXCEPT
  {
    return addr >= *begin_ && addr < *end_ ? iterator(addr) : end_;
  }

private:
  iterator begin_;
  iterator end_;
};

/// Represents a range of IPv4 addresses.
typedef basic_address_range<address_v4> address_v4_range;

} // namespace ip
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_ADDRESS_V4_RANGE_HPP
