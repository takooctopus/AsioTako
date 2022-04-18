//
// TAKO: windows平台下互斥锁的引入
// detail/win_mutex.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_WIN_MUTEX_HPP
#define ASIO_DETAIL_WIN_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_WINDOWS)

#include "asio/detail/noncopyable.hpp"  //不可拷贝
#include "asio/detail/scoped_lock.hpp"  //管理生命周期的指针
#include "asio/detail/socket_types.hpp" //套接字一般的常量

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class win_mutex
  : private noncopyable
{
public:
  typedef asio::detail::scoped_lock<win_mutex> scoped_lock; //定义互斥锁[scoped_lock]

  // Constructor.
  ASIO_DECL win_mutex();

  // Destructor.
  ~win_mutex()
  {
    ::DeleteCriticalSection(&crit_section_);  //删除临界区段
  }

  // Lock the mutex.
  void lock()
  {
    ::EnterCriticalSection(&crit_section_); //进入临界区段
  }

  // Unlock the mutex.
  void unlock()
  {
    ::LeaveCriticalSection(&crit_section_); //离开临界区段
  }

private:
  // Initialisation must be performed in a separate function to the constructor
  // since the compiler does not support the use of structured exceptions and
  // C++ exceptions in the same function.
  // 【初始化得分到两个函数中，因为编译器不支持同一个函数同时使用结构化异常和c++异常】
  ASIO_DECL int do_init();

  ::CRITICAL_SECTION crit_section_; //临界区段结构体对象
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "asio/detail/impl/win_mutex.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // defined(ASIO_WINDOWS)

#endif // ASIO_DETAIL_WIN_MUTEX_HPP
