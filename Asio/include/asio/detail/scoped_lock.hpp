//
// TAKO: scoped_lcok �Զ����ͽ����������İ�����
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
template <typename Mutex>   //��std::lock_guard��࣬ģ��Ҫ��һ��mutex��ȥ
class scoped_lock
  : private noncopyable //�Ҿ��ú�lock_guard��ࡾ���忴һ����֪���ǲ����ˡ�
{
public:
  // Tag type used to distinguish constructors. ����֪�����ô���
  enum adopt_lock_t { adopt_lock };

  // Constructor adopts a lock that is already held. ��[ʹ��һ����lock��mutex��ʼ��]һ����ʼ�������mutex_������locked_ֵ��Ϊ�桿
  scoped_lock(Mutex& m, adopt_lock_t)
    : mutex_(m),
      locked_(true)
  {
  }

  // Constructor acquires the lock. ����ʽ����[˵���ˣ��϶����а�]����ʹ�ô���G��mutex��ʼ���Լ���mutex_����������
  explicit scoped_lock(Mutex& m)
    : mutex_(m)
  {
    mutex_.lock();
    locked_ = true;
  }

  // Destructor releases the lock.������ʱ���Զ����������lock��
  ~scoped_lock()
  {
    if (locked_)
      mutex_.unlock();
  }

  // Explicitly acquire the lock. ����ʽ�ض����滥����������
  void lock()
  {
    if (!locked_)
    {
      mutex_.lock();
      locked_ = true;
    }
  }

  // Explicitly release the lock. ����ʽ�ض����滥����������
  void unlock()
  {
    if (locked_)
    {
      mutex_.unlock();
      locked_ = false;
    }
  }

  // Test whether the lock is held. �����س��л�����������״̬��
  bool locked() const
  {
    return locked_;
  }

  // Get the underlying mutex. �����س��еĻ��������á�
  Mutex& mutex()
  {
    return mutex_;
  }

private:
  // The underlying mutex. �����еĻ�������
  Mutex& mutex_;

  // Whether the mutex is currently locked or unlocked. �����滥������״̬��
  bool locked_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_SCOPED_LOCK_HPP
