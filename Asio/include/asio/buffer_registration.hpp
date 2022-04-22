//
// TAKO:buffer��ע�����
// buffer_registration.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_BUFFER_REGISTRATION_HPP
#define ASIO_BUFFER_REGISTRATION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <iterator>
#include <vector>
#include "asio/detail/memory.hpp"
#include "asio/execution/context.hpp"
#include "asio/execution/executor.hpp"
#include "asio/execution_context.hpp"
#include "asio/is_executor.hpp"
#include "asio/query.hpp"
#include "asio/registered_buffer.hpp"

#if defined(ASIO_HAS_IO_URING)
# include "asio/detail/scheduler.hpp"
# include "asio/detail/io_uring_service.hpp"
#endif // defined(ASIO_HAS_IO_URING)

#if defined(ASIO_HAS_MOVE)
# include <utility>
#endif // defined(ASIO_HAS_MOVE)

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class buffer_registration_base  //bufferע������࣬�����һ��[static]������������������ֻ��ͨ������������л��������вι�����
{
protected:
  static mutable_registered_buffer make_buffer(const mutable_buffer& b,
      const void* scope, int index) ASIO_NOEXCEPT
  {
    return mutable_registered_buffer(b, registered_buffer_id(scope, index));
  }
};

} // namespace detail

/// Automatically registers and unregistered buffers with an execution context.
/// ��ʹ��ִ�����������Զ��Ļ�����ע��ͽ��ע�᡿
/**
 * For portability, applications should assume that only one registration is
 * permitted per execution context.
 * ��Ϊ����ֲ�ԣ�Ӧ��Ӧ�ñ�֤ÿһ��ִ��������ֻ����һ��ע����̡�
 */
template <typename MutableBufferSequence,
    typename Allocator = std::allocator<void> > //<>������ǿɱ仺���������࣬���ڴ������
