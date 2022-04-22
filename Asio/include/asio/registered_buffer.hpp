//
// TAKO:注册缓冲区
// registered_buffer.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_REGISTERED_BUFFER_HPP
#define ASIO_REGISTERED_BUFFER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/buffer.hpp"  //缓冲设计

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

class buffer_registration_base;

} // namespace detail

///======================================================================================================================================================================


class const_registered_buffer;
///======================================================================================================================================================================

/// Type used to identify a registered buffer.【注册缓冲区的id类[scope+index]】
class registered_buffer_id
{
public:
  /// The native buffer identifier type.   【本地缓冲区的序号类型用int】
  typedef int native_handle_type;

  /// Default constructor creates an invalid registered buffer identifier.
  registered_buffer_id() ASIO_NOEXCEPT
    : scope_(0),
      index_(-1)    //默认的非法id用-1，常规操作了
  {
  }

  /// Get the native buffer identifier type. 【获取本地缓冲区标志符】
  native_handle_type native_handle() const ASIO_NOEXCEPT
  {
    return index_;
  }

  /// Compare two IDs for equality. 【比较两个id相同】
  friend bool operator==(const registered_buffer_id& lhs,
      const registered_buffer_id& rhs) ASIO_NOEXCEPT
  {
    return lhs.scope_ == rhs.scope_ && lhs.index_ == rhs.index_;    //【scope要是变动了说明原来的已经删除了】【index就是装位置的序号】
  }

  /// Compare two IDs for equality. 【比较两个id是否不同】
  friend bool operator!=(const registered_buffer_id& lhs,
      const registered_buffer_id& rhs) ASIO_NOEXCEPT
  {
    return lhs.scope_ != rhs.scope_ || lhs.index_ != rhs.index_;
  }

private:
  friend class detail::buffer_registration_base;    // 声明友元类，看看缓冲区注册实现

  // Hidden constructor used by buffer registration.
  registered_buffer_id(const void* scope, int index) ASIO_NOEXCEPT
    : scope_(scope),
      index_(index)
  {
  }

  const void* scope_;   //scope实际上一般传入的是contex，上下文，这样写我们就把它理解成作用域吧
  int index_;   //s32的序号
};


///======================================================================================================================================================================

/// Holds a registered buffer over modifiable data.
/** 
* 【可变注册缓冲区】
 * Satisfies the @c MutableBufferSequence type requirements.
 */
class mutable_registered_buffer
{
public:
#if !defined(ASIO_HAS_DECLTYPE) \
  && !defined(GENERATING_DOCUMENTATION)
  typedef mutable_buffer value_type;
#endif // !defined(ASIO_HAS_DECLTYPE)
       //   && !defined(GENERATING_DOCUMENTATION)

  /// Default constructor creates an invalid registered buffer.
  mutable_registered_buffer() ASIO_NOEXCEPT
    : buffer_(),
      id_()
  {
  }

  /// Get the underlying mutable buffer.【获得下划线可变缓冲区】
  const mutable_buffer& buffer() const ASIO_NOEXCEPT
  {
    return buffer_;
  }

  /// Get a pointer to the beginning of the memory range.【获得下属缓冲区的首地址】
  /**
   * @returns <tt>buffer().data()</tt>.
   */
  void* data() const ASIO_NOEXCEPT
  {
    return buffer_.data();
  }

  /// Get the size of the memory range. 【获得下属缓冲区的大小】
  /**
   * @returns <tt>buffer().size()</tt>.
   */
  std::size_t size() const ASIO_NOEXCEPT
  {
    return buffer_.size();
  }

  /// Get the registered buffer identifier. 【获得这个注册缓冲区的id】
  const registered_buffer_id& id() const ASIO_NOEXCEPT
  {
    return id_;
  }

  /// Move the start of the buffer by the specified number of bytes. 【调用下面缓冲区的+=方法，将缓冲区的首地址向右平移n个字节】
  mutable_registered_buffer& operator+=(std::size_t n) ASIO_NOEXCEPT
  {
    buffer_ += n;
    return *this;
  }

private:
  friend class detail::buffer_registration_base;    //【声明友元类】

  // Hidden constructor used by buffer registration and operators. 【有参数的构建函数只能由友元类访问，隐藏起来了[即默认构造出来的buffer一定是非法的invalid]】
  mutable_registered_buffer(const mutable_buffer& b,
      const registered_buffer_id& i) ASIO_NOEXCEPT
    : buffer_(b),
      id_(i)
  {
  }

#if !defined(GENERATING_DOCUMENTATION)
  friend mutable_registered_buffer buffer(
      const mutable_registered_buffer& b, std::size_t n) ASIO_NOEXCEPT;
#endif // !defined(GENERATING_DOCUMENTATION)

  mutable_buffer buffer_;   // 可变缓冲区
  registered_buffer_id id_; //id_就是上面说的带[scope_和index_]的registered_buffer_id
};

/// =============================================================================================================================================================

/// Holds a registered buffer over non-modifiable data. 【固定注册缓冲区】
/** 
 * Satisfies the @c ConstBufferSequence type requirements.
 */
class const_registered_buffer
{
public:
#if !defined(ASIO_HAS_DECLTYPE) \
  && !defined(GENERATING_DOCUMENTATION)
  typedef const_buffer value_type;
#endif // !defined(ASIO_HAS_DECLTYPE)
       //   && !defined(GENERATING_DOCUMENTATION)

  /// Default constructor creates an invalid registered buffer.
  const_registered_buffer() ASIO_NOEXCEPT
    : buffer_(),
      id_()    
  {
  }

