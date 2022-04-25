//
// TAKO:�������
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

class execution_context;    //ǰ��������ִ��������

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
// ���û��Զ����ͷ�ļ������������ĺ꡿
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
// ֻ��������������ʱ�Ŷ����

class handler_tracking
{
public:
  class completion;

  /// ==========================================================================================================
  // Base class for objects containing tracked handlers. ��������׷�ٵľ���Ļ��ࡿ
  class tracked_handler
  {
  private:
    // Only the handler_tracking class will have access to the id. ��ֻ�о��׷�������Ȩ�޷���id_��
    friend class handler_tracking;
    friend class completion;
    uint64_t id_;   //u64 id

  protected:
    // Constructor initialises with no id. ��Ĭ�Ϲ��캯����idΪ0��
    tracked_handler() : id_(0) {}

    // Prevent deletion through this type. ����������protected��
    ~tracked_handler() {}
  };

  // Initialise the tracking system.
  ASIO_DECL static void init();

  /// ==========================================================================================================
  // λ��
  class location
  {
  public:
    // Constructor adds a location to the stack.�����캯������ջ�����λ�á�
    ASIO_DECL explicit location(const char* file,
        int line, const char* func);

    // Destructor removes a location from the stack.��������������ջ��ɾ��λ�á�
    ASIO_DECL ~location();

  private:
    // Disallow copying and assignment.��λ�õ�Ȼ���ܿ�����ֵ����
    location(const location&) ASIO_DELETED;
    location& operator=(const location&) ASIO_DELETED;

    friend class handler_tracking;      //����Ԫ:ֻ�������� handler_tracking���ʡ�
    const char* file_;                  //�ļ���
    int line_;                          //��ǰ����
    const char* func_;                  //��ǰ�ĺ�����
    location* next_;                    //��һ��λ��ָ��
  };

  /// ==========================================================================================================
  // Record the creation of a tracked handler.����¼��׷�ٵľ���Ĵ�����̡�
  ASIO_DECL static void creation(
      execution_context& context, tracked_handler& h,
      const char* object_type, void* object,
      uintmax_t native_handle, const char* op_name);

  /// ==========================================================================================================
  // ��¼��������Ѻ��ִ�й���
  class completion
  {
  public:
    // Constructor records that handler is to be invoked with no arguments.�����캯����¼�¾��û�в���ʱ�����ѵĹ��̡�
    ASIO_DECL explicit completion(const tracked_handler& h);

    // Destructor records only when an exception is thrown from the handler, or
    // if the memory is being freed without the handler having been invoked.����������ֻ��xxx��¼����1.����׳��쳣 2.�ڴ��ھ��û�л���ʱ�ͱ��ͷš�
    ASIO_DECL ~completion();

    // Records that handler is to be invoked with no arguments.����¼����޲α����ѡ�
    ASIO_DECL void invocation_begin();

    // Records that handler is to be invoked with one arguments.����¼������λ��ѡ�
    ASIO_DECL void invocation_begin(const asio::error_code& ec);

    // Constructor records that handler is to be invoked with two arguments.����¼���˫�λ��ѡ�[error_code, size_t]
    ASIO_DECL void invocation_begin(
        const asio::error_code& ec, std::size_t bytes_transferred);

    // Constructor records that handler is to be invoked with two arguments.����¼���˫�λ��ѡ�[error_code, int]
    ASIO_DECL void invocation_begin(
        const asio::error_code& ec, int signal_number);

    // Constructor records that handler is to be invoked with two arguments.����¼���˫�λ��ѡ�[error_code, const char*]
    ASIO_DECL void invocation_begin(
        const asio::error_code& ec, const char* arg);

    // Record that handler invocation has ended.��������¼��
    ASIO_DECL void invocation_end();

  private:
    friend class handler_tracking;  //һ����ֻ��������Բ鿴id_
    uint64_t id_;                   //�ϲ�
    bool invoked_;                  //����������
    completion* next_;              //��һ����ַ
  };

  /// ==========================================================================================================
  // Record an operation that is not directly associated with a handler.����¼һ������[����ûɶֱ����ϵ��]��[ִ��������, ��������, ����ָ��, ���ؾ��u64, �������ַ���]
  ASIO_DECL static void operation(execution_context& context,
      const char* object_type, void* object,
      uintmax_t native_handle, const char* op_name);

  // Record that a descriptor has been registered with the reactor.����¼��������ע�ᵽ��Ӧ��reactor�ϵĹ��̡�
  ASIO_DECL static void reactor_registration(execution_context& context,
      uintmax_t native_handle, uintmax_t registration);

  // Record that a descriptor has been deregistered from the reactor.����¼��������reactor�Ͻ��ע��Ĺ��̡�
  ASIO_DECL static void reactor_deregistration(execution_context& context,
      uintmax_t native_handle, uintmax_t registration);

  // Record a reactor-based operation that is associated with a handler.����¼�;����صĻ���reactor�Ĳ�����
  ASIO_DECL static void reactor_events(execution_context& context,
      uintmax_t registration, unsigned events);

  // Record a reactor-based operation that is associated with a handler.����¼�;����صĻ���reactor�Ĳ�����
  ASIO_DECL static void reactor_operation(
      const tracked_handler& h, const char* op_name,
      const asio::error_code& ec);

  // Record a reactor-based operation that is associated with a handler.����¼�;����صĻ���reactor�Ĳ�����
  ASIO_DECL static void reactor_operation(
      const tracked_handler& h, const char* op_name,
      const asio::error_code& ec, std::size_t bytes_transferred);

  // Write a line of output.    �������дһ����¼��
  ASIO_DECL static void write_line(const char* format, ...);

private:
  struct tracking_state;            //h��������׷��״̬
  ASIO_DECL static tracking_state* get_state();     //static����õ�ǰ��׷��״̬
};  

/// ==========================================================================================================
# define ASIO_INHERIT_TRACKED_HANDLER \
  : public asio::detail::handler_tracking::tracked_handler

# define ASIO_ALSO_INHERIT_TRACKED_HANDLER \
  , public asio::detail::handler_tracking::tracked_handler

# define ASIO_HANDLER_TRACKING_INIT \
  asio::detail::handler_tracking::init()

/// ==================================================================
// �������ʹ�õ�ʱ��,args����ü�һ�����ţ���Ϊʵ��������滻���Ǵ�����һ������
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
// �����곣����������ʲô��Ӧ�¼�
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
