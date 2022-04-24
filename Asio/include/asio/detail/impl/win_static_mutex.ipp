//
// TAKO:Win下面的静态互斥锁实现
// detail/impl/win_static_mutex.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IMPL_WIN_STATIC_MUTEX_IPP
#define ASIO_DETAIL_IMPL_WIN_STATIC_MUTEX_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_WINDOWS)

#include <cstdio>
#include "asio/detail/throw_error.hpp"  //异常抛出
#include "asio/detail/win_static_mutex.hpp" //头文件定义
#include "asio/error.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

void win_static_mutex::init()
{
  int error = do_init();    //调用真正的初始化实现函数
  asio::error_code ec(error,
      asio::error::get_system_category());
  asio::detail::throw_error(ec, "static_mutex");    //出错了要抛异常
}

int win_static_mutex::do_init()
{
  using namespace std; // For sprintf.
  wchar_t mutex_name[128];      //栈上申请128大小的宽字符数组
#if defined(ASIO_HAS_SECURE_RTL)
  //sprintf_s的参数(buffer, size, 格式)最后的格式用参数有两个(当前进程id，当前类指针地址)
  swprintf_s(                                       
#else // defined(ASIO_HAS_SECURE_RTL)
  _snwprintf(
#endif // defined(ASIO_HAS_SECURE_RTL)
      mutex_name, 128, L"asio-58CCDC44-6264-4842-90C2-F3C545CB8AA7-%u-%p",
      static_cast<unsigned int>(::GetCurrentProcessId()), this);        //格式化填入互斥锁的名字

#if defined(ASIO_WINDOWS_APP)
  HANDLE mutex = ::CreateMutexExW(0, mutex_name, CREATE_MUTEX_INITIAL_OWNER, 0);
#else // defined(ASIO_WINDOWS_APP)
  HANDLE mutex = ::CreateMutexW(0, TRUE, mutex_name);   //创建互斥锁[线程同步互斥量]
#endif // defined(ASIO_WINDOWS_APP)
  DWORD last_error = ::GetLastError();
  if (mutex == 0)
    return ::GetLastError();

  if (last_error == ERROR_ALREADY_EXISTS)   //要是有同名的
  {
#if defined(ASIO_WINDOWS_APP)
    ::WaitForSingleObjectEx(mutex, INFINITE, false);
#else // defined(ASIO_WINDOWS_APP)
    ::WaitForSingleObject(mutex, INFINITE); //有了就不用创建了，一直阻塞知道获得其所有权
#endif // defined(ASIO_WINDOWS_APP)
  }

  if (initialised_) //如果已经初始化过了
  {
    ::ReleaseMutex(mutex);  //释放互斥锁
    ::CloseHandle(mutex);   //关闭互斥锁
    return 0;
  }

#if defined(__MINGW32__)
  // Not sure if MinGW supports structured exception handling, so for now
  // we'll just call the Windows API and hope.
# if defined(UNDER_CE)
  ::InitializeCriticalSection(&crit_section_);
# else
  if (!::InitializeCriticalSectionAndSpinCount(&crit_section_, 0x80000000))
  {
    last_error = ::GetLastError();
    ::ReleaseMutex(mutex);
    ::CloseHandle(mutex);
    return last_error;
  }
# endif
#else
  __try
  {
# if defined(UNDER_CE)
    ::InitializeCriticalSection(&crit_section_);
# elif defined(ASIO_WINDOWS_APP)
    if (!::InitializeCriticalSectionEx(&crit_section_, 0, 0))
    {
      last_error = ::GetLastError();
      ::ReleaseMutex(mutex);
      ::CloseHandle(mutex);
      return last_error;
    }
# else
    if (!::InitializeCriticalSectionAndSpinCount(&crit_section_, 0x80000000))   //尝试初始化一个自旋锁
    {
      last_error = ::GetLastError();
      ::ReleaseMutex(mutex);
      ::CloseHandle(mutex);
      return last_error;    //没有成功就释放资源
    }
# endif
  }
  __except(GetExceptionCode() == STATUS_NO_MEMORY      
      ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)  //捕获异常，看看是不是没有空闲内存，是就返回一个错误，不是就继续向上层搜索
  {
    ::ReleaseMutex(mutex);
    ::CloseHandle(mutex);
    return ERROR_OUTOFMEMORY;
  }
#endif

  initialised_ = true;  //初始化成功
  ::ReleaseMutex(mutex);
  ::CloseHandle(mutex);
  return 0;
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_WINDOWS)

#endif // ASIO_DETAIL_IMPL_WIN_STATIC_MUTEX_IPP