  /// Construct a non-modifiable buffer from a modifiable one.
  const_registered_buffer(
      const mutable_registered_buffer& b) ASIO_NOEXCEPT
    : buffer_(b.buffer()),
      id_(b.id())
  {
  }

  /// Get the underlying constant buffer.
  const const_buffer& buffer() const ASIO_NOEXCEPT
  {
    return buffer_;
  }

  /// Get a pointer to the beginning of the memory range. 【相比之下，这个data()返回的是const void* 是一个常量指针，是不能对里面的数据进行修改的】
  /**
   * @returns <tt>buffer().data()</tt>.
   */
  const void* data() const ASIO_NOEXCEPT
  {
    return buffer_.data();
  }

  /// Get the size of the memory range.
  /**
   * @returns <tt>buffer().size()</tt>.
   */
  std::size_t size() const ASIO_NOEXCEPT
  {
    return buffer_.size();
  }

  /// Get the registered buffer identifier. 【获取这个的id[scope和index]】
  const registered_buffer_id& id() const ASIO_NOEXCEPT
  {
    return id_;
  }

  /// Move the start of the buffer by the specified number of bytes.
  const_registered_buffer& operator+=(std::size_t n) ASIO_NOEXCEPT
  {
    buffer_ += n;
    return *this;
  }

private:
  // Hidden constructor used by operators.
  const_registered_buffer(const const_buffer& b,
      const registered_buffer_id& i) ASIO_NOEXCEPT
    : buffer_(b),
      id_(i)
  {
  }

#if !defined(GENERATING_DOCUMENTATION)
  friend const_registered_buffer buffer(
      const const_registered_buffer& b, std::size_t n) ASIO_NOEXCEPT;
#endif // !defined(GENERATING_DOCUMENTATION)

  const_buffer buffer_;         //固定缓冲区类的实例
  registered_buffer_id id_;     // 注册缓冲区的id类实例[实现就是scope+index]
};



// =======================================================================================================================================================
/** @addtogroup buffer_sequence_begin */

/// Get an iterator to the first element in a buffer sequence. 【传入注册缓冲区，返回其下属可变缓冲区的开头】
inline const mutable_buffer* buffer_sequence_begin(
    const mutable_registered_buffer& b) ASIO_NOEXCEPT
{
  return &b.buffer();
}

/// Get an iterator to the first element in a buffer sequence. 【和上面相比，是常注册缓冲区】
inline const const_buffer* buffer_sequence_begin(
    const const_registered_buffer& b) ASIO_NOEXCEPT
{
  return &b.buffer();
}

/** @} */
/** @addtogroup buffer_sequence_end */

/// Get an iterator to one past the end element in a buffer sequence. 【缓冲区结尾】
inline const mutable_buffer* buffer_sequence_end(
    const mutable_registered_buffer& b) ASIO_NOEXCEPT
{
  return &b.buffer() + 1;
}

/// Get an iterator to one past the end element in a buffer sequence.【缓冲区结尾】
inline const const_buffer* buffer_sequence_end(
    const const_registered_buffer& b) ASIO_NOEXCEPT
{
  return &b.buffer() + 1;
}

/** @} */
/** @addtogroup buffer */

/// Obtain a buffer representing the entire registered buffer. 【输入可变缓冲区b，又输出b? 这不多此一举】
inline mutable_registered_buffer buffer(
    const mutable_registered_buffer& b) ASIO_NOEXCEPT
{
  return b;
}

/// Obtain a buffer representing the entire registered buffer.
inline const_registered_buffer buffer(
    const const_registered_buffer& b) ASIO_NOEXCEPT
{
  return b;
}

/// Obtain a buffer representing part of a registered buffer. 【获取】
inline mutable_registered_buffer buffer(
    const mutable_registered_buffer& b, std::size_t n) ASIO_NOEXCEPT
{
  return mutable_registered_buffer(buffer(b.buffer_, n), b.id_);    //先通过这个注册缓冲区构建一个普通buffer，再借此构建一个新的注册缓冲区
}

/// Obtain a buffer representing part of a registered buffer.
inline const_registered_buffer buffer(
    const const_registered_buffer& b, std::size_t n) ASIO_NOEXCEPT
{
  return const_registered_buffer(buffer(b.buffer_, n), b.id_); //先通过这个注册缓冲区构建一个普通buffer，再借此构建一个新的注册缓冲区
}

/** @} */

/// Create a new modifiable registered buffer that is offset from the start of
/// another.
/**
 * @relates mutable_registered_buffer
 */
inline mutable_registered_buffer operator+(
    const mutable_registered_buffer& b, std::size_t n) ASIO_NOEXCEPT
{
  mutable_registered_buffer tmp(b);
  tmp += n;     //新的偏移n
  return tmp;
}

/// Create a new modifiable buffer that is offset from the start of another.
/**
* 【上面的拓展】
 * @relates mutable_registered_buffer
 */
inline mutable_registered_buffer operator+(std::size_t n,
    const mutable_registered_buffer& b) ASIO_NOEXCEPT
{
  return b + n;
}

/// Create a new non-modifiable registered buffer that is offset from the start
/// of another.
/**
 * @relates const_registered_buffer
 */
inline const_registered_buffer operator+(const const_registered_buffer& b,
    std::size_t n) ASIO_NOEXCEPT
{
  const_registered_buffer tmp(b);
  tmp += n;
  return tmp;
}

/// Create a new non-modifiable buffer that is offset from the start of another.
/**
 * @relates const_registered_buffer
 */
inline const_registered_buffer operator+(std::size_t n,
    const const_registered_buffer& b) ASIO_NOEXCEPT
{
  return b + n;
}

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_REGISTERED_BUFFER_HPP
