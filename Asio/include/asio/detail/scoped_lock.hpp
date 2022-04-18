//
// TAKO: scoped_lcok 自动锁和解锁互斥量的帮助类
// detail/scoped_lock.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_SCOPED_LOCK_HPP
#define ASIO_DETAIL_SCOPED_LOCK_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/noncopyable.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

// Helper class to lock and unlock a mutex automatically.
template <typename Mutex>   //和std::lock_guard差不多，模板要传一个mutex进去
class scoped_lock
  : private noncopyable //我觉得和lock_guard差不多【具体看一看就知道是不是了】
{
public:
  // Tag type used to distinguish constructors. 【不知道的用处】
  enum adopt_lock_t { adopt_lock };

  // Constructor adopts a lock that is already held. 【[使用一个已lock的mutex初始化]一并初始化下面的mutex_，并将locked_值设为真】
  scoped_lock(Mutex& m, adopt_lock_t)
    : mutex_(m),
      locked_(true)
  {
  }

  // Constructor acquires the lock. 【显式构造[说嘛了，肯定得有啊]】【使用传入欸到mutex初始化自己的mutex_，并加锁】
  explicit scoped_lock(Mutex& m)
    : mutex_(m)
  {
    mutex_.lock();
    locked_ = true;
  }

  // Destructor releases the lock.【析构时会自动解锁下面的lock】
  ~scoped_lock()
  {
    if (locked_)
      mutex_.unlock();
  }

  // Explicitly acquire the lock. 【显式地对下面互斥锁上锁】
  void lock()
  {
    if (!locked_)
    {
      mutex_.lock();
      locked_ = true;
    }
  }

  // Explicitly release the lock. 【显式地对下面互斥锁解锁】
  void unlock()
  {
    if (locked_)
    {
      mutex_.unlock();
      locked_ = false;
    }
  }

  // Test whether the lock is held. 【返回持有互斥锁地上锁状态】
  bool locked() const
  {
    return locked_;
  }

  // Get the underlying mutex. 【返回持有的互斥锁引用】
  Mutex& mutex()
  {
    return mutex_;
  }

private:
  // The underlying mutex. 【持有的互斥锁】
  Mutex& mutex_;

  // Whether the mutex is currently locked or unlocked. 【下面互斥锁的状态】
  bool locked_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_SCOPED_LOCK_HPP
