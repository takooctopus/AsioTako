//
// TAKO: scoped_ptr用来自动管理对象的生命周期
// detail/scoped_ptr.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_SCOPED_PTR_HPP
#define ASIO_DETAIL_SCOPED_PTR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

template <typename T>
class scoped_ptr
{
public:
  // Constructor.
  explicit scoped_ptr(T* p = 0) 
    : p_(p) //传入一个指针，将其初始化给内部指针
  {
  }

  // Destructor.
  ~scoped_ptr()
  {
    delete p_;  //退出时删除
  }

  // Access.
  T* get()
  {
    return p_;  //get()函数返回内部指针
  }

  // Access.
  T* operator->()
  {
    return p_;  //*操作和get()相同
  }

  // Dereference.
  T& operator*()
  {
    return *p_; //&操作返回内部指针指向的实例
  }

  // Reset pointer.
  void reset(T* p = 0)
  {
    delete p_;  //重置引用对象
    p_ = p;
  }

  // Release ownership of the pointer.
  T* release()
  {
    T* tmp = p_;    
    p_ = 0;
    return tmp; //释放内部元素所有权
  }

private:
  // Disallow copying and assignment.
  scoped_ptr(const scoped_ptr&);
  scoped_ptr& operator=(const scoped_ptr&);

  T* p_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_SCOPED_PTR_HPP
