//
// detail/noncopyable.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_NONCOPYABLE_HPP
#define ASIO_DETAIL_NONCOPYABLE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class noncopyable	//不可拷贝
{
protected:
  noncopyable() {}	// 构建函数放到protected【只能由子类进行访问】
  ~noncopyable() {}
private:
  noncopyable(const noncopyable&);	//其拷贝构造函数设为私有
  const noncopyable& operator=(const noncopyable&);	//其赋值函数设为私有
};

} // namespace detail

using asio::detail::noncopyable;

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_NONCOPYABLE_HPP
