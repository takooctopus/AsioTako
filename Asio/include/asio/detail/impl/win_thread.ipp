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
  HANDLE handles[2] = { exit_event_, thread_ }; //Handle������顾���溯�����˳��¼����̡߳�
  ::WaitForMultipleObjects(2, handles, FALSE, INFINITE); //�������������ȴ����������[һ����˵���ʱ���Ѿ����������溯��ִ�н�����]
  ::CloseHandle(exit_event_); //������ɺ��ͷ����溯�����˳��¼�
  if (terminate_threads())  //ԭ�Ӳ�������0��־��������Ĭ�Ͼ�����else�Ǹ���֧�ġ�
  {
      //Ҫ������terminate_threads_��0�ˣ���׼�������ر����thread
      //��0ֻ����ȫ�ֵ���set_terminate_threads()��ֵ����Ϊ1ʱ��ʹ�����
    ::TerminateThread(thread_, 0);  
  }
  else
  {
      // ��Ϊ����ִ������˳�֮ǰ�߳�Ĭ��������״̬
    ::QueueUserAPC(apc_function, thread_, 0);   // ����һ��apc_function�����������߳�
    ::WaitForSingleObject(thread_, INFINITE);   // ��һ����˵����ȴ��ܿ�ͽ����ˡ�
  }
}

std::size_t win_thread::hardware_concurrency()
{
  SYSTEM_INFO system_info;  //win32����ֱ����SYSTEM_INFO������
  ::GetSystemInfo(&system_info);
  return system_info.dwNumberOfProcessors;
}

void win_thread::start_thread(func_base* arg, unsigned int stack_size)  //��ʼ�߳�
{
  ::HANDLE entry_event = 0; //��ʼ��һ��HANDLE ������Ϣ
  arg->entry_event_ = entry_event = ::CreateEventW(0, true, false, 0); //����Event���¼����ԣ��ֶ���λ����ʼû���źţ��¼����ơ�����Ϊ�����¼�ʵ�����ڽ��뺯�����ͱض���1�ˣ�����ûɶ�á�
  if (!entry_event)
  {
    DWORD last_error = ::GetLastError();
    delete arg; //����ʧ��Ҫɾ���Ѷ���
    asio::error_code ec(last_error,
        asio::error::get_system_category());
    asio::detail::throw_error(ec, "thread.entry_event"); //���쳣
  }

  arg->exit_event_ = exit_event_ = ::CreateEventW(0, true, false, 0);   //�����˳���Ϣ������˳���ϢҪ���ϲ�thread�е��˳���Ϣ����һ����
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
        stack_size, win_thread_function, arg, 0, &thread_id)); //����thread������ת����void*[HANDLE]��ע�Ȿ����������ֵ��uintptr_t[��64ƽ̨�Ͼ���64bit]
  if (!thread_)
  {
    DWORD last_error = ::GetLastError();
    delete arg; //����ʧ��ɾ���Ѷ���
    if (entry_event)
      ::CloseHandle(entry_event);   //�����entry_event����
    if (exit_event_)
      ::CloseHandle(exit_event_);   //�����exit_event_
    asio::error_code ec(last_error,
        asio::error::get_system_category());
    asio::detail::throw_error(ec, "thread");
  }

  if (entry_event)
  {
    ::WaitForSingleObject(entry_event, INFINITE);   // ��������ȴ����뺯�����¼������뺯��֮ǰ����������
    // ...�ȴ�����win_thread_function����
    ::CloseHandle(entry_event); //���Ҫ�ر�����¼�
  }
}

unsigned int __stdcall win_thread_function(void* arg)
{
  win_thread::auto_func_base_ptr func = {
      static_cast<win_thread::func_base*>(arg) };   //һ������void*������ǿת��func_base*���͵ĵ�ַ

  ::SetEvent(func.ptr->entry_event_); //�������Ľ����¼�ֵ��Ϊ1

  func.ptr->run();  //���к���

  // Signal that the thread has finished its work, but rather than returning go
  // to sleep to put the thread into a well known state. If the thread is being
  // joined during global object destruction then it may be killed using
  // TerminateThread (to avoid a deadlock in DllMain). Otherwise, the SleepEx
  // call will be interrupted using QueueUserAPC and the thread will shut down
  // cleanly.
  HANDLE exit_event = func.ptr->exit_event_;  //��ȡ��������ע����˳��¼���ʵ����ַ
  delete func.ptr;  //ɾ��func����Ķ��ڴ桾ֻ��ɾ��������ַ����ַʵ��ָ��Ķ���û��ɾ����
  func.ptr = 0; //������Ϊnullptr��ֹ�ظ��ͷ�
  ::SetEvent(exit_event);   //�൱�ڽ�thread�е�exit_event_��1��
  ::SleepEx(INFINITE, TRUE); //һֱ���ߣ�ֱ��������á�����Ϊtrue������ӦAPC��

  return 0;
}

#if defined(WINVER) && (WINVER < 0x0500)
void __stdcall apc_function(ULONG) {}
#else
void __stdcall apc_function(ULONG_PTR) {} //һ���պ���������APC�������������ѱ��������߳�
#endif

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_WINDOWS)
       // && !defined(ASIO_WINDOWS_APP)
       // && !defined(UNDER_CE)

#endif // ASIO_DETAIL_IMPL_WIN_THREAD_IPP
