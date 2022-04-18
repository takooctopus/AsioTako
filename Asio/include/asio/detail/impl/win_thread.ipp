//
// detail/impl/win_thread.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IMPL_WIN_THREAD_IPP
#define ASIO_DETAIL_IMPL_WIN_THREAD_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_WINDOWS) \
  && !defined(ASIO_WINDOWS_APP) \
  && !defined(UNDER_CE)

#include <process.h>
#include "asio/detail/throw_error.hpp"
#include "asio/detail/win_thread.hpp"
#include "asio/error.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

win_thread::~win_thread()
{
  ::CloseHandle(thread_);

  // The exit_event_ handle is deliberately allowed to leak here since it
  // is an error for the owner of an internal thread not to join() it.
}

void win_thread::join()
{
  HANDLE handles[2] = { exit_event_, thread_ }; //Handle句柄数组【下面函数的退出事件，线程】
  ::WaitForMultipleObjects(2, handles, FALSE, INFINITE); //【永久阻塞】等待这两个句柄[一般来说这个时候已经阻塞到下面函数执行结束了]
  ::CloseHandle(exit_event_); //阻塞完成后释放下面函数的退出事件
  if (terminate_threads())  //原子操作增加0标志【即调用默认就是走else那个分支的】
  {
      //要是里面terminate_threads_非0了，就准备出来关闭这个thread
      //非0只有在全局调用set_terminate_threads()将值设置为1时才使用这个
    ::TerminateThread(thread_, 0);  
  }
  else
  {
      // 因为函数执行完成退出之前线程默认是休眠状态
    ::QueueUserAPC(apc_function, thread_, 0);   // 增加一个apc_function，用来唤醒线程
    ::WaitForSingleObject(thread_, INFINITE);   // 【一般来说这个等待很快就结束了】
  }
}

std::size_t win_thread::hardware_concurrency()
{
  SYSTEM_INFO system_info;  //win32下面直接用SYSTEM_INFO就行了
  ::GetSystemInfo(&system_info);
  return system_info.dwNumberOfProcessors;
}

void win_thread::start_thread(func_base* arg, unsigned int stack_size)  //开始线程
{
  ::HANDLE entry_event = 0; //初始化一个HANDLE 进入消息
  arg->entry_event_ = entry_event = ::CreateEventW(0, true, false, 0); //创建Event【事件属性，手动复位，初始没有信号，事件名称】【因为进入事件实际上在进入函数后后就必定置1了，所以没啥用】
  if (!entry_event)
  {
    DWORD last_error = ::GetLastError();
    delete arg; //生成失败要删除堆对象
    asio::error_code ec(last_error,
        asio::error::get_system_category());
    asio::detail::throw_error(ec, "thread.entry_event"); //抛异常
  }

  arg->exit_event_ = exit_event_ = ::CreateEventW(0, true, false, 0);   //创建退出消息【这个退出消息要与上层thread中的退出消息共享一个】
  if (!exit_event_)
  {
    DWORD last_error = ::GetLastError();
    delete arg;
    asio::error_code ec(last_error,
        asio::error::get_system_category());
    asio::detail::throw_error(ec, "thread.exit_event");
  }

  unsigned int thread_id = 0;
  thread_ = reinterpret_cast<HANDLE>(::_beginthreadex(0,
        stack_size, win_thread_function, arg, 0, &thread_id)); //创建thread并将其转化成void*[HANDLE]，注意本来函数返回值是uintptr_t[在64平台上就是64bit]
  if (!thread_)
  {
    DWORD last_error = ::GetLastError();
    delete arg; //创建失败删除堆对象
    if (entry_event)
      ::CloseHandle(entry_event);   //如果有entry_event对象
    if (exit_event_)
      ::CloseHandle(exit_event_);   //如果有exit_event_
    asio::error_code ec(last_error,
        asio::error::get_system_category());
    asio::detail::throw_error(ec, "thread");
  }

  if (entry_event)
  {
    ::WaitForSingleObject(entry_event, INFINITE);   // 这个函数等待进入函数的事件【进入函数之前都会阻塞】
    // ...等待调用win_thread_function（）
    ::CloseHandle(entry_event); //最后要关闭这个事件
  }
}

unsigned int __stdcall win_thread_function(void* arg)
{
  win_thread::auto_func_base_ptr func = {
      static_cast<win_thread::func_base*>(arg) };   //一样，将void*的数据强转成func_base*类型的地址

  ::SetEvent(func.ptr->entry_event_); //将函数的进入事件值设为1

  func.ptr->run();  //运行函数

  // Signal that the thread has finished its work, but rather than returning go
  // to sleep to put the thread into a well known state. If the thread is being
  // joined during global object destruction then it may be killed using
  // TerminateThread (to avoid a deadlock in DllMain). Otherwise, the SleepEx
  // call will be interrupted using QueueUserAPC and the thread will shut down
  // cleanly.
  HANDLE exit_event = func.ptr->exit_event_;  //获取方法下面注册的退出事件的实例地址
  delete func.ptr;  //删除func下面的堆内存【只是删了两个地址，地址实际指向的对象没有删除】
  func.ptr = 0; //下面设为nullptr防止重复释放
  ::SetEvent(exit_event);   //相当于将thread中的exit_event_置1了
  ::SleepEx(INFINITE, TRUE); //一直休眠，直到程序调用【设置为true用来响应APC】

  return 0;
}

#if defined(WINVER) && (WINVER < 0x0500)
void __stdcall apc_function(ULONG) {}
#else
void __stdcall apc_function(ULONG_PTR) {} //一个空函数，当作APC函数，用来唤醒被阻塞的线程
#endif

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_WINDOWS)
       // && !defined(ASIO_WINDOWS_APP)
       // && !defined(UNDER_CE)

#endif // ASIO_DETAIL_IMPL_WIN_THREAD_IPP
