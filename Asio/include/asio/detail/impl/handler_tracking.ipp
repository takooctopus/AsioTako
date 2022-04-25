//
// TAKO: ���׷����غ�������ʵ��
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
#include "asio/detail/handler_tracking.hpp" //ͷ�ļ�

#if defined(ASIO_HAS_BOOST_DATE_TIME)
# include "asio/time_traits.hpp"
#elif defined(ASIO_HAS_CHRONO)
# include "asio/detail/chrono.hpp"
# include "asio/detail/chrono_time_traits.hpp"
# include "asio/wait_traits.hpp"
#endif // defined(ASIO_HAS_BOOST_DATE_TIME)

#if defined(ASIO_WINDOWS_RUNTIME)
# include "asio/detail/socket_types.hpp"    //socket��صĳ���
#elif !defined(ASIO_WINDOWS)
# include <unistd.h>
#endif // !defined(ASIO_WINDOWS)

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

/// =============================================================================================================================================================
// ���׷��ʱ���
struct handler_tracking_timestamp
{
  uint64_t seconds;             //u64����
  uint64_t microseconds;        //u64��΢��

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
    // �õ�ʱ���
    seconds = static_cast<uint64_t>(now.total_seconds());
    microseconds = static_cast<uint64_t>(now.total_microseconds() % 1000000);
  }
};

/// =============================================================================================================================================================
// ��������� ׷��״̬
struct handler_tracking::tracking_state
{
  static_mutex mutex_;                              //һ��������
  uint64_t next_id_;                                //��һ��id
  tss_ptr<completion>* current_completion_;         //һ���̱߳��ش洢ָ�룬�浱ǰ�Ĳ���
  tss_ptr<location>* current_location_;             //һ���̱߳��ش洢ָ�룬�浱ǰ��λ��
};

/// =======================================================================================
handler_tracking::tracking_state* handler_tracking::get_state()     //�õ�׷��״̬
{   
  static tracking_state state = { ASIO_STATIC_MUTEX_INIT, 1, 0, 0 };    //��Ҫ��static�ģ���һ�ν���Ҫ��ʼ��
  return &state;
}

/// =======================================================================================
void handler_tracking::init()   //��ʼ��
{
  static tracking_state* state = get_state();   //�õ�static��׷��״̬����   

  state->mutex_.init(); //��ʼ��������

  static_mutex::scoped_lock lock(state->mutex_);    //�ڳ�ʼ��ʱ���õ�����������Ȩ
  if (state->current_completion_ == 0)              //�ȿ�����ǰ׷��״̬����û��completion[��������������¼������ʱ��ִ�й���]
    state->current_completion_ = new tss_ptr<completion>;   //Ҫ��û�У����½�һ���̱߳��ش洢����completion���ȥ
  if (state->current_location_ == 0)
    state->current_location_ = new tss_ptr<location>;   //ͬ��locationҲ�浽�̱߳��ش洢��ȥ
}

/// =======================================================================================
handler_tracking::location::location(
    const char* file, int line, const char* func)   //λ�ã��������[�ļ�����������������]
  : file_(file),
    line_(line),
    func_(func),
    next_(*get_state()->current_location_)  //һ���Ĳ������ϲ�
{
  if (file_)
    *get_state()->current_location_ = this; //������ļ�������׷��״̬�еĵ�ǰλ������Ϊ�½���locationָ��
}

/// =======================================================================================
handler_tracking::location::~location() //λ�ö��������
{
  if (file_)
    *get_state()->current_location_ = next_;    //������ļ�������ջ������׷��״̬�е���Ӧ�ֶ�
}

/// =======================================================================================
void handler_tracking::creation(execution_context&,
    handler_tracking::tracked_handler& h,
    const char* object_type, void* object,
    uintmax_t /*native_handle*/, const char* op_name)
{
  static tracking_state* state = get_state();       //�õ�static��׷��״̬

  static_mutex::scoped_lock lock(state->mutex_);    //��scoped_lock��������
  h.id_ = state->next_id_++;                        //�����hһ��id��ͬʱ׷��״̬��next_id����
  lock.unlock();                                    //����

  handler_tracking_timestamp timestamp;             //��õ�ǰʱ���ʱ���

  uint64_t current_id = 0;                          //�¿�һ�����ر���id����Ϊ0
  if (completion* current_completion = *state->current_completion_) //�����ǰ׷��״̬�е�completion��Ϊnullptr
    current_id = current_completion->id_;                           //��ֵ��ǰ��completion ����

  for (location* current_location = *state->current_location_;  // �����ο�ջλ�á�
      current_location; current_location = current_location->next_)
  {
    write_line(
#if defined(ASIO_WINDOWS)
        "@asio|%I64u.%06I64u|%I64u^%I64u|%s%s%.80s%s(%.80s:%d)\n",
#else // defined(ASIO_WINDOWS)
        "@asio|%llu.%06llu|%llu^%llu|%s%s%.80s%s(%.80s:%d)\n",
#endif // defined(ASIO_WINDOWS)
        timestamp.seconds, timestamp.microseconds,                                      //ʱ���
        current_id, h.id_,                                                              //��ǰid�����id
        current_location == *state->current_location_ ? "in " : "called from ",         //���׷��״̬�뵱ǰλ����ͬ������in��������Ǵ��ϲ���õ�
        current_location->func_ ? "'" : "",                                             //
        current_location->func_ ? current_location->func_ : "",                         //��ǰ����
        current_location->func_ ? "' " : "",                                            //
        current_location->file_, current_location->line_);                              //�ļ�������
  }

  write_line(
#if defined(ASIO_WINDOWS)
      "@asio|%I64u.%06I64u|%I64u*%I64u|%.20s@%p.%.50s\n",
#else // defined(ASIO_WINDOWS)
      "@asio|%llu.%06llu|%llu*%llu|%.20s@%p.%.50s\n",
#endif // defined(ASIO_WINDOWS)
      timestamp.seconds, timestamp.microseconds,                                        //ʱ���
      current_id, h.id_, object_type, object, op_name);                                 //��ǰid�����id���������ͣ�������
}

