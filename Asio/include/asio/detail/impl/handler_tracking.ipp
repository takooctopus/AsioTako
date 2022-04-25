//
// TAKO: 句柄追踪相关函数和类实现
// detail/impl/handler_tracking.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IMPL_HANDLER_TRACKING_IPP
#define ASIO_DETAIL_IMPL_HANDLER_TRACKING_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_CUSTOM_HANDLER_TRACKING)

// The handler tracking implementation is provided by the user-specified header.

#elif defined(ASIO_ENABLE_HANDLER_TRACKING)

#include <cstdarg>
#include <cstdio>
#include "asio/detail/handler_tracking.hpp" //头文件

#if defined(ASIO_HAS_BOOST_DATE_TIME)
# include "asio/time_traits.hpp"
#elif defined(ASIO_HAS_CHRONO)
# include "asio/detail/chrono.hpp"
# include "asio/detail/chrono_time_traits.hpp"
# include "asio/wait_traits.hpp"
#endif // defined(ASIO_HAS_BOOST_DATE_TIME)

#if defined(ASIO_WINDOWS_RUNTIME)
# include "asio/detail/socket_types.hpp"    //socket相关的常量
#elif !defined(ASIO_WINDOWS)
# include <unistd.h>
#endif // !defined(ASIO_WINDOWS)

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

/// =============================================================================================================================================================
// 句柄追踪时间戳
struct handler_tracking_timestamp
{
  uint64_t seconds;             //u64的秒
  uint64_t microseconds;        //u64的微秒

  handler_tracking_timestamp()
  {
#if defined(ASIO_HAS_BOOST_DATE_TIME)
    boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
    boost::posix_time::time_duration now =
      boost::posix_time::microsec_clock::universal_time() - epoch;
#elif defined(ASIO_HAS_CHRONO)
    typedef chrono_time_traits<chrono::system_clock,
        asio::wait_traits<chrono::system_clock> > traits_helper;
    traits_helper::posix_time_duration now(
        chrono::system_clock::now().time_since_epoch());
#endif
    // 拿到时间戳
    seconds = static_cast<uint64_t>(now.total_seconds());
    microseconds = static_cast<uint64_t>(now.total_microseconds() % 1000000);
  }
};

/// =============================================================================================================================================================
// 定义的子类 追溯状态
struct handler_tracking::tracking_state
{
  static_mutex mutex_;                              //一个互斥锁
  uint64_t next_id_;                                //下一个id
  tss_ptr<completion>* current_completion_;         //一个线程本地存储指针，存当前的步骤
  tss_ptr<location>* current_location_;             //一个线程本地存储指针，存当前的位置
};

/// =======================================================================================
handler_tracking::tracking_state* handler_tracking::get_state()     //拿到追溯状态
{   
  static tracking_state state = { ASIO_STATIC_MUTEX_INIT, 1, 0, 0 };    //主要是static的，第一次进来要初始化
  return &state;
}

/// =======================================================================================
void handler_tracking::init()   //初始化
{
  static tracking_state* state = get_state();   //拿到static的追溯状态对象   

  state->mutex_.init(); //初始化互斥锁

  static_mutex::scoped_lock lock(state->mutex_);    //在初始化时将拿到互斥锁所有权
  if (state->current_completion_ == 0)              //先看看当前追溯状态里有没有completion[这个类就是用来记录句柄完成时的执行过程]
    state->current_completion_ = new tss_ptr<completion>;   //要是没有，就新建一个线程本地存储，将completion存进去
  if (state->current_location_ == 0)
    state->current_location_ = new tss_ptr<location>;   //同样location也存到线程本地存储里去
}

/// =======================================================================================
handler_tracking::location::location(
    const char* file, int line, const char* func)   //位置，传入参数[文件名，行数，函数名]
  : file_(file),
    line_(line),
    func_(func),
    next_(*get_state()->current_location_)  //一样的插入最上层
{
  if (file_)
    *get_state()->current_location_ = this; //如果有文件名，将追溯状态中的当前位置设置为新建的location指针
}

