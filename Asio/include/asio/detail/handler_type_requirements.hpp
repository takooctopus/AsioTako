//
// TAKO: 句柄类型要求
// detail/handler_type_requirements.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_HANDLER_TYPE_REQUIREMENTS_HPP
#define ASIO_DETAIL_HANDLER_TYPE_REQUIREMENTS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

// Older versions of gcc have difficulty compiling the sizeof expressions where
// we test the handler type requirements. We'll disable checking of handler type
// requirements for those compilers, but otherwise enable it by default.
#if !defined(ASIO_DISABLE_HANDLER_TYPE_REQUIREMENTS)
# if !defined(__GNUC__) || (__GNUC__ >= 4)
#  define ASIO_ENABLE_HANDLER_TYPE_REQUIREMENTS 1
# endif // !defined(__GNUC__) || (__GNUC__ >= 4)
#endif // !defined(ASIO_DISABLE_HANDLER_TYPE_REQUIREMENTS)

// With C++0x we can use a combination of enhanced SFINAE and static_assert to
// generate better template error messages. As this technique is not yet widely
// portable, we'll only enable it for tested compilers.
#if !defined(ASIO_DISABLE_HANDLER_TYPE_REQUIREMENTS_ASSERT)
# if defined(__GNUC__)
#  if ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 5)) || (__GNUC__ > 4)
#   if defined(__GXX_EXPERIMENTAL_CXX0X__)
#    define ASIO_ENABLE_HANDLER_TYPE_REQUIREMENTS_ASSERT 1
#   endif // defined(__GXX_EXPERIMENTAL_CXX0X__)
#  endif // ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 5)) || (__GNUC__ > 4)
# endif // defined(__GNUC__)
# if defined(ASIO_MSVC)
#  if (_MSC_VER >= 1600)
#   define ASIO_ENABLE_HANDLER_TYPE_REQUIREMENTS_ASSERT 1
#  endif // (_MSC_VER >= 1600)
# endif // defined(ASIO_MSVC)
# if defined(__clang__)
#  if __has_feature(__cxx_static_assert__)
#   define ASIO_ENABLE_HANDLER_TYPE_REQUIREMENTS_ASSERT 1
#  endif // __has_feature(cxx_static_assert)
# endif // defined(__clang__)
#endif // !defined(ASIO_DISABLE_HANDLER_TYPE_REQUIREMENTS)

#if defined(ASIO_ENABLE_HANDLER_TYPE_REQUIREMENTS)
# include "asio/async_result.hpp"
#endif // defined(ASIO_ENABLE_HANDLER_TYPE_REQUIREMENTS)

