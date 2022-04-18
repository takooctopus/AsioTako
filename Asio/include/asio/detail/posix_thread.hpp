//
// detail/posix_thread.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_POSIX_THREAD_HPP
#define ASIO_DETAIL_POSIX_THREAD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_PTHREADS)

#include <cstddef>
#include <pthread.h>
#include "asio/detail/noncopyable.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

extern "C"
{
  ASIO_DECL void* asio_detail_posix_thread_function(void* arg); //先制声明
}

class posix_thread
  : private noncopyable
{
public:
  // Constructor.
  template <typename Function>
  posix_thread(Function f, unsigned int = 0)
    : joined_(false)
  {
    start_thread(new func<Function>(f));    //posix的thread方法，传入一个函数指针
  }

  // Destructor.[inline]
  ASIO_DECL ~posix_thread();

  // Wait for the thread to exit.[inline]
  ASIO_DECL void join();

  // Get number of CPUs.
  ASIO_DECL static std::size_t hardware_concurrency();

private:
  friend void* asio_detail_posix_thread_function(void* arg);

  class func_base   // 私有类，函数基本抽象类
  {
  public:
    virtual ~func_base() {} //虚函数【析构】
    virtual void run() = 0; //虚函数【运行】
  };

  struct auto_func_base_ptr //私有结构
  {
    func_base* ptr; //一个指向func_base的指针
    ~auto_func_base_ptr() { delete ptr; }   //析构时释放保有的函数基本类
  };

  template <typename Function>
  class func  
    : public func_base  //需要实现func_base的两个接口函数
  {
  public:
    func(Function f)    
      : f_(f)   // 传入一个Function，并初始化私有变量f_
    {
    }

    virtual void run()
    {
      f_();
    }

  private:
    Function f_;    
  };

  ASIO_DECL void start_thread(func_base* arg);  // 开始线程

  ::pthread_t thread_;  //保有poxisx线程实例
  bool joined_; //当前线程是否join
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "asio/detail/impl/posix_thread.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // defined(ASIO_HAS_PTHREADS)

#endif // ASIO_DETAIL_POSIX_THREAD_HPP