/// =======================================================================================
handler_tracking::location::~location() //位置对象的析构
{
  if (file_)
    *get_state()->current_location_ = next_;    //如果有文件名，出栈，设置追溯状态中的相应字段
}

/// =======================================================================================
void handler_tracking::creation(execution_context&,
    handler_tracking::tracked_handler& h,
    const char* object_type, void* object,
    uintmax_t /*native_handle*/, const char* op_name)
{
  static tracking_state* state = get_state();       //拿到static的追溯状态

  static_mutex::scoped_lock lock(state->mutex_);    //用scoped_lock管理互斥锁
  h.id_ = state->next_id_++;                        //分配给h一个id，同时追溯状态的next_id自增
  lock.unlock();                                    //解锁

  handler_tracking_timestamp timestamp;             //获得当前时间的时间戳

  uint64_t current_id = 0;                          //新开一个本地变量id设置为0
  if (completion* current_completion = *state->current_completion_) //如果当前追溯状态中的completion不为nullptr
    current_id = current_completion->id_;                           //赋值当前的completion 【】

  for (location* current_location = *state->current_location_;  // 【依次看栈位置】
      current_location; current_location = current_location->next_)
  {
    write_line(
#if defined(ASIO_WINDOWS)
        "@asio|%I64u.%06I64u|%I64u^%I64u|%s%s%.80s%s(%.80s:%d)\n",
#else // defined(ASIO_WINDOWS)
        "@asio|%llu.%06llu|%llu^%llu|%s%s%.80s%s(%.80s:%d)\n",
#endif // defined(ASIO_WINDOWS)
        timestamp.seconds, timestamp.microseconds,                                      //时间戳
        current_id, h.id_,                                                              //当前id，句柄id
        current_location == *state->current_location_ ? "in " : "called from ",         //如果追溯状态与当前位置相同，就是in，否则就是从上层调用的
        current_location->func_ ? "'" : "",                                             //
        current_location->func_ ? current_location->func_ : "",                         //当前函数
        current_location->func_ ? "' " : "",                                            //
        current_location->file_, current_location->line_);                              //文件和行数
  }

  write_line(
#if defined(ASIO_WINDOWS)
      "@asio|%I64u.%06I64u|%I64u*%I64u|%.20s@%p.%.50s\n",
#else // defined(ASIO_WINDOWS)
      "@asio|%llu.%06llu|%llu*%llu|%.20s@%p.%.50s\n",
#endif // defined(ASIO_WINDOWS)
      timestamp.seconds, timestamp.microseconds,                                        //时间戳
      current_id, h.id_, object_type, object, op_name);                                 //当前id，句柄id，对象类型，操作名
}

/// =======================================================================================
// 构建函数
handler_tracking::completion::completion(
    const handler_tracking::tracked_handler& h)     //传入被追踪的句柄
  : id_(h.id_),             //句柄id[u64]
    invoked_(false),        //默认是没有被调用的
    next_(*get_state()->current_completion_)    //看样子这个链表最新的在最前面
{
  *get_state()->current_completion_ = this; //将当前static的追溯状态的对应completion指针设置为新创建的completion
}

handler_tracking::completion::~completion()
{
  if (id_)
  {
    handler_tracking_timestamp timestamp;

    write_line(
#if defined(ASIO_WINDOWS)
        "@asio|%I64u.%06I64u|%c%I64u|\n",
#else // defined(ASIO_WINDOWS)
        "@asio|%llu.%06llu|%c%llu|\n",
#endif // defined(ASIO_WINDOWS)
        timestamp.seconds, timestamp.microseconds,
        invoked_ ? '!' : '~', id_);
  }

  *get_state()->current_completion_ = next_;
}

/// =======================================================================================
//开始调用函数
void handler_tracking::completion::invocation_begin()
{
  handler_tracking_timestamp timestamp; //先初始化一个时间戳

  write_line(
#if defined(ASIO_WINDOWS)
      "@asio|%I64u.%06I64u|>%I64u|\n",
#else // defined(ASIO_WINDOWS)
      "@asio|%llu.%06llu|>%llu|\n",
#endif // defined(ASIO_WINDOWS)
      timestamp.seconds, timestamp.microseconds, id_);  //写记录[时间戳，id]

  invoked_ = true;          //设置开始调用了invoked_
}