class buffer_registration
  : detail::buffer_registration_base    //�̳��������Ԫ��ӿڡ�ֻ��ͨ��������ܽ���ע�Ỻ�������вι���(������һ���Ϸ���id)��
{
public:
  /// The allocator type used for allocating storage for the buffers container. ���ڴ��������allocator��
  typedef Allocator allocator_type;

#if defined(GENERATING_DOCUMENTATION)
  /// The type of an iterator over the registered buffers.
  typedef unspecified iterator;

  /// The type of a const iterator over the registered buffers.
  typedef unspecified const_iterator;
#else // defined(GENERATING_DOCUMENTATION)
  typedef std::vector<mutable_registered_buffer>::const_iterator iterator;  //vector�ĵ�����
  typedef std::vector<mutable_registered_buffer>::const_iterator const_iterator;    //vector�ĳ�������
#endif // defined(GENERATING_DOCUMENTATION)

  /// Register buffers with an executor's execution context.
  template <typename Executor>
  buffer_registration(const Executor& ex,               //������ִ�С�
      const MutableBufferSequence& buffer_sequence,
      const allocator_type& alloc = allocator_type(),
      typename constraint<
        is_executor<Executor>::value || execution::is_executor<Executor>::value
      >::type = 0)
    : buffer_sequence_(buffer_sequence),
      buffers_(
          ASIO_REBIND_ALLOC(allocator_type,
            mutable_registered_buffer)(alloc))
  {
    init_buffers(buffer_registration::get_context(ex),      //�����ȡ��ִ��������
        asio::buffer_sequence_begin(buffer_sequence_),
        asio::buffer_sequence_end(buffer_sequence_));
  }

  /// Register buffers with an execution context.
  template <typename ExecutionContext>
  buffer_registration(ExecutionContext& ctx,    //��1.����һ��ִ�������ġ�
      const MutableBufferSequence& buffer_sequence, //��2.����һ�����������С�
      const allocator_type& alloc = allocator_type(),   //��3.�����ڴ�ķ�����
      typename constraint<
        is_convertible<ExecutionContext&, execution_context&>::value
      >::type = 0)  //��4.Լ����ʵ������һ���̳���enable_if�Ľṹ�壬Ҫ����ת��������Ĭ���������Ϊint������Ϊ0��
    : buffer_sequence_(buffer_sequence),
      buffers_(
          ASIO_REBIND_ALLOC(allocator_type,
            mutable_registered_buffer)(alloc))
  {
    init_buffers(ctx,
        asio::buffer_sequence_begin(buffer_sequence_),
        asio::buffer_sequence_end(buffer_sequence_));   //��ʼ��buffer
  }

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move constructor.���ƶ����졿
  buffer_registration(buffer_registration&& other) ASIO_NOEXCEPT
    : buffer_sequence_(std::move(other.buffer_sequence_)),
      buffers_(std::move(other.buffers_))
  {
#if defined(ASIO_HAS_IO_URING)
    service_ = other.service_;
    other.service_ = 0;
#endif // defined(ASIO_HAS_IO_URING)
  }
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Unregisters the buffers.������������
  ~buffer_registration()
  {
#if defined(ASIO_HAS_IO_URING)
    if (service_)
      service_->unregister_buffers();
#endif // defined(ASIO_HAS_IO_URING)
  }
  
#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move assignment. ���ƶ���ֵ��
  buffer_registration& operator=(
      buffer_registration&& other) ASIO_NOEXCEPT
  {
    if (this != &other)
    {
      buffer_sequence_ = std::move(other.buffer_sequence_);
      buffers_ = std::move(other.buffers_);
#if defined(ASIO_HAS_IO_URING)
      if (service_)
        service_->unregister_buffers();
      service_ = other.service_;
      other.service_ = 0;
#endif // defined(ASIO_HAS_IO_URING)
    }
    return *this;
  }
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Get the number of registered buffers. ������װregistered_buffer��vector�Ĵ�С��
  std::size_t size() const ASIO_NOEXCEPT
  {
    return buffers_.size();
  }

  /// Get the begin iterator for the sequence of registered buffers. ������vector���׵�������
  const_iterator begin() const ASIO_NOEXCEPT
  {
    return buffers_.begin();
  }

  /// Get the begin iterator for the sequence of registered buffers.������vector��cbegin��
  const_iterator cbegin() const ASIO_NOEXCEPT
  {
    return buffers_.cbegin();
  }

  /// Get the end iterator for the sequence of registered buffers.������vector��end()��
  const_iterator end() const ASIO_NOEXCEPT
  {
    return buffers_.end();
  }

  /// Get the end iterator for the sequence of registered buffers.������vector��cend()��
  const_iterator cend() const ASIO_NOEXCEPT
  {
    return buffers_.cend();
  }

  /// Get the buffer at the specified index.������vector��ĳһ����ŵĻ�������
  const mutable_registered_buffer& operator[](std::size_t i) ASIO_NOEXCEPT
  {
    return buffers_[i];
  }

  /// Get the buffer at the specified index.��������һ����
  const mutable_registered_buffer& at(std::size_t i) ASIO_NOEXCEPT
  {
    return buffers_.at(i);
  }

private:
  // Disallow copying and assignment. �����������͸�ֵ������
  buffer_registration(const buffer_registration&) ASIO_DELETED;
  buffer_registration& operator=(const buffer_registration&) ASIO_DELETED;

  // Helper function to get an executor's context.���õ�ִ�������ĵİ���������
  template <typename T>
  static execution_context& get_context(const T& t,
      typename enable_if<execution::is_executor<T>::value>::type* = 0)  //����������֪��enable_if�ڶ�������Ĭ����void���������������ػ����ӵڶ�������Ĭ��Ϊvoid*��
  {
    return asio::query(t, execution::context);  //��ѯ�Ի��ִ��������
  }

  // Helper function to get an executor's context.
  template <typename T>
  static execution_context& get_context(const T& t,
      typename enable_if<!execution::is_executor<T>::value>::type* = 0) //����������֪��enable_if�ڶ�������Ĭ����void���������������ػ����ӵڶ�������Ĭ��Ϊvoid*��
  {
    return t.context();
  }

  // Helper function to initialise the container of buffers. ��������������ʼ�������������
  template <typename Iterator>
  void init_buffers(execution_context& ctx, Iterator begin, Iterator end)
  {
    std::size_t n = std::distance(begin, end);  //�������ľ���
    buffers_.resize(n); //��buffer_��vector�Ĵ�С����Ϊ��ô��

#if defined(ASIO_HAS_IO_URING)
    service_ = &use_service<detail::io_uring_service>(ctx);
    std::vector<iovec,
      ASIO_REBIND_ALLOC(allocator_type, iovec)> iovecs(n,
          ASIO_REBIND_ALLOC(allocator_type, iovec)(
            buffers_.get_allocator()));
#endif // defined(ASIO_HAS_IO_URING)

    Iterator iter = begin;  //ʹ������������Ŀ�ͷ
    for (int index = 0; iter != end; ++index, ++iter)
    {
      mutable_buffer b(*iter);  //�½�һ���ɱ仺��
      std::size_t i = static_cast<std::size_t>(index);  //֪���İɣ�index��0-n =>��ת����u64
      buffers_[i] = this->make_buffer(b, &ctx, index);  //���ݴ����Ŀɱ仺�壬������[�����������]��index����[make_buffer()]�������ĸ�����ӿڵİ��������������ȥע��һ���������ġ�

#if defined(ASIO_HAS_IO_URING)
      iovecs[i].iov_base = buffers_[i].data();
      iovecs[i].iov_len = buffers_[i].size();
#endif // defined(ASIO_HAS_IO_URING)
    }

#if defined(ASIO_HAS_IO_URING)
    if (n > 0)
    {
      service_->register_buffers(&iovecs[0],
          static_cast<unsigned>(iovecs.size()));
    }
#endif // defined(ASIO_HAS_IO_URING)
  }

  MutableBufferSequence buffer_sequence_;       //�ɱ仺��������
  std::vector<mutable_registered_buffer,    
    ASIO_REBIND_ALLOC(allocator_type,
      mutable_registered_buffer)> buffers_;     //��vector������װ����ע�����buffer��
#if defined(ASIO_HAS_IO_URING)
  detail::io_uring_service* service_;
#endif // defined(ASIO_HAS_IO_URING)
};


