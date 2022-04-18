//
// TAKO:posix的线程实现
// detail/impl/posix_thread.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IMPL_POSIX_THREAD_IPP
#define ASIO_DETAIL_IMPL_POSIX_THREAD_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_PTHREADS)

#include "asio/detail/posix_thread.hpp"
#include "asio/detail/throw_error.hpp"
#include "asio/error.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

posix_thread::~posix_thread()
{
  if (!joined_)
    ::pthread_detach(thread_); // 析构，要是没有join就得detatch
}

void posix_thread::join()
{
  if (!joined_) //只能join一次
  {
    ::pthread_join(thread_, 0); 
    joined_ = true;     // join后改变标志
  }
}

std::size_t posix_thread::hardware_concurrency()
{
#if defined(_SC_NPROCESSORS_ONLN)
  long result = sysconf(_SC_NPROCESSORS_ONLN);
  if (result > 0)
    return result;
#endif // defined(_SC_NPROCESSORS_ONLN)
  return 0;
}

void posix_thread::start_thread(func_base* arg)
{
  int error = ::pthread_create(&thread_, 0,
        asio_detail_posix_thread_function, arg);    //获取错误返回值
  if (error != 0)
  {
    delete arg;
    asio::error_code ec(error,
        asio::error::get_system_category());
    asio::detail::throw_error(ec, "thread");    //根据错误抛出异常
  }
}

void* asio_detail_posix_thread_function(void* arg)  //传入的应该是指针void*[32/64bit]
{
  posix_thread::auto_func_base_ptr func = {
      static_cast<posix_thread::func_base*>(arg) }; //回去看auto_func_base_ptr这个结构体，里面实际上只有一个数据结构func_base*，因此可以强转后直接赋值。 而这里的void* arg应该是传入的一个指针地址吧，我们只是指明了这个地址指向的数据实例应该继承了func_base接口。【这种还是很有趣的】
  func.ptr->run();
  return 0;
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_PTHREADS)

#endif // ASIO_DETAIL_IMPL_POSIX_THREAD_IPP
