//
// TAKO:执行成员
// traits/execute_member.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_TRAITS_EXECUTE_MEMBER_HPP
#define ASIO_TRAITS_EXECUTE_MEMBER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/type_traits.hpp"      //类型萃取

#if defined(ASIO_HAS_DECLTYPE) \
  && defined(ASIO_HAS_NOEXCEPT) \
  && defined(ASIO_HAS_WORKING_EXPRESSION_SFINAE)
# define ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT 1
#endif // defined(ASIO_HAS_DECLTYPE)
       //   && defined(ASIO_HAS_NOEXCEPT)
       //   && defined(ASIO_HAS_WORKING_EXPRESSION_SFINAE)

#include "asio/detail/push_options.hpp"

namespace asio {
namespace traits {

///===================================================================
// 前置声明
template <typename T, typename F, typename = void>
struct execute_member_default;

///===================================================================
// 前置声明
template <typename T, typename F, typename = void>
struct execute_member;

} // namespace traits
namespace detail {

struct no_execute_member
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = false);        //不可用
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);     //要抛异常
};

#if defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

///===================================================================
template <typename T, typename F, typename = void>
struct execute_member_trait : no_execute_member
{
};  //不符合参数的全部特化到不算的那一类里

///===================================================================
// 主要是为了定义几个类型萃取的参数
template <typename T, typename F>
struct execute_member_trait<T, F,
  typename void_type<
    decltype(declval<T>().execute(declval<F>()))
  >::type>  //保证T类型的对象能够使用F类型的对象当作参数
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true); //可用

  using result_type = decltype(
    declval<T>().execute(declval<F>()));    //返回类型就是上面<>第三个参数的类型

  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = noexcept(
    declval<T>().execute(declval<F>())));       //判断一下这个函数是不是不抛异常的[noexcept()]，再去看看
};

#else // defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

template <typename T, typename F, typename = void>
struct execute_member_trait :
  conditional<
    is_same<T, typename decay<T>::type>::value
      && is_same<F, typename decay<F>::type>::value,
    no_execute_member,
    traits::execute_member<
      typename decay<T>::type,
      typename decay<F>::type>
  >::type
{
};

#endif // defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

} // namespace detail
namespace traits {

///===================================================================
// 在trait命名空间里的向外暴露
template <typename T, typename F, typename>
struct execute_member_default :
  detail::execute_member_trait<T, F>    
{
};

///===================================================================
// 在trait命名空间的向外暴露
template <typename T, typename F, typename>
struct execute_member :
  execute_member_default<T, F>
{
};

} // namespace traits
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_TRAITS_EXECUTE_MEMBER_HPP