namespace asio {
namespace detail {

#if defined(ASIO_ENABLE_HANDLER_TYPE_REQUIREMENTS)

# if defined(ASIO_ENABLE_HANDLER_TYPE_REQUIREMENTS_ASSERT)
//=====================================================================================================
template <typename Handler>
auto zero_arg_copyable_handler_test(Handler h, void*)       //推导函数类型【没有参数的可拷贝的句柄测试】
  -> decltype(
    sizeof(Handler(static_cast<const Handler&>(h))),    //验证这个h可以加const&，加完const后可以调用拷贝构造，最后还可以求大小
    (ASIO_MOVE_OR_LVALUE(Handler)(h)()),                //验证这个h可以转化成右值
    char(0));   //推导的最后结果是char(0) => char

template <typename Handler>
char (&zero_arg_copyable_handler_test(Handler, ...))[2];    //返回值为char[2]【没有通过检查的返回char[2]】，最外面用长度来表示其检查是否通过

//=====================================================================================================
template <typename Handler, typename Arg1>
auto one_arg_handler_test(Handler h, Arg1* a1)  //第二个参数是指针【一个参数的】
  -> decltype(
    sizeof(Handler(ASIO_MOVE_CAST(Handler)(h))),        //验证这个h可以转化成右值，最后可以求大小
    (ASIO_MOVE_OR_LVALUE(Handler)(h)(*a1)),             //看看第二个参数能不能被右值转化后的h进行调用
    char(0));                                           //实际推导类型结果是char

template <typename Handler>
char (&one_arg_handler_test(Handler h, ...))[2];    //返回值char[2]

//=====================================================================================================
template <typename Handler, typename Arg1, typename Arg2>
auto two_arg_handler_test(Handler h, Arg1* a1, Arg2* a2)
  -> decltype(
    sizeof(Handler(ASIO_MOVE_CAST(Handler)(h))),
    (ASIO_MOVE_OR_LVALUE(Handler)(h)(*a1, *a2)),
    char(0));

template <typename Handler>
char (&two_arg_handler_test(Handler, ...))[2];

template <typename Handler, typename Arg1, typename Arg2>
auto two_arg_move_handler_test(Handler h, Arg1* a1, Arg2* a2)
  -> decltype(
    sizeof(Handler(ASIO_MOVE_CAST(Handler)(h))),
    (ASIO_MOVE_OR_LVALUE(Handler)(h)(
      *a1, ASIO_MOVE_CAST(Arg2)(*a2))),
    char(0));

template <typename Handler>
char (&two_arg_move_handler_test(Handler, ...))[2];

//=====================================================================================================
#  define ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT(expr, msg) \
     static_assert(expr, msg);

# else // defined(ASIO_ENABLE_HANDLER_TYPE_REQUIREMENTS_ASSERT)

#  define ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT(expr, msg)

# endif // defined(ASIO_ENABLE_HANDLER_TYPE_REQUIREMENTS_ASSERT)

//=====================================================================================================
// 这一系列的函数原型只是为了类型而设置的
template <typename T> T& lvref();   //模板返回左值引用类型
template <typename T> T& lvref(T);  //模板返回左值引用类型
template <typename T> const T& clvref();
template <typename T> const T& clvref(T);
#if defined(ASIO_HAS_MOVE)
template <typename T> T rvref();    //返回一个T类型[讲道理，这怎么判断类型的，这得多出多少个模板特化函数啊]
template <typename T> T rvref(T);   //返回一个T类型
template <typename T> T rorlvref(); //返回一个T类型
#else // defined(ASIO_HAS_MOVE)
template <typename T> const T& rvref();
template <typename T> const T& rvref(T);
template <typename T> T& rorlvref();
#endif // defined(ASIO_HAS_MOVE)
template <typename T> char argbyv(T);   //返回一个char类型

template <int>
struct handler_type_requirements
{
};

//=====================================================================================================
// 优雅的完成句柄检查
// 我们先定义这个真正的句柄类型，其ASIO_COMPLETION_SIGNATURES_TARGS即第二个参数[函数签名]是void()
// 因为是0参，用的是上面的zero那个check，当然，我们上去看这个check函数的返回值是char，那么sizeof就应该是1【没通过检查返回的是char[2]，sizeof就是2】
// 最后定义一个未用类型进行类型检查(1.看这个类型能不能变成const T&，再看能不能通过类型检查 2.看看能不能调用[在第二个sizeof种，又通过,进行了一次检查，看看能不能调用，实际返回的是1]) => 
// 这样实际下来typedef出来的就是sizeof(T) + 1，当然因为是类型检查，最后类型其实并没啥用就是了
#define ASIO_LEGACY_COMPLETION_HANDLER_CHECK( \
    handler_type, handler) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void()) asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::zero_arg_copyable_handler_test( \
          asio::detail::clvref< \
            asio_true_handler_type>(), 0)) == 1, \
      "CompletionHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::clvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()(), \
        char(0))> ASIO_UNUSED_TYPEDEF

//=====================================================================================================
//读取句柄检查
//后面的函数签名void(error_code, size_t)
//那么用的就是最上面two的拿一个测试【一样，通过了测试sizeof就是1，没有就是2】【这里面主要还是考虑能不能加const，能不能左右值转换】
#define ASIO_READ_HANDLER_CHECK( \
    handler_type, handler) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void(asio::error_code, std::size_t)) \
    asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::two_arg_handler_test( \
          asio::detail::rvref< \
            asio_true_handler_type>(), \
          static_cast<const asio::error_code*>(0), \
          static_cast<const std::size_t*>(0))) == 1, \
      "ReadHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::rvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()( \
            asio::detail::lvref<const asio::error_code>(), \
            asio::detail::lvref<const std::size_t>()), \
        char(0))> ASIO_UNUSED_TYPEDEF

//=====================================================================================================
// 写句柄的检查[双参数]
#define ASIO_WRITE_HANDLER_CHECK( \
    handler_type, handler) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void(asio::error_code, std::size_t)) \
    asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::two_arg_handler_test( \
          asio::detail::rvref< \
            asio_true_handler_type>(), \
          static_cast<const asio::error_code*>(0), \
          static_cast<const std::size_t*>(0))) == 1, \
      "WriteHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::rvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()( \
            asio::detail::lvref<const asio::error_code>(), \
            asio::detail::lvref<const std::size_t>()), \
        char(0))> ASIO_UNUSED_TYPEDEF

//=====================================================================================================
//接受句柄检查[单参数][只有一个error_code]
#define ASIO_ACCEPT_HANDLER_CHECK( \
    handler_type, handler) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void(asio::error_code)) \
    asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::one_arg_handler_test( \
          asio::detail::rvref< \
            asio_true_handler_type>(), \
          static_cast<const asio::error_code*>(0))) == 1, \
      "AcceptHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::rvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()( \
            asio::detail::lvref<const asio::error_code>()), \
        char(0))> ASIO_UNUSED_TYPEDEF

