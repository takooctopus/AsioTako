//
// TAKO: posix_event�ࡾ�����������ľ��庯��ʵ��
// detail/impl/posix_event.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IMPL_POSIX_EVENT_IPP
#define ASIO_DETAIL_IMPL_POSIX_EVENT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_PTHREADS)

#include "asio/detail/posix_event.hpp"
#include "asio/detail/throw_error.hpp"
#include "asio/error.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

posix_event::posix_event()
  : state_(0)   //��ʼ����������ʼ����ǰ״̬Ϊ0��
{
#if (defined(__MACH__) && defined(__APPLE__)) \
      || (defined(__ANDROID__) && (__ANDROID_API__ < 21))
  int error = ::pthread_cond_init(&cond_, 0);   //��Щarch��ƽ̨
#else // (defined(__MACH__) && defined(__APPLE__))
      // || (defined(__ANDROID__) && (__ANDROID_API__ < 21))
  ::pthread_condattr_t attr;    //�����������Խṹ��
  int error = ::pthread_condattr_init(&attr);   //Ȼ���ʼ����Ϣ�ṹ��
  if (error == 0)
  {
    error = ::pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);    //CLOCK_MONOTONIC => ϵͳ���������ڵ�ʱ��
    if (error == 0)
      error = ::pthread_cond_init(&cond_, &attr);   //��ʼ���������� ��int pthread_cond_init(pthread_cond_t *cv, const pthread_condattr_t *cattr); �������������ʱ����������������ڲ���cond_ָ����ڴ��� �������ɶ���߳�ͬʱ��ʼ��һ����������������Ҫ���³�ʼ�����ͷ�һ����������ʱ��Ӧ�ó�����뱣֤�����������δ��ʹ�á�
    ::pthread_condattr_destroy(&attr);  //��ʼ��������ǵ�����pthread_condattr_t������ʵ��
  }
#endif // (defined(__MACH__) && defined(__APPLE__))
       // || (defined(__ANDROID__) && (__ANDROID_API__ < 21))

  asio::error_code ec(error,
      asio::error::get_system_category());
  asio::detail::throw_error(ec, "event");
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_PTHREADS)

#endif // ASIO_DETAIL_IMPL_POSIX_EVENT_IPP
