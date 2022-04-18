//
// TAKO：posix条件变量的阻塞事件
// detail/posix_event.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_POSIX_EVENT_HPP
#define ASIO_DETAIL_POSIX_EVENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_PTHREADS)

#include <cstddef>
#include <pthread.h>
#include "asio/detail/assert.hpp"
#include "asio/detail/noncopyable.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class posix_event
  : private noncopyable //作为条件变量，不能拷贝
{
public:
  // Constructor.
  ASIO_DECL posix_event();

  // Destructor.
  ~posix_event()
  {
    ::pthread_cond_destroy(&cond_); //销毁调用pthread_cond_destroy
  }

  // Signal the event. (Retained for backward compatibility.) 
  template <typename Lock>  //【模板Lock应该就是scoped_lock类了】
  void signal(Lock& lock)
  {
    this->signal_all(lock); //向所有阻塞的发送消息
  }

  // Signal all waiters.
  template <typename Lock>
  void signal_all(Lock& lock)
  {
    ASIO_ASSERT(lock.locked()); //断言传入的scoped_lock是已经上锁了的
    (void)lock; //告诉编译器这个变量已经使用了
    state_ |= 1;    //将状态u64 => 加上1
    ::pthread_cond_broadcast(&cond_); // Ignore EINVAL. 【使用pthread_cond_broadcast唤醒所有的线程】
  }

  // Unlock the mutex and signal one waiter. 【解锁并唤醒一个】
  template <typename Lock>
  void unlock_and_signal_one(Lock& lock)
  {
    ASIO_ASSERT(lock.locked());
    state_ |= 1;
    bool have_waiters = (state_ > 1); //要是除了最低位还有flag，那么等待的数量有很多
    lock.unlock();
    if (have_waiters)
      ::pthread_cond_signal(&cond_); // Ignore EINVAL.  【使用pthread_cond_singal唤醒一个】
  }

  // Unlock the mutex and signal one waiter who may destroy us.
  template <typename Lock>
  void unlock_and_signal_one_for_destruction(Lock& lock)
  {
    ASIO_ASSERT(lock.locked());
    state_ |= 1;
    bool have_waiters = (state_ > 1);
    if (have_waiters)
      ::pthread_cond_signal(&cond_); // Ignore EINVAL.
    lock.unlock();  //【因为等待的可能会摧毁调用的scoped_lock，所以先唤醒，再解锁】【这个unlock()是条件的，没锁上是不会再解锁的】
  }

  // If there's a waiter, unlock the mutex and signal it.
  template <typename Lock>
  bool maybe_unlock_and_signal_one(Lock& lock)
  {
    ASIO_ASSERT(lock.locked());
    state_ |= 1;
    if (state_ > 1) // 【有等待者才解锁】
    {
      lock.unlock();
      ::pthread_cond_signal(&cond_); // Ignore EINVAL.
      return true;
    }
    return false;
  }

  // Reset the event.
  template <typename Lock>
  void clear(Lock& lock)
  {
    ASIO_ASSERT(lock.locked());
    (void)lock;
    state_ &= ~std::size_t(1);  //只将state_最低位置为0
  }

  // Wait for the event to become signalled. 【等待信号传入】
  template <typename Lock>
  void wait(Lock& lock)
  {
    ASIO_ASSERT(lock.locked());
    while ((state_ & 1) == 0)
    {
      state_ += 2; //每多一个等待者 state_+2
      ::pthread_cond_wait(&cond_, &lock.mutex().mutex_); // Ignore EINVAL. 【调用pthread_cond_wait(&cond, &mutex)进行阻塞】
      state_ -= 2; //阻塞结束后，state_-2
    }
  }

  // Timed wait for the event to become signalled. 【等待多少微秒】
  template <typename Lock>
  bool wait_for_usec(Lock& lock, long usec) //传入一个long
  {
    ASIO_ASSERT(lock.locked());
    if ((state_ & 1) == 0)  //当前要是没有等待
    {
      state_ += 2;  //那么当前要等待
      timespec ts;
#if (defined(__MACH__) && defined(__APPLE__)) \
      || (defined(__ANDROID__) && (__ANDROID_API__ < 21) \
          && defined(HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE))
      ts.tv_sec = usec / 1000000;   //秒数
      ts.tv_nsec = (usec % 1000000) * 1000; //纳秒数
      ::pthread_cond_timedwait_relative_np(
          &cond_, &lock.mutex().mutex_, &ts); // Ignore EINVAL. 【调用pthread_cond_timedwait_relative_np(&cond, &mutex, &ts)进行时间阻塞】
#else // (defined(__MACH__) && defined(__APPLE__))
      // || (defined(__ANDROID__) && (__ANDROID_API__ < 21)
      //     && defined(HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE))
      if (::clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
      {
        ts.tv_sec += usec / 1000000;
        ts.tv_nsec += (usec % 1000000) * 1000;
        ts.tv_sec += ts.tv_nsec / 1000000000;
        ts.tv_nsec = ts.tv_nsec % 1000000000;
        ::pthread_cond_timedwait(&cond_,
            &lock.mutex().mutex_, &ts); // Ignore EINVAL. 【非arch平台下的】
      }
#endif // (defined(__MACH__) && defined(__APPLE__))
       // || (defined(__ANDROID__) && (__ANDROID_API__ < 21)
       //     && defined(HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE))
      state_ -= 2;
    }
    return (state_ & 1) != 0;   //返回当前有没有锁， 有锁为true
  }

private:
  ::pthread_cond_t cond_;   //一个结构体【用以阻塞的条件变量】
  std::size_t state_;   //【u64】 当前状态
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "asio/detail/impl/posix_event.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // defined(ASIO_HAS_PTHREADS)

#endif // ASIO_DETAIL_POSIX_EVENT_HPP