///======================================================================================================================================================

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
/// Register buffers with an execution context. ��ʹ��ִ��������ע�Ỻ������
template <typename Executor, typename MutableBufferSequence>
ASIO_NODISCARD inline
buffer_registration<MutableBufferSequence>
register_buffers(const Executor& ex,    //ִ����
    const MutableBufferSequence& buffer_sequence,   //�ڴ�����
    typename constraint<
      is_executor<Executor>::value || execution::is_executor<Executor>::value
    >::type = 0)    //�ػ�������ֻ��Executor��ִ����ʱ���ػ�Ϊint = 0 ������ģ��С���ɰɡ�
{
  return buffer_registration<MutableBufferSequence>(ex, buffer_sequence);
}

/// Register buffers with an execution context. ��ʹ��ִ��������ע��(��������)��
template <typename Executor, typename MutableBufferSequence, typename Allocator>
ASIO_NODISCARD inline
buffer_registration<MutableBufferSequence, Allocator>
register_buffers(const Executor& ex,    //ִ����
    const MutableBufferSequence& buffer_sequence, const Allocator& alloc,   //�ڴ����У� ������
    typename constraint<
      is_executor<Executor>::value || execution::is_executor<Executor>::value
    >::type = 0)    //�ػ�����
{
  return buffer_registration<MutableBufferSequence, Allocator>(
      ex, buffer_sequence, alloc);
}

/// Register buffers with an execution context. ��ʹ��ִ��������ע�᡿
template <typename ExecutionContext, typename MutableBufferSequence>
ASIO_NODISCARD inline
buffer_registration<MutableBufferSequence>
register_buffers(ExecutionContext& ctx, //ִ��������
    const MutableBufferSequence& buffer_sequence,
    typename constraint<
      is_convertible<ExecutionContext&, execution_context&>::value
    >::type = 0)    //�ػ�����
{
  return buffer_registration<MutableBufferSequence>(ctx, buffer_sequence);
}

/// Register buffers with an execution context. ��ʹ��ִ��������ע�ᣬ����������
template <typename ExecutionContext,
    typename MutableBufferSequence, typename Allocator>
ASIO_NODISCARD inline
buffer_registration<MutableBufferSequence, Allocator>
register_buffers(ExecutionContext& ctx,
    const MutableBufferSequence& buffer_sequence, const Allocator& alloc,
    typename constraint<
      is_convertible<ExecutionContext&, execution_context&>::value
    >::type = 0)
{
  return buffer_registration<MutableBufferSequence, Allocator>(
      ctx, buffer_sequence, alloc);
}
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_BUFFER_REGISTRATION_HPP
