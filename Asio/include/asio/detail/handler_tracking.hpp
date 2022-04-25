//
// TAKO:句柄跟踪
// detail/handler_tracking.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_HANDLER_TRACKING_HPP
#define ASIO_DETAIL_HANDLER_TRACKING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

namespace asio {

class execution_context;    //前置声明，执行上下文

} // namespace asio

#if defined(ASIO_CUSTOM_HANDLER_TRACKING)
# include ASIO_CUSTOM_HANDLER_TRACKING
#elif defined(ASIO_ENABLE_HANDLER_TRACKING)
# include "asio/error_code.hpp"
# include "asio/detail/cstdint.hpp"
# include "asio/detail/static_mutex.hpp"
# include "asio/detail/tss_ptr.hpp"
#endif // defined(ASIO_ENABLE_HANDLER_TRACKING)

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

#if defined(ASIO_CUSTOM_HANDLER_TRACKING)

// The user-specified header must define the following macros:
// 【用户自定义的头文件必须包括下面的宏】
// - ASIO_INHERIT_TRACKED_HANDLER
// - ASIO_ALSO_INHERIT_TRACKED_HANDLER
// - ASIO_HANDLER_TRACKING_INIT
// - ASIO_HANDLER_CREATION(args)
// - ASIO_HANDLER_COMPLETION(args)
// - ASIO_HANDLER_INVOCATION_BEGIN(args)
// - ASIO_HANDLER_INVOCATION_END
// - ASIO_HANDLER_OPERATION(args)
// - ASIO_HANDLER_REACTOR_REGISTRATION(args)
// - ASIO_HANDLER_REACTOR_DEREGISTRATION(args)
// - ASIO_HANDLER_REACTOR_READ_EVENT
// - ASIO_HANDLER_REACTOR_WRITE_EVENT
// - ASIO_HANDLER_REACTOR_ERROR_EVENT
// - ASIO_HANDLER_REACTOR_EVENTS(args)
// - ASIO_HANDLER_REACTOR_OPERATION(args)

# if !defined(ASIO_ENABLE_HANDLER_TRACKING)
#  define ASIO_ENABLE_HANDLER_TRACKING 1
# endif /// !defined(ASIO_ENABLE_HANDLER_TRACKING)

#elif defined(ASIO_ENABLE_HANDLER_TRACKING)
// 只有在允许句柄跟踪时才定义的

class handler_tracking
{
public:
  class completion;

  /// ==========================================================================================================
  // Base class for objects containing tracked handlers. 【包含被追踪的句柄的基类】
  class tracked_handler
  {
  private:
    // Only the handler_tracking class will have access to the id. 【只有句柄追溯类才有权限访问id_】
    friend class handler_tracking;
    friend class completion;
    uint64_t id_;   //u64 id

  protected:
    // Constructor initialises with no id. 【默认构造函数，id为0】
    tracked_handler() : id_(0) {}

    // Prevent deletion through this type. 【析构函数protected】
    ~tracked_handler() {}
  };

  // Initialise the tracking system.
  ASIO_DECL static void init();

  /// ==========================================================================================================
  // 位置
  class location
  {
  public:
    // Constructor adds a location to the stack.【构造函数，向栈中添加位置】
    ASIO_DECL explicit location(const char* file,
        int line, const char* func);

    // Destructor removes a location from the stack.【析构函数，从栈中删除位置】
    ASIO_DECL ~location();

  private:
    // Disallow copying and assignment.【位置当然不能拷贝或赋值啦】
    location(const location&) ASIO_DELETED;
    location& operator=(const location&) ASIO_DELETED;

    friend class handler_tracking;      //【友元:只能由主类 handler_tracking访问】
    const char* file_;                  //文件名
    int line_;                          //当前行数
    const char* func_;                  //当前的函数名
    location* next_;                    //下一个位置指针
  };

  /// ==========================================================================================================
  // Record the creation of a tracked handler.【记录被追踪的句柄的创造过程】
  ASIO_DECL static void creation(
      execution_context& context, tracked_handler& h,
      const char* object_type, void* object,
      uintmax_t native_handle, const char* op_name);

  /// ==========================================================================================================
  // 记录句柄被唤醒后的执行过程
  class completion
  {
  public:
    // Constructor records that handler is to be invoked with no arguments.【构造函数记录下句柄没有参数时被唤醒的过程】
    ASIO_DECL explicit completion(const tracked_handler& h);

    // Destructor records only when an exception is thrown from the handler, or
    // if the memory is being freed without the handler having been invoked.【析构函数只在xxx记录】【1.句柄抛出异常 2.内存在句柄没有唤醒时就被释放】
    ASIO_DECL ~completion();

    // Records that handler is to be invoked with no arguments.【记录句柄无参被唤醒】
    ASIO_DECL void invocation_begin();

    // Records that handler is to be invoked with one arguments.【记录句柄单参唤醒】
    ASIO_DECL void invocation_begin(const asio::error_code& ec);

    // Constructor records that handler is to be invoked with two arguments.【记录句柄双参唤醒】[error_code, size_t]
    ASIO_DECL void invocation_begin(
        const asio::error_code& ec, std::size_t bytes_transferred);

    // Constructor records that handler is to be invoked with two arguments.【记录句柄双参唤醒】[error_code, int]
    ASIO_DECL void invocation_begin(
        const asio::error_code& ec, int signal_number);

    // Constructor records that handler is to be invoked with two arguments.【记录句柄双参唤醒】[error_code, const char*]
    ASIO_DECL void invocation_begin(
        const asio::error_code& ec, const char* arg);

