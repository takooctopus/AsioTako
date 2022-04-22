//
// TAKO:buffer的注册过程
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

class buffer_registration_base  //buffer注册过程类，里面就一个[static]方法，创建缓冲区【只能通过这个方法进行缓冲区的有参构建】
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
/// 【使用执行上下文做自动的缓冲区注册和解除注册】
/**
 * For portability, applications should assume that only one registration is
 * permitted per execution context.
 * 【为了移植性，应用应该保证每一个执行上下文只允许一个注册过程】
 */
template <typename MutableBufferSequence,
    typename Allocator = std::allocator<void> > //<>传入的是可变缓冲区序列类，和内存分配器
class buffer_registration
  : detail::buffer_registration_base    //继承上面的友元类接口【只有通过这个才能进行注册缓冲区的有参构建(即分配一个合法的id)】
{
public:
  /// The allocator type used for allocating storage for the buffers container. 【内存分配器类allocator】
  typedef Allocator allocator_type;

#if defined(GENERATING_DOCUMENTATION)
  /// The type of an iterator over the registered buffers.
  typedef unspecified iterator;

  /// The type of a const iterator over the registered buffers.
  typedef unspecified const_iterator;
#else // defined(GENERATING_DOCUMENTATION)
  typedef std::vector<mutable_registered_buffer>::const_iterator iterator;  //vector的迭代器
  typedef std::vector<mutable_registered_buffer>::const_iterator const_iterator;    //vector的常迭代器
#endif // defined(GENERATING_DOCUMENTATION)

  /// Register buffers with an executor's execution context.
  template <typename Executor>
  buffer_registration(const Executor& ex,               //【传入执行】
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
    init_buffers(buffer_registration::get_context(ex),      //这里获取了执行上下文
        asio::buffer_sequence_begin(buffer_sequence_),
        asio::buffer_sequence_end(buffer_sequence_));
  }

  /// Register buffers with an execution context.
  template <typename ExecutionContext>
  buffer_registration(ExecutionContext& ctx,    //【1.传入一个执行上下文】
      const MutableBufferSequence& buffer_sequence, //【2.传入一个缓冲区序列】
      const allocator_type& alloc = allocator_type(),   //【3.分配内存的方法】
      typename constraint<
        is_convertible<ExecutionContext&, execution_context&>::value
      >::type = 0)  //【4.约束，实际上是一个继承于enable_if的结构体，要是能转换出来，默认这个参数为int类型且为0】
    : buffer_sequence_(buffer_sequence),
      buffers_(
          ASIO_REBIND_ALLOC(allocator_type,
            mutable_registered_buffer)(alloc))
  {
    init_buffers(ctx,
        asio::buffer_sequence_begin(buffer_sequence_),
        asio::buffer_sequence_end(buffer_sequence_));   //初始化buffer
  }

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move constructor.【移动构造】
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

  /// Unregisters the buffers.【析构函数】
  ~buffer_registration()
  {
#if defined(ASIO_HAS_IO_URING)
    if (service_)
      service_->unregister_buffers();
#endif // defined(ASIO_HAS_IO_URING)
  }
  
#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move assignment. 【移动赋值】
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

  /// Get the number of registered buffers. 【返回装registered_buffer的vector的大小】
  std::size_t size() const ASIO_NOEXCEPT
  {
    return buffers_.size();
  }

  /// Get the begin iterator for the sequence of registered buffers. 【返回vector的首迭代器】
  const_iterator begin() const ASIO_NOEXCEPT
  {
    return buffers_.begin();
  }

  /// Get the begin iterator for the sequence of registered buffers.【返回vector的cbegin】
  const_iterator cbegin() const ASIO_NOEXCEPT
  {
    return buffers_.cbegin();
  }

  /// Get the end iterator for the sequence of registered buffers.【返回vector的end()】
  const_iterator end() const ASIO_NOEXCEPT
  {
    return buffers_.end();
  }

  /// Get the end iterator for the sequence of registered buffers.【返回vector的cend()】
  const_iterator cend() const ASIO_NOEXCEPT
  {
    return buffers_.cend();
  }

  /// Get the buffer at the specified index.【返回vector中某一个序号的缓冲区】
  const mutable_registered_buffer& operator[](std::size_t i) ASIO_NOEXCEPT
  {
    return buffers_[i];
  }

  /// Get the buffer at the specified index.【和上面一样】
  const mutable_registered_buffer& at(std::size_t i) ASIO_NOEXCEPT
  {
    return buffers_.at(i);
  }

private:
  // Disallow copying and assignment. 【禁掉拷贝和赋值函数】
  buffer_registration(const buffer_registration&) ASIO_DELETED;
  buffer_registration& operator=(const buffer_registration&) ASIO_DELETED;

  // Helper function to get an executor's context.【拿到执行上下文的帮助函数】
  template <typename T>
  static execution_context& get_context(const T& t,
      typename enable_if<execution::is_executor<T>::value>::type* = 0)  //【这里我们知道enable_if第二个参数默认是void，我们这样进行特化，加第二个参数默认为void*】
  {
    return asio::query(t, execution::context);  //查询以获得执行上下文
  }

  // Helper function to get an executor's context.
  template <typename T>
  static execution_context& get_context(const T& t,
      typename enable_if<!execution::is_executor<T>::value>::type* = 0) //【这里我们知道enable_if第二个参数默认是void，我们这样进行特化，加第二个参数默认为void*】
  {
    return t.context();
  }

  // Helper function to initialise the container of buffers. 【帮助函数，初始化缓冲的容器】
  template <typename Iterator>
  void init_buffers(execution_context& ctx, Iterator begin, Iterator end)
  {
    std::size_t n = std::distance(begin, end);  //迭代器的距离
    buffers_.resize(n); //将buffer_即vector的大小重设为这么大

#if defined(ASIO_HAS_IO_URING)
    service_ = &use_service<detail::io_uring_service>(ctx);
    std::vector<iovec,
      ASIO_REBIND_ALLOC(allocator_type, iovec)> iovecs(n,
          ASIO_REBIND_ALLOC(allocator_type, iovec)(
            buffers_.get_allocator()));
#endif // defined(ASIO_HAS_IO_URING)

    Iterator iter = begin;  //使用这个迭代器的开头
    for (int index = 0; iter != end; ++index, ++iter)
    {
      mutable_buffer b(*iter);  //新建一个可变缓冲
      std::size_t i = static_cast<std::size_t>(index);  //知道的吧，index从0-n =>再转换成u64
      buffers_[i] = this->make_buffer(b, &ctx, index);  //根据创建的可变缓冲，上下文[这个是作用域]和index调用[make_buffer()]【就是哪个基类接口的帮助函数，用这个去注册一个缓冲区的】

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

  MutableBufferSequence buffer_sequence_;       //可变缓冲区序列
  std::vector<mutable_registered_buffer,    
    ASIO_REBIND_ALLOC(allocator_type,
      mutable_registered_buffer)> buffers_;     //【vector，里面装的是注册过的buffer】
#if defined(ASIO_HAS_IO_URING)
  detail::io_uring_service* service_;
#endif // defined(ASIO_HAS_IO_URING)
};


///======================================================================================================================================================

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
/// Register buffers with an execution context. 【使用执行上下文注册缓冲区】
template <typename Executor, typename MutableBufferSequence>
ASIO_NODISCARD inline
buffer_registration<MutableBufferSequence>
register_buffers(const Executor& ex,    //执行器
    const MutableBufferSequence& buffer_sequence,   //内存序列
    typename constraint<
      is_executor<Executor>::value || execution::is_executor<Executor>::value
    >::type = 0)    //特化参数，只有Executor是执行器时才特化为int = 0 【算是模板小技巧吧】
{
  return buffer_registration<MutableBufferSequence>(ex, buffer_sequence);
}

/// Register buffers with an execution context. 【使用执行上下文注册(带分配器)】
template <typename Executor, typename MutableBufferSequence, typename Allocator>
ASIO_NODISCARD inline
buffer_registration<MutableBufferSequence, Allocator>
register_buffers(const Executor& ex,    //执行器
    const MutableBufferSequence& buffer_sequence, const Allocator& alloc,   //内存序列， 分配器
    typename constraint<
      is_executor<Executor>::value || execution::is_executor<Executor>::value
    >::type = 0)    //特化参数
{
  return buffer_registration<MutableBufferSequence, Allocator>(
      ex, buffer_sequence, alloc);
}

/// Register buffers with an execution context. 【使用执行上下文注册】
template <typename ExecutionContext, typename MutableBufferSequence>
ASIO_NODISCARD inline
buffer_registration<MutableBufferSequence>
register_buffers(ExecutionContext& ctx, //执行上下文
    const MutableBufferSequence& buffer_sequence,
    typename constraint<
      is_convertible<ExecutionContext&, execution_context&>::value
    >::type = 0)    //特化参数
{
  return buffer_registration<MutableBufferSequence>(ctx, buffer_sequence);
}

/// Register buffers with an execution context. 【使用执行上下文注册，带分配器】
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
