//
// TAKO: 类型萃取
// detail/type_traits.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_TYPE_TRAITS_HPP
#define ASIO_DETAIL_TYPE_TRAITS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_STD_TYPE_TRAITS)
# include <type_traits>
#else // defined(ASIO_HAS_STD_TYPE_TRAITS)
# include <boost/type_traits/add_const.hpp>
# include <boost/type_traits/add_lvalue_reference.hpp>
# include <boost/type_traits/aligned_storage.hpp>
# include <boost/type_traits/alignment_of.hpp>
# include <boost/type_traits/conditional.hpp>
# include <boost/type_traits/decay.hpp>
# include <boost/type_traits/has_nothrow_copy.hpp>
# include <boost/type_traits/has_nothrow_destructor.hpp>
# include <boost/type_traits/integral_constant.hpp>
# include <boost/type_traits/is_base_of.hpp>
# include <boost/type_traits/is_class.hpp>
# include <boost/type_traits/is_const.hpp>
# include <boost/type_traits/is_convertible.hpp>
# include <boost/type_traits/is_constructible.hpp>
# include <boost/type_traits/is_copy_constructible.hpp>
# include <boost/type_traits/is_destructible.hpp>
# include <boost/type_traits/is_function.hpp>
# include <boost/type_traits/is_object.hpp>
# include <boost/type_traits/is_same.hpp>
# include <boost/type_traits/remove_cv.hpp>
# include <boost/type_traits/remove_pointer.hpp>
# include <boost/type_traits/remove_reference.hpp>
# include <boost/utility/declval.hpp>
# include <boost/utility/enable_if.hpp>
# include <boost/utility/result_of.hpp>
#endif // defined(ASIO_HAS_STD_TYPE_TRAITS)

namespace asio {

#if defined(ASIO_HAS_STD_TYPE_TRAITS)
using std::add_const; //添加 const 或/与 volatile 限定符到给定类型
using std::add_lvalue_reference; //向给定类型添加左值或右值引用
using std::aligned_storage; // 定义适于用作给定大小的类型的未初始化存储的类型 (C++11)(C++23 中弃用)
using std::alignment_of; //获取类型的对齐要求
using std::conditional; //基于编译时布尔值选择一个类型或另一个std::conditional<true, int, double>::type 用来选择类型
using std::decay;	//类型 T 应用左值到右值、数组到指针及函数到指针隐式转换，移除 cv 限定符，并定义结果类型为成员 typedef type
using std::declval; //用于推断类型[一个类可能没有默认构造函数，decltype可能不能执行]
using std::enable_if; //SFINAE用法，
using std::false_type;	//std::integral_constant<bool, false>
using std::integral_constant; //std::integral_constant<bool, true>
using std::is_base_of;	//std::is_base_of<A, B>::value 判断A是不是B的基类[多次派生也算的]
using std::is_class;	//std::is_class<T>::value 判断是不是class[或者struct]
using std::is_const;	//判断是不是const限定
using std::is_constructible;	// std::is_constructible<T, param_t_1, param_t2,...> 是否可以构造
using std::is_convertible;	//std::is_convertible<B*, A*>::value; 能不能从B转A； 一般来说类只能子类转父类[实例，指针都是] [完美转发构造函数可以从任何一个转过去]
using std::is_copy_constructible; //是否拥有复制构造函数
using std::is_destructible;	//是否拥有未被弃置的析构函数
using std::is_function;	//判断是不是函数类型 普通函数和类中静态函数被认为是函数类型，比较特殊的是通过模板萃取出成员函数类型也可以是函数类型，但是通过std::mem_fun得到的并不认为是函数类型，是一 函数指针
using std::is_move_constructible; //是不是有移动构造
using std::is_nothrow_copy_constructible; //拷贝构造里面无异常
using std::is_nothrow_destructible;	//析构函数无异常
using std::is_object;	//是一个对象
using std::is_reference;	//是引用
using std::is_same;	//类型相同
using std::is_scalar; //是标量 C++ 内存模型中的每个独立内存位置，包括语言特性所用的隐藏内存位置（如虚表指针），都拥有标量类型（或为相邻的零长位域序列）。表达式求值中的副效应排序、线程间同步和依赖顺序全部都基于标量对象定义。
using std::remove_cv;	//从给定类型移除 const 或/与 volatile 限定符
template <typename T>
struct remove_cvref : remove_cv<typename std::remove_reference<T>::type> {};	//从给定类型移除常引用
using std::remove_pointer;	//移除给定类型的一层指针
using std::remove_reference; //从给定类型移除引用
#if defined(ASIO_HAS_STD_INVOKE_RESULT)
template <typename> struct result_of;	//推导以一组实参调用一个可调用对象的结果类型 (C++20 中移除)
template <typename F, typename... Args>
struct result_of<F(Args...)> : std::invoke_result<F, Args...> {};
#else // defined(ASIO_HAS_STD_INVOKE_RESULT)
using std::result_of;
#endif // defined(ASIO_HAS_STD_INVOKE_RESULT)
using std::true_type;
#else // defined(ASIO_HAS_STD_TYPE_TRAITS)
using boost::add_const;
using boost::add_lvalue_reference;
using boost::aligned_storage;
using boost::alignment_of;
template <bool Condition, typename Type = void>
struct enable_if : boost::enable_if_c<Condition, Type> {};
using boost::conditional;
using boost::decay;
using boost::declval;
using boost::false_type;
using boost::integral_constant;
using boost::is_base_of;
using boost::is_class;
using boost::is_const;
using boost::is_constructible;
using boost::is_convertible;
using boost::is_copy_constructible;
using boost::is_destructible;
using boost::is_function;
#if defined(ASIO_HAS_MOVE)
template <typename T>
struct is_move_constructible : false_type {};
#else // defined(ASIO_HAS_MOVE)
template <typename T>
struct is_move_constructible : is_copy_constructible<T> {};
#endif // defined(ASIO_HAS_MOVE)
template <typename T>
struct is_nothrow_copy_constructible : boost::has_nothrow_copy<T> {};
template <typename T>
struct is_nothrow_destructible : boost::has_nothrow_destructor<T> {};
using boost::is_object;
using boost::is_reference;
using boost::is_same;
using boost::is_scalar;
using boost::remove_cv;
template <typename T>
struct remove_cvref : remove_cv<typename boost::remove_reference<T>::type> {};
using boost::remove_pointer;
using boost::remove_reference;
using boost::result_of;
using boost::true_type;
#endif // defined(ASIO_HAS_STD_TYPE_TRAITS)

template <typename> struct void_type { typedef void type; };

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename...> struct conjunction : true_type {};
template <typename T> struct conjunction<T> : T {};
template <typename Head, typename... Tail> struct conjunction<Head, Tail...> :
  conditional<Head::value, conjunction<Tail...>, Head>::type {};

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

struct defaulted_constraint
{
  ASIO_CONSTEXPR defaulted_constraint() {}
};

template <bool Condition, typename Type = int>
struct constraint : enable_if<Condition, Type> {}; //重新定义了一个继承于enable_if的结构体

} // namespace asio

#endif // ASIO_DETAIL_TYPE_TRAITS_HPP