/// =======================================================================================
// ��������
handler_tracking::completion::completion(
    const handler_tracking::tracked_handler& h)     //���뱻׷�ٵľ��
  : id_(h.id_),             //���id[u64]
    invoked_(false),        //Ĭ����û�б����õ�
    next_(*get_state()->current_completion_)    //����������������µ�����ǰ��
{
  *get_state()->current_completion_ = this; //����ǰstatic��׷��״̬�Ķ�Ӧcompletionָ������Ϊ�´�����completion
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
//��ʼ���ú���
void handler_tracking::completion::invocation_begin()
{
  handler_tracking_timestamp timestamp; //�ȳ�ʼ��һ��ʱ���

  write_line(
#if defined(ASIO_WINDOWS)
      "@asio|%I64u.%06I64u|>%I64u|\n",
#else // defined(ASIO_WINDOWS)
      "@asio|%llu.%06llu|>%llu|\n",
#endif // defined(ASIO_WINDOWS)
      timestamp.seconds, timestamp.microseconds, id_);  //д��¼[ʱ�����id]

  invoked_ = true;          //���ÿ�ʼ������invoked_
}

/// =======================================================================================
void handler_tracking::completion::invocation_begin(
    const asio::error_code& ec)             //�������Ŀ�ʼ
{
  handler_tracking_timestamp timestamp;

  write_line(
#if defined(ASIO_WINDOWS)
      "@asio|%I64u.%06I64u|>%I64u|ec=%.20s:%d\n",
#else // defined(ASIO_WINDOWS)
      "@asio|%llu.%06llu|>%llu|ec=%.20s:%d\n",
#endif // defined(ASIO_WINDOWS)
      timestamp.seconds, timestamp.microseconds,
      id_, ec.category().name(), ec.value());           //��Ҫ�������������ͺ�ֵ

  invoked_ = true;          //��ʾ�Ѿ�����
}

/// =======================================================================================
void handler_tracking::completion::invocation_begin(
    const asio::error_code& ec, std::size_t bytes_transferred)  //˫�����Ŀ�ʼ
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
      static_cast<uint64_t>(bytes_transferred));        //�ټ���һ�������ֽ���

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
//��������
void handler_tracking::completion::invocation_end()
{
  if (id_)      //�����id_�Ž��н���
  {
    handler_tracking_timestamp timestamp;

    write_line(
#if defined(ASIO_WINDOWS)
        "@asio|%I64u.%06I64u|<%I64u|\n",
#else // defined(ASIO_WINDOWS)
        "@asio|%llu.%06llu|<%llu|\n",
#endif // defined(ASIO_WINDOWS)
        timestamp.seconds, timestamp.microseconds, id_);    //��¼һ��ʱ���

    id_ = 0;
  }
}

/// =======================================================================================
void handler_tracking::operation(execution_context&,
    const char* object_type, void* object,
    uintmax_t /*native_handle*/, const char* op_name)
{
  static tracking_state* state = get_state();       //��ȡ��ǰ׷��״̬

  handler_tracking_timestamp timestamp;     //ʱ���

  unsigned long long current_id = 0;        
  if (completion* current_completion = *state->current_completion_)
    current_id = current_completion->id_;       //�õ���ǰid

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
// ���һ�С�����׼��������
void handler_tracking::write_line(const char* format, ...)
{
  using namespace std; // For sprintf (or equivalent).

  va_list args;     //չ���ɱ����
  va_start(args, format);

  char line[256] = "";  //������
#if defined(ASIO_HAS_SECURE_RTL)
  int length = vsprintf_s(line, sizeof(line), format, args);    //ʹ�ð�ȫ�ĸ�ʽ��������ʽ��������
#else // defined(ASIO_HAS_SECURE_RTL)
  int length = vsprintf(line, format, args);
#endif // defined(ASIO_HAS_SECURE_RTL)

  va_end(args);

#if defined(ASIO_WINDOWS_RUNTIME)
  wchar_t wline[256] = L"";
  mbstowcs_s(0, wline, sizeof(wline) / sizeof(wchar_t), line, length);
  ::OutputDebugStringW(wline);
#elif defined(ASIO_WINDOWS)
  HANDLE stderr_handle = ::GetStdHandle(STD_ERROR_HANDLE);  //��׼������
  DWORD bytes_written = 0;
  ::WriteFile(stderr_handle, line, length, &bytes_written, 0);  //windows�µ�д����
#else // defined(ASIO_WINDOWS)
  ::write(STDERR_FILENO, line, length); //д����׼��������
#endif // defined(ASIO_WINDOWS)
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_ENABLE_HANDLER_TRACKING)

#endif // ASIO_DETAIL_IMPL_HANDLER_TRACKING_IPP