    // Record that handler invocation has ended.【结束记录】
    ASIO_DECL void invocation_end();

  private:
    friend class handler_tracking;  //一样，只有主类可以查看id_
    uint64_t id_;                   //上层
    bool invoked_;                  //被唤醒了嘛
    completion* next_;              //下一个地址
  };

  /// ==========================================================================================================
  // Record an operation that is not directly associated with a handler.【记录一个操作[与句柄没啥直接联系的]】[执行上下文, 对象类型, 对象指针, 本地句柄u64, 操作名字符串]
  ASIO_DECL static void operation(execution_context& context,
      const char* object_type, void* object,
      uintmax_t native_handle, const char* op_name);

  // Record that a descriptor has been registered with the reactor.【记录描述符被注册到反应器reactor上的过程】
  ASIO_DECL static void reactor_registration(execution_context& context,
      uintmax_t native_handle, uintmax_t registration);

  // Record that a descriptor has been deregistered from the reactor.【记录描述符从reactor上解除注册的过程】
  ASIO_DECL static void reactor_deregistration(execution_context& context,
      uintmax_t native_handle, uintmax_t registration);

  // Record a reactor-based operation that is associated with a handler.【记录和句柄相关的基于reactor的操作】
  ASIO_DECL static void reactor_events(execution_context& context,
      uintmax_t registration, unsigned events);

  // Record a reactor-based operation that is associated with a handler.【记录和句柄相关的基于reactor的操作】
  ASIO_DECL static void reactor_operation(
      const tracked_handler& h, const char* op_name,
      const asio::error_code& ec);

  // Record a reactor-based operation that is associated with a handler.【记录和句柄相关的基于reactor的操作】
  ASIO_DECL static void reactor_operation(
      const tracked_handler& h, const char* op_name,
      const asio::error_code& ec, std::size_t bytes_transferred);

  // Write a line of output.    【向输出写一条记录】
  ASIO_DECL static void write_line(const char* format, ...);

private:
  struct tracking_state;            //h中声明，追溯状态
  ASIO_DECL static tracking_state* get_state();     //static，获得当前的追踪状态
};  

/// ==========================================================================================================
# define ASIO_INHERIT_TRACKED_HANDLER \
  : public asio::detail::handler_tracking::tracked_handler

# define ASIO_ALSO_INHERIT_TRACKED_HANDLER \
  , public asio::detail::handler_tracking::tracked_handler

# define ASIO_HANDLER_TRACKING_INIT \
  asio::detail::handler_tracking::init()

/// ==================================================================
// 这个宏在使用的时候,args外面得加一层括号，因为实际上这个替换后是创建了一个对象
# define ASIO_HANDLER_LOCATION(args) \
  asio::detail::handler_tracking::location tracked_location args

# define ASIO_HANDLER_CREATION(args) \
  asio::detail::handler_tracking::creation args

# define ASIO_HANDLER_COMPLETION(args) \
  asio::detail::handler_tracking::completion tracked_completion args

# define ASIO_HANDLER_INVOCATION_BEGIN(args) \
  tracked_completion.invocation_begin args

# define ASIO_HANDLER_INVOCATION_END \
  tracked_completion.invocation_end()

# define ASIO_HANDLER_OPERATION(args) \
  asio::detail::handler_tracking::operation args

# define ASIO_HANDLER_REACTOR_REGISTRATION(args) \
  asio::detail::handler_tracking::reactor_registration args

# define ASIO_HANDLER_REACTOR_DEREGISTRATION(args) \
  asio::detail::handler_tracking::reactor_deregistration args

/// ==================================================================
// 几个宏常量，看看是什么反应事件
# define ASIO_HANDLER_REACTOR_READ_EVENT 1
# define ASIO_HANDLER_REACTOR_WRITE_EVENT 2
# define ASIO_HANDLER_REACTOR_ERROR_EVENT 4

/// ==================================================================
# define ASIO_HANDLER_REACTOR_EVENTS(args) \
  asio::detail::handler_tracking::reactor_events args

# define ASIO_HANDLER_REACTOR_OPERATION(args) \
  asio::detail::handler_tracking::reactor_operation args

#else // defined(ASIO_ENABLE_HANDLER_TRACKING)

# define ASIO_INHERIT_TRACKED_HANDLER
# define ASIO_ALSO_INHERIT_TRACKED_HANDLER
# define ASIO_HANDLER_TRACKING_INIT (void)0
# define ASIO_HANDLER_LOCATION(loc) (void)0
# define ASIO_HANDLER_CREATION(args) (void)0
# define ASIO_HANDLER_COMPLETION(args) (void)0
# define ASIO_HANDLER_INVOCATION_BEGIN(args) (void)0
# define ASIO_HANDLER_INVOCATION_END (void)0
# define ASIO_HANDLER_OPERATION(args) (void)0
# define ASIO_HANDLER_REACTOR_REGISTRATION(args) (void)0
# define ASIO_HANDLER_REACTOR_DEREGISTRATION(args) (void)0
# define ASIO_HANDLER_REACTOR_READ_EVENT 0
# define ASIO_HANDLER_REACTOR_WRITE_EVENT 0
# define ASIO_HANDLER_REACTOR_ERROR_EVENT 0
# define ASIO_HANDLER_REACTOR_EVENTS(args) (void)0
# define ASIO_HANDLER_REACTOR_OPERATION(args) (void)0

#endif // defined(ASIO_ENABLE_HANDLER_TRACKING)

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "asio/detail/impl/handler_tracking.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_DETAIL_HANDLER_TRACKING_HPP