/// =======================================================================================
void handler_tracking::completion::invocation_begin(
    const asio::error_code& ec)             //单参数的开始
{
  handler_tracking_timestamp timestamp;

  write_line(
#if defined(ASIO_WINDOWS)
      "@asio|%I64u.%06I64u|>%I64u|ec=%.20s:%d\n",
#else // defined(ASIO_WINDOWS)
      "@asio|%llu.%06llu|>%llu|ec=%.20s:%d\n",
#endif // defined(ASIO_WINDOWS)
      timestamp.seconds, timestamp.microseconds,
      id_, ec.category().name(), ec.value());           //还要输出错误码的类型和值

  invoked_ = true;          //表示已经唤醒
}

/// =======================================================================================
void handler_tracking::completion::invocation_begin(
    const asio::error_code& ec, std::size_t bytes_transferred)  //双参数的开始
{
  handler_tracking_timestamp timestamp;

  write_line(
#if defined(ASIO_WINDOWS)
      "@asio|%I64u.%06I64u|>%I64u|ec=%.20s:%d,bytes_transferred=%I64u\n",
#else // defined(ASIO_WINDOWS)
      "@asio|%llu.%06llu|>%llu|ec=%.20s:%d,bytes_transferred=%llu\n",
#endif // defined(ASIO_WINDOWS)
      timestamp.seconds, timestamp.microseconds,
      id_, ec.category().name(), ec.value(),
      static_cast<uint64_t>(bytes_transferred));        //再加上一个传输字节数

  invoked_ = true;
}

/// =======================================================================================
void handler_tracking::completion::invocation_begin(
    const asio::error_code& ec, int signal_number)
{
  handler_tracking_timestamp timestamp;

  write_line(
#if defined(ASIO_WINDOWS)
      "@asio|%I64u.%06I64u|>%I64u|ec=%.20s:%d,signal_number=%d\n",
#else // defined(ASIO_WINDOWS)
      "@asio|%llu.%06llu|>%llu|ec=%.20s:%d,signal_number=%d\n",
#endif // defined(ASIO_WINDOWS)
      timestamp.seconds, timestamp.microseconds,
      id_, ec.category().name(), ec.value(), signal_number);

  invoked_ = true;
}

/// =======================================================================================
void handler_tracking::completion::invocation_begin(
    const asio::error_code& ec, const char* arg)
{
  handler_tracking_timestamp timestamp;

  write_line(
#if defined(ASIO_WINDOWS)
      "@asio|%I64u.%06I64u|>%I64u|ec=%.20s:%d,%.50s\n",
#else // defined(ASIO_WINDOWS)
      "@asio|%llu.%06llu|>%llu|ec=%.20s:%d,%.50s\n",
#endif // defined(ASIO_WINDOWS)
      timestamp.seconds, timestamp.microseconds,
      id_, ec.category().name(), ec.value(), arg);

  invoked_ = true;
}

/// =======================================================================================
//结束调用
void handler_tracking::completion::invocation_end()
{
  if (id_)      //如果有id_才进行结束
  {
    handler_tracking_timestamp timestamp;

    write_line(
#if defined(ASIO_WINDOWS)
        "@asio|%I64u.%06I64u|<%I64u|\n",
#else // defined(ASIO_WINDOWS)
        "@asio|%llu.%06llu|<%llu|\n",
#endif // defined(ASIO_WINDOWS)
        timestamp.seconds, timestamp.microseconds, id_);    //记录一个时间戳

    id_ = 0;
  }
}

