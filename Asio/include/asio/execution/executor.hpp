//
// TAKO:执行器
// execution/executor.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_EXECUTOR_HPP
#define ASIO_EXECUTION_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/type_traits.hpp"  //类型萃取
#include "asio/execution/execute.hpp"   //执行
#include "asio/execution/invocable_archetype.hpp"   //可唤醒函数原型
#include "asio/traits/equality_comparable.hpp"

#if defined(ASIO_HAS_DEDUCED_EXECUTE_FREE_TRAIT) \
  && defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT) \
  && defined(ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)
# define ASIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT 1
#endif // defined(ASIO_HAS_DEDUCED_EXECUTE_FREE_TRAIT)
       //   && defined(ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)
       //   && defined(ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

#include "asio/detail/push_options.hpp"

namespace asio {
namespace execution {
namespace detail {

/// <summary>
/// 看看T是不是F这个实现的执行器，默认的特化就是false_type
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="F"></typeparam>
/// <typeparam name=""></typeparam>
/// <typeparam name=""></typeparam>
/// <typeparam name=""></typeparam>
/// <typeparam name=""></typeparam>
/// <typeparam name=""></typeparam>
/// <typeparam name=""></typeparam>
/// <typeparam name=""></typeparam>
/// <typeparam name=""></typeparam>
template <typename T, typename F,
    typename = void, typename = void, typename = void, typename = void,
    typename = void, typename = void, typename = void, typename = void>
struct is_executor_of_impl : false_type
{
};

/// <summary>
/// 看看是不是执行器，派生于true_type的
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="F"></typeparam>
template <typename T, typename F>
struct is_executor_of_impl<T, F,
  typename enable_if<
    can_execute<typename add_const<T>::type, F>::value
  >::type,  //3. t.execute(f)能不能运行 => 只有可以才特化
  typename void_type<
    typename result_of<typename decay<F>::type&()>::type
  >::type,  //4. t.execute(f)返回值 => type => void
  typename enable_if<
    is_constructible<typename decay<F>::type, F>::value
  >::type,  //5. F()的参数能不能用F【应该是看能不能拷贝】[只有可以才特化]
  typename enable_if<
    is_move_constructible<typename decay<F>::type>::value
  >::type,  //6. F()能不能用移动构造[只有可以才特化]
#if defined(ASIO_HAS_NOEXCEPT)
  typename enable_if<
    is_nothrow_copy_constructible<T>::value
  >::type,  //7. T有没有不抛异常的拷贝构造[只有不抛异常才能特化]
  typename enable_if<
    is_nothrow_destructible<T>::value
  >::type,  //8. T有没有不抛异常的析构函数[只有不抛才特化]
#else // defined(ASIO_HAS_NOEXCEPT)
  typename enable_if<
    is_copy_constructible<T>::value
  >::type,
  typename enable_if<
    is_destructible<T>::value
  >::type,
#endif // defined(ASIO_HAS_NOEXCEPT)
  typename enable_if<
    traits::equality_comparable<T>::is_valid
  >::type,  //9. T能不能等价比较[只有可以比较才特化]
  typename enable_if<
    traits::equality_comparable<T>::is_noexcept
  >::type> : true_type  //10. T能不能不抛异常地等价比较[只有不抛异常才特化]
{
};

/// <summary>
/// 执行器形状
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name=""></typeparam>
template <typename T, typename = void>
struct executor_shape
{
  typedef std::size_t type; //定义一个u64地类型
};

/// <summary>
/// 上面的模板特化，主要是重定义了type类型
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
struct executor_shape<T,
    typename void_type<
      typename T::shape_type
    >::type>    //为了实现模板特化，将T=>使用void_type<>转化成下面的type => void
{
  typedef typename T::shape_type type;  //重定义type类型，传递type
};

/// <summary>
/// 执行器序号
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="Default"></typeparam>
/// <typeparam name=""></typeparam>
template <typename T, typename Default, typename = void>
struct executor_index
{
  typedef Default type; //传递类型type => Default[第二个模板参数]
};

/// <summary>
/// 执行器序号[上面版本的特化][主要是传递<>第一个参数的index类型]
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="Default"></typeparam>
template <typename T, typename Default>
struct executor_index<T, Default,
    typename void_type<
      typename T::index_type
    >::type>    // 将T::index_type
{
  typedef typename T::index_type type;  //传递T的index类型
};

} // namespace detail

///======================================================================================================================
/// The is_executor trait detects whether a type T satisfies the
/// execution::executor concept.
/**
 * Class template @c is_executor is a UnaryTypeTrait that is derived from @c
 * true_type if the type @c T meets the concept definition for an executor,
 * otherwise @c false_type.
 * 如果模板类型T满足执行器的概念，那么就派生于true_type，否则就是false_type
 */
template <typename T>
struct is_executor :
#if defined(GENERATING_DOCUMENTATION)
  integral_constant<bool, automatically_determined>
#else // defined(GENERATING_DOCUMENTATION)
  // 我们回看上面的特化，我们要保证 T可以使用execute()调用invocable_archetype
  // 还要保证invocable_archetype类的构造特性
  // 还要保证T的构造和析构特性
  // 还要保证T实现了== 和 != 方法
  detail::is_executor_of_impl<T, invocable_archetype>   
#endif // defined(GENERATING_DOCUMENTATION)
{
};

#if defined(ASIO_HAS_VARIABLE_TEMPLATES)

/// <summary>
/// 定义编译器常量，true_type/false_type
/// </summary>
template <typename T>
ASIO_CONSTEXPR const bool is_executor_v = is_executor<T>::value;

#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

#if defined(ASIO_HAS_CONCEPTS)

/// <summary>
/// c++20后有concept概念： 是不是执行器
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
ASIO_CONCEPT executor = is_executor<T>::value;

#define ASIO_EXECUTION_EXECUTOR ::asio::execution::executor

#else // defined(ASIO_HAS_CONCEPTS)

#define ASIO_EXECUTION_EXECUTOR typename

#endif // defined(ASIO_HAS_CONCEPTS)

/// The is_executor_of trait detects whether a type T satisfies the
/// execution::executor_of concept for some set of value arguments.
/**
 * Class template @c is_executor_of is a type trait that is derived from @c
 * true_type if the type @c T meets the concept definition for an executor
 * that is invocable with a function object of type @c F, otherwise @c
 * false_type.
 * T是不是F的执行器
 */
template <typename T, typename F>
struct is_executor_of :
#if defined(GENERATING_DOCUMENTATION)
  integral_constant<bool, automatically_determined>
#else // defined(GENERATING_DOCUMENTATION)
  integral_constant<bool,
    is_executor<T>::value && detail::is_executor_of_impl<T, F>::value
  > //判断 1.T是不是执行器 2.T是不是F这个实现的执行器
#endif // defined(GENERATING_DOCUMENTATION)
{
};

#if defined(ASIO_HAS_VARIABLE_TEMPLATES)

/// <summary>
/// 上面的编译器常量模板
/// </summary>
template <typename T, typename F>
ASIO_CONSTEXPR const bool is_executor_of_v =
  is_executor_of<T, F>::value;

#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

#if defined(ASIO_HAS_CONCEPTS)

/// <summary>
/// 定义概念，T是F的执行器
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="F"></typeparam>
template <typename T, typename F>
ASIO_CONCEPT executor_of = is_executor_of<T, F>::value;

#define ASIO_EXECUTION_EXECUTOR_OF(f) \
  ::asio::execution::executor_of<f>

#else // defined(ASIO_HAS_CONCEPTS)

#define ASIO_EXECUTION_EXECUTOR_OF typename

#endif // defined(ASIO_HAS_CONCEPTS)

/// The executor_shape trait detects the type used by an executor to represent
/// the shape of a bulk operation.
/**
 * Class template @c executor_shape is a type trait with a nested type alias
 * @c type whose type is @c T::shape_type if @c T::shape_type is valid,
 * otherwise @c std::size_t.
 */
template <typename T>
struct executor_shape
#if !defined(GENERATING_DOCUMENTATION)
  : detail::executor_shape<T>   //继承于执行器形状，这个模板主要的作用是传递了T::shape_type的类型
#endif // !defined(GENERATING_DOCUMENTATION)
{
#if defined(GENERATING_DOCUMENTATION)
 /// @c T::shape_type if @c T::shape_type is valid, otherwise @c std::size_t.
 typedef automatically_determined type;
#endif // defined(GENERATING_DOCUMENTATION)
};

#if defined(ASIO_HAS_ALIAS_TEMPLATES)

/// <summary>
/// 传递了类型，我们也可以再向外传递类型
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
using executor_shape_t = typename executor_shape<T>::type;

#endif // defined(ASIO_HAS_ALIAS_TEMPLATES)

/// The executor_index trait detects the type used by an executor to represent
/// an index within a bulk operation.
/**
 * Class template @c executor_index is a type trait with a nested type alias
 * @c type whose type is @c T::index_type if @c T::index_type is valid,
 * otherwise @c executor_shape_t<T>.
 */
template <typename T>
struct executor_index
#if !defined(GENERATING_DOCUMENTATION)
  : detail::executor_index<T, typename executor_shape<T>::type> //同样的，传递了T::index_type
#endif // !defined(GENERATING_DOCUMENTATION)
{
#if defined(GENERATING_DOCUMENTATION)
 /// @c T::index_type if @c T::index_type is valid, otherwise
 /// @c executor_shape_t<T>.
 typedef automatically_determined type;
#endif // defined(GENERATING_DOCUMENTATION)
};

#if defined(ASIO_HAS_ALIAS_TEMPLATES)

// 继续向外传递T::index_type
template <typename T>
using executor_index_t = typename executor_index<T>::type;

#endif // defined(ASIO_HAS_ALIAS_TEMPLATES)

} // namespace execution
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_EXECUTION_EXECUTOR_HPP
