//
// TAKO: ��׼�¼���
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

#include <chrono>                               //ʱ��
#include <condition_variable>                   //��������
#include "asio/detail/assert.hpp"               //����
#include "asio/detail/noncopyable.hpp"          //���ɿ���

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

/// =================================================================================================================================================
//��׼�¼�
class std_event
  : private noncopyable
{
public:
  // Constructor.��Ĭ�Ϲ���������״̬��Ϊ0��
  std_event()
    : state_(0)
  {
  }

  // Destructor.
  ~std_event()
  {
  }

  // Signal the event. (Retained for backward compatibility.)�������������ѡ�
  template <typename Lock>
  void signal(Lock& lock)
  {
    this->signal_all(lock);
  }

  // Signal all waiters.
  template <typename Lock>
  void signal_all(Lock& lock)
  {
    ASIO_ASSERT(lock.locked()); //��֤�������ϵ�
    (void)lock; //���߱����������ǵ�lock�Ѿ�������
    state_ |= 1;    // ״̬�� |= 1
    cond_.notify_all(); //����������֪ͨ���еĵȴ���
  }

  // Unlock the mutex and signal one waiter. ��������֪ͨһ����
  template <typename Lock>
  void unlock_and_signal_one(Lock& lock)    
  {
    ASIO_ASSERT(lock.locked());     //��ʼǰ�������ŵ�
    state_ |= 1;                    //״̬�� |= 1
    bool have_waiters = (state_ > 1);   // ������û�еȴ���
    lock.unlock();                  //�Ƚ���
    if (have_waiters)
      cond_.notify_one();           //�еȴ��ߣ���֪ͨ
  }

  // Unlock the mutex and signal one waiter who may destroy us.��������֪ͨһ�����ܻ��������ǵĵȴ��ߡ�
  template <typename Lock>
  void unlock_and_signal_one_for_destruction(Lock& lock)
  {
    ASIO_ASSERT(lock.locked());     //��֤�������ŵ�
    state_ |= 1;
    bool have_waiters = (state_ > 1);
    if (have_waiters)
      cond_.notify_one();
    lock.unlock();
  }

  // If there's a waiter, unlock the mutex and signal it. ������еȴ��ߣ�������֪ͨ��
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

  // Reset the event.   �������¼���
  template <typename Lock>
  void clear(Lock& lock)
  {
    ASIO_ASSERT(lock.locked());
    (void)lock;
    state_ &= ~std::size_t(1);  //ֻ�������һλΪ0����δ�����׶Ρ�
  }

  // Wait for the event to become signalled.���������¼���֪ͨ��
  template <typename Lock>
  void wait(Lock& lock)
  {
    ASIO_ASSERT(lock.locked());
    unique_lock_adapter u_lock(lock);   //һ���ӿڣ���װ��unique_lock�������ڳ�������ʱ�Զ��ͷ���[������scoped_lock]
    while ((state_ & 1) == 0)       //���ȱ�֤�¼�û�б�����
    {
      waiter w(state_);             //����һ���ȴ����� state_ += 2
      cond_.wait(u_lock.unique_lock_);  //ʹ�������������������ȴ�
    }
  }

  // Timed wait for the event to become signalled.����ʱ��������
  template <typename Lock>
  bool wait_for_usec(Lock& lock, long usec)
  {
    ASIO_ASSERT(lock.locked());     //�ȱ�֤�Ѿ�������
    unique_lock_adapter u_lock(lock);   //scoped_lock ת���� unique_lock
    if ((state_ & 1) == 0)
    {
      waiter w(state_);
      cond_.wait_for(u_lock.unique_lock_, std::chrono::microseconds(usec)); //�������������������ȴ�����
    }
    return (state_ & 1) != 0;
  }

private:
  /// ==============================================================================================
  // Helper class to temporarily adapt a scoped_lock into a unique_lock so that
  // it can be passed to std::condition_variable::wait().
  // �����ࣺ��ʱ�Եؽ�scope_lock���õ�unique_lock��������Ա���������ʹ��[��Ϊ���Ƕ����scoped_lock���Ǳ�׼��������������������wait�ô���һ��std����]
  struct unique_lock_adapter
  {
    template <typename Lock>
    explicit unique_lock_adapter(Lock& lock)
      : unique_lock_(lock.mutex().mutex_, std::adopt_lock)  //ʹ��std::adopt_lock��ʾmutex_�ڴ���ʱ���Ѿ�������
    {
    }

    ~unique_lock_adapter()
    {
      unique_lock_.release();
    }

    std::unique_lock<std::mutex> unique_lock_;      //ʹ��unique_lock���ж�mutex_
  };

  /// ==============================================================================================
  // Helper to increment and decrement the state to track outstanding waiters.
  // ÿ��ʼ��һ��waiter��Ķ��󣬶��������õ�state_����[���ϲ����״̬��]�ı� +2/-2
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

  std::condition_variable cond_;        //һ����������
  std::size_t state_;                   //u64��״̬�롾ÿ��һ���ȴ��ߣ�state_ + 2��
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_STD_MUTEX_AND_CONDVAR)

#endif // ASIO_DETAIL_STD_EVENT_HPP