/// =======================================================================================
void handler_tracking::operation(execution_context&,
    const char* object_type, void* object,
    uintmax_t /*native_handle*/, const char* op_name)
{
  static tracking_state* state = get_state();       //获取当前追溯状态

  handler_tracking_timestamp timestamp;     //时间戳

  unsigned long long current_id = 0;        
  if (completion* current_completion = *state->current_completion_)
    current_id = current_completion->id_;       //拿到当前id

  write_line(
#if defined(ASIO_WINDOWS)
      "@asio|%I64u.%06I64u|%I64u|%.20s@%p.%.50s\n",
#else // defined(ASIO_WINDOWS)
      "@asio|%llu.%06llu|%llu|%.20s@%p.%.50s\n",
#endif // defined(ASIO_WINDOWS)
      timestamp.seconds, timestamp.microseconds,
      current_id, object_type, object, op_name);
}

/// =======================================================================================
void handler_tracking::reactor_registration(execution_context& /*context*/,
    uintmax_t /*native_handle*/, uintmax_t /*registration*/)
{
}

/// =======================================================================================
void handler_tracking::reactor_deregistration(execution_context& /*context*/,
    uintmax_t /*native_handle*/, uintmax_t /*registration*/)
{
}

/// =======================================================================================
void handler_tracking::reactor_events(execution_context& /*context*/,
    uintmax_t /*native_handle*/, unsigned /*events*/)
{
}

/// =======================================================================================
void handler_tracking::reactor_operation(
    const tracked_handler& h, const char* op_name,
    const asio::error_code& ec)
{
  handler_tracking_timestamp timestamp;

  write_line(
#if defined(ASIO_WINDOWS)
      "@asio|%I64u.%06I64u|.%I64u|%s,ec=%.20s:%d\n",
#else // defined(ASIO_WINDOWS)
      "@asio|%llu.%06llu|.%llu|%s,ec=%.20s:%d\n",
#endif // defined(ASIO_WINDOWS)
      timestamp.seconds, timestamp.microseconds,
      h.id_, op_name, ec.category().name(), ec.value());
}

/// =======================================================================================
void handler_tracking::reactor_operation(
    const tracked_handler& h, const char* op_name,
    const asio::error_code& ec, std::size_t bytes_transferred)
{
  handler_tracking_timestamp timestamp;

  write_line(
#if defined(ASIO_WINDOWS)
      "@asio|%I64u.%06I64u|.%I64u|%s,ec=%.20s:%d,bytes_transferred=%I64u\n",
#else // defined(ASIO_WINDOWS)
      "@asio|%llu.%06llu|.%llu|%s,ec=%.20s:%d,bytes_transferred=%llu\n",
#endif // defined(ASIO_WINDOWS)
      timestamp.seconds, timestamp.microseconds,
      h.id_, op_name, ec.category().name(), ec.value(),
      static_cast<uint64_t>(bytes_transferred));
}

/// =======================================================================================
// 输出一行【到标准错误流】
void handler_tracking::write_line(const char* format, ...)
{
  using namespace std; // For sprintf (or equivalent).

  va_list args;     //展开可变参数
  va_start(args, format);

  char line[256] = "";  //缓冲区
#if defined(ASIO_HAS_SECURE_RTL)
  int length = vsprintf_s(line, sizeof(line), format, args);    //使用安全的格式化函数格式化缓冲区
#else // defined(ASIO_HAS_SECURE_RTL)
  int length = vsprintf(line, format, args);
#endif // defined(ASIO_HAS_SECURE_RTL)

  va_end(args);

#if defined(ASIO_WINDOWS_RUNTIME)
  wchar_t wline[256] = L"";
  mbstowcs_s(0, wline, sizeof(wline) / sizeof(wchar_t), line, length);
  ::OutputDebugStringW(wline);
#elif defined(ASIO_WINDOWS)
  HANDLE stderr_handle = ::GetStdHandle(STD_ERROR_HANDLE);  //标准错误流
  DWORD bytes_written = 0;
  ::WriteFile(stderr_handle, line, length, &bytes_written, 0);  //windows下的写函数
#else // defined(ASIO_WINDOWS)
  ::write(STDERR_FILENO, line, length); //写到标准错误流中
#endif // defined(ASIO_WINDOWS)
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_ENABLE_HANDLER_TRACKING)

#endif // ASIO_DETAIL_IMPL_HANDLER_TRACKING_IPP