//=====================================================================================================
// 可移动的接受句柄检查[双参数][error_code和socket_type]
#define ASIO_MOVE_ACCEPT_HANDLER_CHECK( \
    handler_type, handler, socket_type) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void(asio::error_code, socket_type)) \
    asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::two_arg_move_handler_test( \
          asio::detail::rvref< \
            asio_true_handler_type>(), \
          static_cast<const asio::error_code*>(0), \
          static_cast<socket_type*>(0))) == 1, \
      "MoveAcceptHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::rvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()( \
            asio::detail::lvref<const asio::error_code>(), \
            asio::detail::rvref<socket_type>()), \
        char(0))> ASIO_UNUSED_TYPEDEF

//=====================================================================================================
// 连接句柄检查[单参][error_code]
#define ASIO_CONNECT_HANDLER_CHECK( \
    handler_type, handler) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void(asio::error_code)) \
    asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::one_arg_handler_test( \
          asio::detail::rvref< \
            asio_true_handler_type>(), \
          static_cast<const asio::error_code*>(0))) == 1, \
      "ConnectHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::rvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()( \
            asio::detail::lvref<const asio::error_code>()), \
        char(0))> ASIO_UNUSED_TYPEDEF

//=====================================================================================================
// 范围连接句柄[双参][void(error_code,endpoint_type)]
#define ASIO_RANGE_CONNECT_HANDLER_CHECK( \
    handler_type, handler, endpoint_type) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void(asio::error_code, endpoint_type)) \
    asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::two_arg_handler_test( \
          asio::detail::rvref< \
            asio_true_handler_type>(), \
          static_cast<const asio::error_code*>(0), \
          static_cast<const endpoint_type*>(0))) == 1, \
      "RangeConnectHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::rvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()( \
            asio::detail::lvref<const asio::error_code>(), \
            asio::detail::lvref<const endpoint_type>()), \
        char(0))> ASIO_UNUSED_TYPEDEF

//=====================================================================================================
// 迭代器连接句柄[双参][error_code，iter_type]
#define ASIO_ITERATOR_CONNECT_HANDLER_CHECK( \
    handler_type, handler, iter_type) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void(asio::error_code, iter_type)) \
    asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::two_arg_handler_test( \
          asio::detail::rvref< \
            asio_true_handler_type>(), \
          static_cast<const asio::error_code*>(0), \
          static_cast<const iter_type*>(0))) == 1, \
      "IteratorConnectHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::rvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()( \
            asio::detail::lvref<const asio::error_code>(), \
            asio::detail::lvref<const iter_type>()), \
        char(0))> ASIO_UNUSED_TYPEDEF

//=====================================================================================================
// 解析器句柄检查[双参][error_code, range_type]
#define ASIO_RESOLVE_HANDLER_CHECK( \
    handler_type, handler, range_type) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void(asio::error_code, range_type)) \
    asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::two_arg_handler_test( \
          asio::detail::rvref< \
            asio_true_handler_type>(), \
          static_cast<const asio::error_code*>(0), \
          static_cast<const range_type*>(0))) == 1, \
      "ResolveHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::rvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()( \
            asio::detail::lvref<const asio::error_code>(), \
            asio::detail::lvref<const range_type>()), \
        char(0))> ASIO_UNUSED_TYPEDEF

//=====================================================================================================
// 等待句柄类型检查[单参][error_code]
#define ASIO_WAIT_HANDLER_CHECK( \
    handler_type, handler) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void(asio::error_code)) \
    asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::one_arg_handler_test( \
          asio::detail::rvref< \
            asio_true_handler_type>(), \
          static_cast<const asio::error_code*>(0))) == 1, \
      "WaitHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::rvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()( \
            asio::detail::lvref<const asio::error_code>()), \
        char(0))> ASIO_UNUSED_TYPEDEF

//=====================================================================================================
//信号句柄检查[双参][error_code, int]
#define ASIO_SIGNAL_HANDLER_CHECK( \
    handler_type, handler) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void(asio::error_code, int)) \
    asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::two_arg_handler_test( \
          asio::detail::rvref< \
            asio_true_handler_type>(), \
          static_cast<const asio::error_code*>(0), \
          static_cast<const int*>(0))) == 1, \
      "SignalHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::rvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()( \
            asio::detail::lvref<const asio::error_code>(), \
            asio::detail::lvref<const int>()), \
        char(0))> ASIO_UNUSED_TYPEDEF

