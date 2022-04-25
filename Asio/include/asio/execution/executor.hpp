//
// TAKO:ִ����
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
#include "asio/detail/type_traits.hpp"  //������ȡ
#include "asio/execution/execute.hpp"   //ִ��
#include "asio/execution/invocable_archetype.hpp"   //�ɻ��Ѻ���ԭ��
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
/// ����T�ǲ���F���ʵ�ֵ�ִ������Ĭ�ϵ��ػ�����false_type
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
/// �����ǲ���ִ������������true_type��
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="F"></typeparam>
template <typename T, typename F>
struct is_executor_of_impl<T, F,
  typename enable_if<
    can_execute<typename add_const<T>::type, F>::value
  >::type,  //3. t.execute(f)�ܲ������� => ֻ�п��Բ��ػ�
  typename void_type<
    typename result_of<typename decay<F>::type&()>::type
  >::type,  //4. t.execute(f)����ֵ => type => void
  typename enable_if<
    is_constructible<typename decay<F>::type, F>::value
  >::type,  //5. F()�Ĳ����ܲ�����F��Ӧ���ǿ��ܲ��ܿ�����[ֻ�п��Բ��ػ�]
  typename enable_if<
    is_move_constructible<typename decay<F>::type>::value
  >::type,  //6. F()�ܲ������ƶ�����[ֻ�п��Բ��ػ�]
#if defined(ASIO_HAS_NOEXCEPT)
  typename enable_if<
    is_nothrow_copy_constructible<T>::value
  >::type,  //7. T��û�в����쳣�Ŀ�������[ֻ�в����쳣�����ػ�]
  typename enable_if<
    is_nothrow_destructible<T>::value
  >::type,  //8. T��û�в����쳣����������[ֻ�в��ײ��ػ�]
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
  >::type,  //9. T�ܲ��ܵȼ۱Ƚ�[ֻ�п��ԱȽϲ��ػ�]
  typename enable_if<
    traits::equality_comparable<T>::is_noexcept
  >::type> : true_type  //10. T�ܲ��ܲ����쳣�صȼ۱Ƚ�[ֻ�в����쳣���ػ�]
{
};

/// <summary>
/// ִ������״
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name=""></typeparam>
template <typename T, typename = void>
struct executor_shape
{
  typedef std::size_t type; //����һ��u64������
};

/// <summary>
/// �����ģ���ػ�����Ҫ���ض�����type����
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
struct executor_shape<T,
    typename void_type<
      typename T::shape_type
    >::type>    //Ϊ��ʵ��ģ���ػ�����T=>ʹ��void_type<>ת���������type => void
{
  typedef typename T::shape_type type;  //�ض���type���ͣ�����type
};

/// <summary>
/// ִ�������
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="Default"></typeparam>
/// <typeparam name=""></typeparam>
template <typename T, typename Default, typename = void>
struct executor_index
{
  typedef Default type; //��������type => Default[�ڶ���ģ�����]
};

/// <summary>
/// ִ�������[����汾���ػ�][��Ҫ�Ǵ���<>��һ��������index����]
/// </summary>
/// <typeparam name="T"></typeparam>
/// <typeparam name="Default"></typeparam>
template <typename T, typename Default>
struct executor_index<T, Default,
    typename void_type<
      typename T::index_type
    >::type>    // ��T::index_type
{
  typedef typename T::index_type type;  //����T��index����
};

} // namespace detail

///======================================================================================================================
/// The is_executor trait detects whether a type T satisfies the
/// execution::executor concept.
/**
 * Class template @c is_executor is a UnaryTypeTrait that is derived from @c
 * true_type if the type @c T meets the concept definition for an executor,
 * otherwise @c false_type.
 * ���ģ������T����ִ�����ĸ����ô��������true_type���������false_type
 */
template <typename T>
struct is_executor :
#if defined(GENERATING_DOCUMENTATION)
  integral_constant<bool, automatically_determined>
#else // defined(GENERATING_DOCUMENTATION)
  // ���ǻؿ�������ػ�������Ҫ��֤ T����ʹ��execute()����invocable_archetype
  // ��Ҫ��֤invocable_archetype��Ĺ�������
  // ��Ҫ��֤T�Ĺ������������
  // ��Ҫ��֤Tʵ����== �� != ����
  detail::is_executor_of_impl<T, invocable_archetype>   
#endif // defined(GENERATING_DOCUMENTATION)
{
};

#if defined(ASIO_HAS_VARIABLE_TEMPLATES)

/// <summary>
/// ���������������true_type/false_type
/// </summary>
template <typename T>
ASIO_CONSTEXPR const bool is_executor_v = is_executor<T>::value;

#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

#if defined(ASIO_HAS_CONCEPTS)

/// <summary>
/// c++20����concept��� �ǲ���ִ����
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
 * T�ǲ���F��ִ����
 */
template <typename T, typename F>
struct is_executor_of :
#if defined(GENERATING_DOCUMENTATION)
  integral_constant<bool, automatically_determined>
#else // defined(GENERATING_DOCUMENTATION)
  integral_constant<bool,
    is_executor<T>::value && detail::is_executor_of_impl<T, F>::value
  > //�ж� 1.T�ǲ���ִ���� 2.T�ǲ���F���ʵ�ֵ�ִ����
#endif // defined(GENERATING_DOCUMENTATION)
{
};

#if defined(ASIO_HAS_VARIABLE_TEMPLATES)

/// <summary>
/// ����ı���������ģ��
/// </summary>
template <typename T, typename F>
ASIO_CONSTEXPR const bool is_executor_of_v =
  is_executor_of<T, F>::value;

#endif // defined(ASIO_HAS_VARIABLE_TEMPLATES)

#if defined(ASIO_HAS_CONCEPTS)

/// <summary>
/// ������T��F��ִ����
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
  : detail::executor_shape<T>   //�̳���ִ������״�����ģ����Ҫ�������Ǵ�����T::shape_type������
#endif // !defined(GENERATING_DOCUMENTATION)
{
#if defined(GENERATING_DOCUMENTATION)
 /// @c T::shape_type if @c T::shape_type is valid, otherwise @c std::size_t.
 typedef automatically_determined type;
#endif // defined(GENERATING_DOCUMENTATION)
};

#if defined(ASIO_HAS_ALIAS_TEMPLATES)

/// <summary>
/// ���������ͣ�����Ҳ���������⴫������
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
  : detail::executor_index<T, typename executor_shape<T>::type> //ͬ���ģ�������T::index_type
#endif // !defined(GENERATING_DOCUMENTATION)
{
#if defined(GENERATING_DOCUMENTATION)
 /// @c T::index_type if @c T::index_type is valid, otherwise
 /// @c executor_shape_t<T>.
 typedef automatically_determined type;
#endif // defined(GENERATING_DOCUMENTATION)
};

#if defined(ASIO_HAS_ALIAS_TEMPLATES)

// �������⴫��T::index_type
template <typename T>
using executor_index_t = typename executor_index<T>::type;

#endif // defined(ASIO_HAS_ALIAS_TEMPLATES)

} // namespace execution
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_EXECUTION_EXECUTOR_HPP
