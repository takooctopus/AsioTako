//
// TAKO:可调用
// execution/detail/as_invocable.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_DETAIL_AS_INVOCABLE_HPP
#define ASIO_EXECUTION_DETAIL_AS_INVOCABLE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/atomic_count.hpp" //原子操作
#include "asio/detail/memory.hpp"   //内存
#include "asio/detail/type_traits.hpp"  //类型萃取
#include "asio/execution/receiver_invocation_error.hpp" //调用错误
#include "asio/execution/set_done.hpp"
#include "asio/execution/set_error.hpp"
#include "asio/execution/set_value.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace execution {
namespace detail {

#if defined(ASIO_HAS_MOVE)

///===================================================================
// 可以被唤醒
template <typename Receiver, typename>
struct as_invocable
{
  Receiver* receiver_;  //接收者指针

  explicit as_invocable(Receiver& r) ASIO_NOEXCEPT
    : receiver_(asio::detail::addressof(r))     //初始化接收者[用地址]
  {
  }

  as_invocable(as_invocable&& other) ASIO_NOEXCEPT
    : receiver_(other.receiver_)    //移动构造
  {
    other.receiver_ = 0;
  }

  ~as_invocable()   //析构
  {
    if (receiver_)
      execution::set_done(ASIO_MOVE_OR_LVALUE(Receiver)(*receiver_));   //有相对的接收者，就调用执行过程的set_done函数关闭
  }

  void operator()() ASIO_LVALUE_REF_QUAL ASIO_NOEXCEPT
  {
#if !defined(ASIO_NO_EXCEPTIONS)
    try
    {
#endif // !defined(ASIO_NO_EXCEPTIONS)
      execution::set_value(ASIO_MOVE_CAST(Receiver)(*receiver_));       //试着设值
      receiver_ = 0;                                                    //然后清除绑定的接收者
#if !defined(ASIO_NO_EXCEPTIONS)
    }
    catch (...)
    {
#if defined(ASIO_HAS_STD_EXCEPTION_PTR)
      execution::set_error(ASIO_MOVE_CAST(Receiver)(*receiver_),
          std::make_exception_ptr(receiver_invocation_error()));        //异常发生：设置异常[接收者，异常指针]
      receiver_ = 0;                                                    //清除绑定
#else // defined(ASIO_HAS_STD_EXCEPTION_PTR)
      std::terminate();
#endif // defined(ASIO_HAS_STD_EXCEPTION_PTR)
    }
#endif // !defined(ASIO_NO_EXCEPTIONS)
  }
};

#else // defined(ASIO_HAS_MOVE)

template <typename Receiver, typename>
struct as_invocable
{
  Receiver* receiver_;
  asio::detail::shared_ptr<asio::detail::atomic_count> ref_count_;

  explicit as_invocable(Receiver& r,
      const asio::detail::shared_ptr<
        asio::detail::atomic_count>& c) ASIO_NOEXCEPT
    : receiver_(asio::detail::addressof(r)),
      ref_count_(c)
  {
  }

  as_invocable(const as_invocable& other) ASIO_NOEXCEPT
    : receiver_(other.receiver_),
      ref_count_(other.ref_count_)
  {
    ++(*ref_count_);
  }

  ~as_invocable()
  {
    if (--(*ref_count_) == 0)
      execution::set_done(*receiver_);
  }

  void operator()() ASIO_LVALUE_REF_QUAL ASIO_NOEXCEPT
  {
#if !defined(ASIO_NO_EXCEPTIONS)
    try
    {
#endif // !defined(ASIO_NO_EXCEPTIONS)
      execution::set_value(*receiver_);
      ++(*ref_count_);
    }
#if !defined(ASIO_NO_EXCEPTIONS)
    catch (...)
    {
#if defined(ASIO_HAS_STD_EXCEPTION_PTR)
      execution::set_error(*receiver_,
          std::make_exception_ptr(receiver_invocation_error()));
      ++(*ref_count_);
#else // defined(ASIO_HAS_STD_EXCEPTION_PTR)
      std::terminate();
#endif // defined(ASIO_HAS_STD_EXCEPTION_PTR)
    }
#endif // !defined(ASIO_NO_EXCEPTIONS)
  }
};

#endif // defined(ASIO_HAS_MOVE)

///===================================================================
// <>只传一个参，默认为false
template <typename T>
struct is_as_invocable : false_type
{
};

///===================================================================
// <>传一个参，但这个参数是as_invocable<>的对象，默认为真
template <typename Function, typename T>
struct is_as_invocable<as_invocable<Function, T> > : true_type
{
};

} // namespace detail
} // namespace execution
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_EXECUTION_DETAIL_AS_INVOCABLE_HPP
