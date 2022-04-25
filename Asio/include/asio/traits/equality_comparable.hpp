//
// TAKO: 能等价比较
// traits/equality_comparable.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_TRAITS_EQUALITY_COMPARABLE_HPP
#define ASIO_TRAITS_EQUALITY_COMPARABLE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/type_traits.hpp"

#if defined(ASIO_HAS_DECLTYPE) \
  && defined(ASIO_HAS_NOEXCEPT) \
  && defined(ASIO_HAS_WORKING_EXPRESSION_SFINAE)
# define ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT 1
#endif // defined(ASIO_HAS_DECLTYPE)
       //   && defined(ASIO_HAS_NOEXCEPT)
       //   && defined(ASIO_HAS_WORKING_EXPRESSION_SFINAE)

namespace asio {
namespace traits {

template <typename T, typename = void>
struct equality_comparable_default;

template <typename T, typename = void>
struct equality_comparable;

} // namespace traits
namespace detail {

///==================================================================
// 默认的不能等价比较，定义了两个编译器常量
struct no_equality_comparable
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = false);
  ASIO_STATIC_CONSTEXPR(bool, is_noexcept = false);
};

#if defined(ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

///==================================================================
// 默认继承上面的基础结构体
template <typename T, typename = void>
struct equality_comparable_trait : no_equality_comparable
{
};

///==================================================================
template <typename T>
struct equality_comparable_trait<T,
  typename void_type<
    decltype(
      static_cast<void>(
        static_cast<bool>(declval<const T>() == declval<const T>())
      ),    // [无用式子] 1. true => bool => void 【保证==可用】
      static_cast<void>(
        static_cast<bool>(declval<const T>() != declval<const T>())
      )     // [实际式子] 2. false => bool => void 【保证!=可用】
    )   //推导出来是 void
  >::type>  // 推导出来 void
{
  ASIO_STATIC_CONSTEXPR(bool, is_valid = true);

  ASIO_STATIC_CONSTEXPR(bool, is_noexcept =
    noexcept(declval<const T>() == declval<const T>())
      && noexcept(declval<const T>() != declval<const T>()));   //继承于下面的有无异常
};

#else // defined(ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

template <typename T, typename = void>
struct equality_comparable_trait :
  conditional<
    is_same<T, typename decay<T>::type>::value,
    no_equality_comparable,
    traits::equality_comparable<typename decay<T>::type>
  >::type
{
};

#endif // defined(ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

} // namespace detail
namespace traits {

template <typename T, typename>
struct equality_comparable_default : detail::equality_comparable_trait<T>
{
};

template <typename T, typename>
struct equality_comparable : equality_comparable_default<T>
{
};

} // namespace traits
} // namespace asio

#endif // ASIO_TRAITS_EQUALITY_COMPARABLE_HPP
