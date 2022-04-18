//
// detail/win_thread.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_WIN_THREAD_HPP
#define ASIO_DETAIL_WIN_THREAD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_WINDOWS) \
  && !defined(ASIO_WINDOWS_APP) \
  && !defined(UNDER_CE)

#include <cstddef>
#include "asio/detail/noncopyable.hpp"
#include "asio/detail/socket_types.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

ASIO_DECL unsigned int __stdcall win_thread_function(void* arg);

#if defined(WINVER) && (WINVER < 0x0500)
ASIO_DECL void __stdcall apc_function(ULONG data);  //传入32bit地址
#else
ASIO_DECL void __stdcall apc_function(ULONG_PTR data);  //传入64bit地址
#endif

template <typename T>
class win_thread_base
{
public:
  static bool terminate_threads()
  {
    return ::InterlockedExchangeAdd(&terminate_threads_, 0) != 0; //原子+0，要是不为0就返回true
  }

  static void set_terminate_threads(bool b)
  {
    ::InterlockedExchange(&terminate_threads_, b ? 1 : 0); //设定是否退出当前线程【要么为1，要么为0】
  }

private:
  static long terminate_threads_;   //一般来说，只要判断的时候为0，就不退出
};

template <typename T>
long win_thread_base<T>::terminate_threads_ = 0;

class win_thread
  : private noncopyable,
    public win_thread_base<win_thread>
{
public:
  // Constructor.
  template <typename Function>
  win_thread(Function f, unsigned int stack_size = 0)
    : thread_(0),
      exit_event_(0)
  {
    start_thread(new func<Function>(f), stack_size);
  }

  // Destructor.
  ASIO_DECL ~win_thread();

  // Wait for the thread to exit.
  ASIO_DECL void join();

  // Get number of CPUs.
  ASIO_DECL static std::size_t hardware_concurrency();

private:
  friend ASIO_DECL unsigned int __stdcall win_thread_function(void* arg);

#if defined(WINVER) && (WINVER < 0x0500)
  friend ASIO_DECL void __stdcall apc_function(ULONG);
#else
  friend ASIO_DECL void __stdcall apc_function(ULONG_PTR);
#endif

  class func_base
  {
  public:
    virtual ~func_base() {}
    virtual void run() = 0;
    ::HANDLE entry_event_;  //对于每个方法类，都有一个这个方法的进入和退出事件
    ::HANDLE exit_event_;
  };

  struct auto_func_base_ptr
  {
    func_base* ptr; //同样的，这个指针结构只有一个基本指针，之所以多这么一层是借由这个的析构函数来自动调用下面的堆对象的析构函数
    ~auto_func_base_ptr() { delete ptr; } //删除时只是把指向对象里面的entry_event_和exit_event_这两个地址删除了，但这两个地址指向的event是并没有删除的
  };

  template <typename Function>
  class func
    : public func_base
  {
  public:
    func(Function f)
      : f_(f)
    {
    }

    virtual void run()
    {
      f_();
    }

  private:
    Function f_;
  };

  ASIO_DECL void start_thread(func_base* arg, unsigned int stack_size);

  ::HANDLE thread_;
  ::HANDLE exit_event_; //thread的退出事件
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "asio/detail/impl/win_thread.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // defined(ASIO_WINDOWS)
       // && !defined(ASIO_WINDOWS_APP)
       // && !defined(UNDER_CE)

#endif // ASIO_DETAIL_WIN_THREAD_HPP
