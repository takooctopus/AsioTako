//
// TAKO: 标准事件类
// detail/std_event.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_STD_EVENT_HPP
#define ASIO_DETAIL_STD_EVENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_STD_MUTEX_AND_CONDVAR)

#include <chrono>                               //时钟
#include <condition_variable>                   //条件变量
#include "asio/detail/assert.hpp"               //断言
#include "asio/detail/noncopyable.hpp"          //不可拷贝

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

/// =================================================================================================================================================
//标准事件
class std_event
  : private noncopyable
{
public:
  // Constructor.【默认构建函数，状态码为0】
  std_event()
    : state_(0)
  {
  }

  // Destructor.
  ~std_event()
  {
  }

  // Signal the event. (Retained for backward compatibility.)【传入锁，唤醒】
  template <typename Lock>
  void signal(Lock& lock)
  {
    this->signal_all(lock);
  }

  // Signal all waiters.
  template <typename Lock>
  void signal_all(Lock& lock)
  {
    ASIO_ASSERT(lock.locked()); //保证锁是锁上的
    (void)lock; //告诉编译器，我们的lock已经用上了
    state_ |= 1;    // 状态码 |= 1
    cond_.notify_all(); //让条件变量通知所有的等待者
  }

  // Unlock the mutex and signal one waiter. 【解锁并通知一个】
  template <typename Lock>
  void unlock_and_signal_one(Lock& lock)    
  {
    ASIO_ASSERT(lock.locked());     //开始前得是锁着的
    state_ |= 1;                    //状态码 |= 1
    bool have_waiters = (state_ > 1);   // 看看有没有等待者
    lock.unlock();                  //先解锁
    if (have_waiters)
      cond_.notify_one();           //有等待者，就通知
  }

  // Unlock the mutex and signal one waiter who may destroy us.【解锁并通知一个可能会销毁我们的等待者】
  template <typename Lock>
  void unlock_and_signal_one_for_destruction(Lock& lock)
  {
    ASIO_ASSERT(lock.locked());     //保证是上锁着的
    state_ |= 1;
    bool have_waiters = (state_ > 1);
    if (have_waiters)
      cond_.notify_one();
    lock.unlock();
  }

  // If there's a waiter, unlock the mutex and signal it. 【如果有等待者，解锁并通知】
  template <typename Lock>
  bool maybe_unlock_and_signal_one(Lock& lock)
  {
    ASIO_ASSERT(lock.locked());
    state_ |= 1;
    if (state_ > 1)
    {
      lock.unlock();
      cond_.notify_one();
      return true;
    }
    return false;
  }

  // Reset the event.   【重置事件】
  template <typename Lock>
  void clear(Lock& lock)
  {
    ASIO_ASSERT(lock.locked());
    (void)lock;
    state_ &= ~std::size_t(1);  //只重置最后一位为0【到未触发阶段】
  }

  // Wait for the event to become signalled.【阻塞到事件被通知】
  template <typename Lock>
  void wait(Lock& lock)
  {
    ASIO_ASSERT(lock.locked());
    unique_lock_adapter u_lock(lock);   //一个接口，封装了unique_lock，可以在出作用域时自动释放锁[这里是scoped_lock]
    while ((state_ & 1) == 0)       //首先保证事件没有被触发
    {
      waiter w(state_);             //创建一个等待对象， state_ += 2
      cond_.wait(u_lock.unique_lock_);  //使用条件变量进行阻塞等待
    }
  }

  // Timed wait for the event to become signalled.【定时的阻塞】
  template <typename Lock>
  bool wait_for_usec(Lock& lock, long usec)
  {
    ASIO_ASSERT(lock.locked());     //先保证已经上锁了
    unique_lock_adapter u_lock(lock);   //scoped_lock 转换到 unique_lock
    if ((state_ & 1) == 0)
    {
      waiter w(state_);
      cond_.wait_for(u_lock.unique_lock_, std::chrono::microseconds(usec)); //调用条件变量的阻塞等待函数
    }
    return (state_ & 1) != 0;
  }

private:
  /// ==============================================================================================
  // Helper class to temporarily adapt a scoped_lock into a unique_lock so that
  // it can be passed to std::condition_variable::wait().
  // 帮助类：暂时性地将scope_lock适用到unique_lock，让其可以被条件变量使用[因为我们定义的scoped_lock并非标准库的锁，而条件变量里的wait得传入一个std的锁]
  struct unique_lock_adapter
  {
    template <typename Lock>
    explicit unique_lock_adapter(Lock& lock)
      : unique_lock_(lock.mutex().mutex_, std::adopt_lock)  //使用std::adopt_lock表示mutex_在传入时是已经上锁的
    {
    }

    ~unique_lock_adapter()
    {
      unique_lock_.release();
    }

    std::unique_lock<std::mutex> unique_lock_;      //使用unique_lock进行对mutex_
  };

  /// ==============================================================================================
  // Helper to increment and decrement the state to track outstanding waiters.
  // 每初始化一个waiter类的对象，都会让引用的state_对象[即上层类的状态码]改变 +2/-2
  class waiter
  {
  public:
    explicit waiter(std::size_t& state)
      : state_(state)
    {
      state_ += 2;
    }

    ~waiter()
    {
      state_ -= 2;
    }

  private:
    std::size_t& state_;
  };

  std::condition_variable cond_;        //一个条件变量
  std::size_t state_;                   //u64的状态码【每多一个等待者，state_ + 2】
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_STD_MUTEX_AND_CONDVAR)

#endif // ASIO_DETAIL_STD_EVENT_HPP
