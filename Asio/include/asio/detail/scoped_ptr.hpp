//
// TAKO: scoped_ptr�����Զ�����������������
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
    : p_(p) //����һ��ָ�룬�����ʼ�����ڲ�ָ��
  {
  }

  // Destructor.
  ~scoped_ptr()
  {
    delete p_;  //�˳�ʱɾ��
  }

  // Access.
  T* get()
  {
    return p_;  //get()���������ڲ�ָ��
  }

  // Access.
  T* operator->()
  {
    return p_;  //*������get()��ͬ
  }

  // Dereference.
  T& operator*()
  {
    return *p_; //&���������ڲ�ָ��ָ���ʵ��
  }

  // Reset pointer.
  void reset(T* p = 0)
  {
    delete p_;  //�������ö���
    p_ = p;
  }

  // Release ownership of the pointer.
  T* release()
  {
    T* tmp = p_;    
    p_ = 0;
    return tmp; //�ͷ��ڲ�Ԫ������Ȩ
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