//=====================================================================================================
// 握手句柄检查[单参][error_code]
#define ASIO_HANDSHAKE_HANDLER_CHECK( \
    handler_type, handler) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void(asio::error_code)) \
    asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::one_arg_handler_test( \
          asio::detail::rvref< \
            asio_true_handler_type>(), \
          static_cast<const asio::error_code*>(0))) == 1, \
      "HandshakeHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::rvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()( \
            asio::detail::lvref<const asio::error_code>()), \
        char(0))> ASIO_UNUSED_TYPEDEF

//=====================================================================================================
//有缓冲的握手句柄检查[双参][error_code, size_t]
#define ASIO_BUFFERED_HANDSHAKE_HANDLER_CHECK( \
    handler_type, handler) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void(asio::error_code, std::size_t)) \
    asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::two_arg_handler_test( \
          asio::detail::rvref< \
            asio_true_handler_type>(), \
          static_cast<const asio::error_code*>(0), \
          static_cast<const std::size_t*>(0))) == 1, \
      "BufferedHandshakeHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::rvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()( \
          asio::detail::lvref<const asio::error_code>(), \
          asio::detail::lvref<const std::size_t>()), \
        char(0))> ASIO_UNUSED_TYPEDEF

//=====================================================================================================
// 关闭句柄检查[单参][error_code]
#define ASIO_SHUTDOWN_HANDLER_CHECK( \
    handler_type, handler) \
  \
  typedef ASIO_HANDLER_TYPE(handler_type, \
      void(asio::error_code)) \
    asio_true_handler_type; \
  \
  ASIO_HANDLER_TYPE_REQUIREMENTS_ASSERT( \
      sizeof(asio::detail::one_arg_handler_test( \
          asio::detail::rvref< \
            asio_true_handler_type>(), \
          static_cast<const asio::error_code*>(0))) == 1, \
      "ShutdownHandler type requirements not met") \
  \
  typedef asio::detail::handler_type_requirements< \
      sizeof( \
        asio::detail::argbyv( \
          asio::detail::rvref< \
            asio_true_handler_type>())) + \
      sizeof( \
        asio::detail::rorlvref< \
          asio_true_handler_type>()( \
            asio::detail::lvref<const asio::error_code>()), \
        char(0))> ASIO_UNUSED_TYPEDEF

#else // !defined(ASIO_ENABLE_HANDLER_TYPE_REQUIREMENTS)

#define ASIO_LEGACY_COMPLETION_HANDLER_CHECK( \
    handler_type, handler) \
  typedef int ASIO_UNUSED_TYPEDEF

#define ASIO_READ_HANDLER_CHECK( \
    handler_type, handler) \
  typedef int ASIO_UNUSED_TYPEDEF

#define ASIO_WRITE_HANDLER_CHECK( \
    handler_type, handler) \
  typedef int ASIO_UNUSED_TYPEDEF

#define ASIO_ACCEPT_HANDLER_CHECK( \
    handler_type, handler) \
  typedef int ASIO_UNUSED_TYPEDEF

#define ASIO_MOVE_ACCEPT_HANDLER_CHECK( \
    handler_type, handler, socket_type) \
  typedef int ASIO_UNUSED_TYPEDEF

#define ASIO_CONNECT_HANDLER_CHECK( \
    handler_type, handler) \
  typedef int ASIO_UNUSED_TYPEDEF

#define ASIO_RANGE_CONNECT_HANDLER_CHECK( \
    handler_type, handler, iter_type) \
  typedef int ASIO_UNUSED_TYPEDEF

#define ASIO_ITERATOR_CONNECT_HANDLER_CHECK( \
    handler_type, handler, iter_type) \
  typedef int ASIO_UNUSED_TYPEDEF

#define ASIO_RESOLVE_HANDLER_CHECK( \
    handler_type, handler, iter_type) \
  typedef int ASIO_UNUSED_TYPEDEF

#define ASIO_WAIT_HANDLER_CHECK( \
    handler_type, handler) \
  typedef int ASIO_UNUSED_TYPEDEF

#define ASIO_SIGNAL_HANDLER_CHECK( \
    handler_type, handler) \
  typedef int ASIO_UNUSED_TYPEDEF

#define ASIO_HANDSHAKE_HANDLER_CHECK( \
    handler_type, handler) \
  typedef int ASIO_UNUSED_TYPEDEF

#define ASIO_BUFFERED_HANDSHAKE_HANDLER_CHECK( \
    handler_type, handler) \
  typedef int ASIO_UNUSED_TYPEDEF

#define ASIO_SHUTDOWN_HANDLER_CHECK( \
    handler_type, handler) \
  typedef int ASIO_UNUSED_TYPEDEF

#endif // !defined(ASIO_ENABLE_HANDLER_TYPE_REQUIREMENTS)

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_HANDLER_TYPE_REQUIREMENTS_HPP
