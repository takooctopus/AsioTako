//
// TAKO: 串行缓冲区适配器
// detail/buffer_sequence_adapter.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_BUFFER_SEQUENCE_ADAPTER_HPP
#define ASIO_DETAIL_BUFFER_SEQUENCE_ADAPTER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/buffer.hpp"              //缓冲区
#include "asio/detail/array_fwd.hpp"    //数组前置声明
#include "asio/detail/socket_types.hpp" //socket相关常量
#include "asio/registered_buffer.hpp"   //注册buffer

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

/// ====================================================================================================================================================================
/*
* 缓冲序列适配器
*/
class buffer_sequence_adapter_base 
{
#if defined(ASIO_WINDOWS_RUNTIME)
public:
  // The maximum number of buffers to support in a single operation.
  enum { max_buffers = 1 };

protected:
  typedef Windows::Storage::Streams::IBuffer^ native_buffer_type;

  ASIO_DECL static void init_native_buffer(
      native_buffer_type& buf,
      const asio::mutable_buffer& buffer);

  ASIO_DECL static void init_native_buffer(
      native_buffer_type& buf,
      const asio::const_buffer& buffer);
#elif defined(ASIO_WINDOWS) || defined(__CYGWIN__)  //windows下的缓冲区类型
public:
  // The maximum number of buffers to support in a single operation.【iovec结构数组中元素的数目存在某个限制（IOV_MAX】
  enum { max_buffers = 64 < max_iov_len ? 64 : max_iov_len };

protected:
  typedef WSABUF native_buffer_type;    //接收WSASocket数据的缓冲

  static void init_native_buffer(WSABUF& buf,
      const asio::mutable_buffer& buffer)   //将buffer中的数据放到本地缓冲结构中
  {
    buf.buf = static_cast<char*>(buffer.data());    //【将可变buffer中的数据放到本地WSASocket缓冲数据中】
    buf.len = static_cast<ULONG>(buffer.size());    //【同样的还有缓冲区长度】
  }

  static void init_native_buffer(WSABUF& buf,
      const asio::const_buffer& buffer) //将固定缓冲区数据放到本地缓冲结构中
  {
    buf.buf = const_cast<char*>(static_cast<const char*>(buffer.data()));
    buf.len = static_cast<ULONG>(buffer.size());
  }
#else // defined(ASIO_WINDOWS) || defined(__CYGWIN__)
public:
  // The maximum number of buffers to support in a single operation.
  enum { max_buffers = 64 < max_iov_len ? 64 : max_iov_len };

protected:
  typedef iovec native_buffer_type; //【linux下的缓冲结构是iovector】

  static void init_iov_base(void*& base, void* addr)
  {
    base = addr;    //base是一个地址引用，这样可以修改base
  }

  template <typename T>
  static void init_iov_base(T& base, void* addr)
  {
    base = static_cast<T>(addr);    //T作为类型引用，将addr区块的内存转化成T类型的数据对象
  }

  static void init_native_buffer(iovec& iov,
      const asio::mutable_buffer& buffer)   //【这个就是初始化iovector这个linux下缓冲区的数据结构的】
  {
    init_iov_base(iov.iov_base, buffer.data()); //将iov_base赋值为缓冲区的指针
    iov.iov_len = buffer.size();    //长度
  }

  static void init_native_buffer(iovec& iov,
      const asio::const_buffer& buffer)
  {
    init_iov_base(iov.iov_base, const_cast<void*>(buffer.data()));
    iov.iov_len = buffer.size();
  }
#endif // defined(ASIO_WINDOWS) || defined(__CYGWIN__)
};

/// =======================================================================================================================================================
// Helper class to translate buffers into the native buffer representation. 【这个类继承了上面的接口】
template <typename Buffer, typename Buffers>
class buffer_sequence_adapter
  : buffer_sequence_adapter_base
{
public:
  enum { is_single_buffer = false };    //是不是单缓冲
  enum { is_registered_buffer = false };    //是不是已经注册了

  explicit buffer_sequence_adapter(const Buffers& buffer_sequence)  //通过缓冲序列，将缓冲区序列初始化到adapter里
    : count_(0), total_buffer_size_(0)
  {
    buffer_sequence_adapter::init(
        asio::buffer_sequence_begin(buffer_sequence),
        asio::buffer_sequence_end(buffer_sequence));
  }

  native_buffer_type* buffers() //返回这个包含本地缓冲区容器数组的首地址
  {
    return buffers_;
  }

  std::size_t count() const //返回本地缓冲结构的容器数组里面的元素个数
  {
    return count_;
  }

  std::size_t total_size() const    //返回容器数组里面所有缓冲的字节长度的总和
  {
    return total_buffer_size_;
  }

  registered_buffer_id registered_id() const    //返回缓冲区的注册id[因为是本地缓冲结构，那么id全是非法id[用的默认构造函数]]
  {
    return registered_buffer_id();
  }

  bool all_empty() const    //是否为空的
  {
    return total_buffer_size_ == 0; //判断一下是不是空的
  }

  static bool all_empty(const Buffers& buffer_sequence) //传入一个缓冲区序列，判断一下是不是全部空的
  {
    return buffer_sequence_adapter::all_empty(
        asio::buffer_sequence_begin(buffer_sequence),
        asio::buffer_sequence_end(buffer_sequence));
  }

  static void validate(const Buffers& buffer_sequence)  //传入一个缓冲区序列，进行一下数据验证
  {
    buffer_sequence_adapter::validate(
        asio::buffer_sequence_begin(buffer_sequence),
        asio::buffer_sequence_end(buffer_sequence));
  }

  static Buffer first(const Buffers& buffer_sequence)   //传入一个缓冲区序列，返回其首元素
  {
    return buffer_sequence_adapter::first(
        asio::buffer_sequence_begin(buffer_sequence),
        asio::buffer_sequence_end(buffer_sequence));
  }

  enum { linearisation_storage_size = 8192 };   //【线性存储大小8192字节[2页]】

  static Buffer linearise(const Buffers& buffer_sequence,
      const asio::mutable_buffer& storage)  //线性化
  {
    return buffer_sequence_adapter::linearise(  
        asio::buffer_sequence_begin(buffer_sequence),
        asio::buffer_sequence_end(buffer_sequence), storage);  
  }

private:
  template <typename Iterator>
  void init(Iterator begin, Iterator end)   //私有的初始化函数【上面的共有函数调用的这边】【传入两个iterator】
  {
    Iterator iter = begin;
    for (; iter != end && count_ < max_buffers; ++iter, ++count_)   //考虑到iovector有最大的数量限制
    {
      Buffer buffer(*iter);                             //先根据传入的iter初始化可变\固定缓冲
      init_native_buffer(buffers_[count_], buffer);     //然后本地缓冲的容器数组的对应idx上用出来的buffer对其进行初始化
      total_buffer_size_ += buffer.size();              //计算所有的缓冲字节长度相加
    }
  }

  template <typename Iterator>
  static bool all_empty(Iterator begin, Iterator end)   //私有的判断是否为空的函数【是不是全部为空】【传入两个iterator】
  {
    Iterator iter = begin;
    std::size_t i = 0;
    for (; iter != end && i < max_buffers; ++iter, ++i)
      if (Buffer(*iter).size() > 0) //通过迭代器初始化可变\固定缓冲，然后依次比较是不是size为0
        return false;
    return true;
  }

  template <typename Iterator>
  static void validate(Iterator begin, Iterator end)    //私有化的验证函数【传入两个迭代器】
  {
    Iterator iter = begin;
    for (; iter != end; ++iter)
    {
      Buffer buffer(*iter);
      buffer.data();    //怎么啥都没做啊？
    }
  }

  template <typename Iterator>
  static Buffer first(Iterator begin, Iterator end)
  {
    Iterator iter = begin;
    for (; iter != end; ++iter)
    {
      Buffer buffer(*iter);
      if (buffer.size() != 0)
        return buffer;  //只要返回第一个可变缓冲就行
    }
    return Buffer();    //空的就返回空的
  }

  template <typename Iterator>
  static Buffer linearise(Iterator begin, Iterator end,
      const asio::mutable_buffer& storage)  //线性化【第三个参数是一个可变缓冲引用】
  {
    asio::mutable_buffer unused_storage = storage;  
    Iterator iter = begin;
    while (iter != end && unused_storage.size() != 0)
    {
      Buffer buffer(*iter);
      ++iter;
      if (buffer.size() == 0)
        continue;
      if (unused_storage.size() == storage.size())
      {
        if (iter == end)
          return buffer;
        if (buffer.size() >= unused_storage.size()) //要是大于剩余空间了，说明放不下了
          return buffer;
      }
      unused_storage += asio::buffer_copy(unused_storage, buffer);
    }
    return Buffer(storage.data(), storage.size() - unused_storage.size());
  }

  native_buffer_type buffers_[max_buffers]; //这个adapter里面是一个本地缓冲结构的数组[win=>WSABUF, linux=>iovec]
  std::size_t count_;       //这个数组里面的元素个数
  std::size_t total_buffer_size_;   //总缓冲区的字节长度
};

///===================================================================================================================================================================== 

template <typename Buffer>
class buffer_sequence_adapter<Buffer, asio::mutable_buffer>     //上面的在特化类，特化到可变缓冲了【但下划线buffer_只有一个，并非上面的数组】
  : buffer_sequence_adapter_base
{
public:
  enum { is_single_buffer = true };
  enum { is_registered_buffer = false };

  explicit buffer_sequence_adapter(
      const asio::mutable_buffer& buffer_sequence)
  {
    init_native_buffer(buffer_, Buffer(buffer_sequence));
    total_buffer_size_ = buffer_sequence.size();
  }

  native_buffer_type* buffers()
  {
    return &buffer_;
  }

  std::size_t count() const
  {
    return 1;
  }

  std::size_t total_size() const
  {
    return total_buffer_size_;
  }

  registered_buffer_id registered_id() const
  {
    return registered_buffer_id();
  }

  bool all_empty() const
  {
    return total_buffer_size_ == 0;
  }

  static bool all_empty(const asio::mutable_buffer& buffer_sequence)
  {
    return buffer_sequence.size() == 0;
  }

  static void validate(const asio::mutable_buffer& buffer_sequence)
  {
    buffer_sequence.data();
  }

  static Buffer first(const asio::mutable_buffer& buffer_sequence)
  {
    return Buffer(buffer_sequence);
  }

  enum { linearisation_storage_size = 1 };  //线性化只有一个

  static Buffer linearise(const asio::mutable_buffer& buffer_sequence,
      const Buffer&)
  {
    return Buffer(buffer_sequence); //直接返回一个Buffer[可变/固定]
  }

private:
  native_buffer_type buffer_;   // 只有一个本地缓冲区数据结构
  std::size_t total_buffer_size_;   //总共的大小
};

///===================================================================================================================================================================== 


template <typename Buffer>
class buffer_sequence_adapter<Buffer, asio::const_buffer>   //特化模板固定缓冲区
  : buffer_sequence_adapter_base
{
public:
  enum { is_single_buffer = true }; //单缓冲
  enum { is_registered_buffer = false };    //非注册

  explicit buffer_sequence_adapter(
      const asio::const_buffer& buffer_sequence)
  {
    init_native_buffer(buffer_, Buffer(buffer_sequence));
    total_buffer_size_ = buffer_sequence.size();
  }

  native_buffer_type* buffers()
  {
    return &buffer_;
  }

  std::size_t count() const
  {
    return 1;
  }

  std::size_t total_size() const
  {
    return total_buffer_size_;
  }

  registered_buffer_id registered_id() const
  {
    return registered_buffer_id();
  }

  bool all_empty() const
  {
    return total_buffer_size_ == 0;
  }

  static bool all_empty(const asio::const_buffer& buffer_sequence)
  {
    return buffer_sequence.size() == 0;
  }

  static void validate(const asio::const_buffer& buffer_sequence)
  {
    buffer_sequence.data();
  }

  static Buffer first(const asio::const_buffer& buffer_sequence)
  {
    return Buffer(buffer_sequence);
  }

  enum { linearisation_storage_size = 1 };

  static Buffer linearise(const asio::const_buffer& buffer_sequence,
      const Buffer&)
  {
    return Buffer(buffer_sequence);
  }

private:
  native_buffer_type buffer_;       //只有一个单独的本地缓冲结构
  std::size_t total_buffer_size_;
};
///===================================================================================================================================================================== 

#if !defined(ASIO_NO_DEPRECATED)

template <typename Buffer>
class buffer_sequence_adapter<Buffer, asio::mutable_buffers_1>
  : buffer_sequence_adapter_base
{
public:
  enum { is_single_buffer = true };
  enum { is_registered_buffer = false };

  explicit buffer_sequence_adapter(
      const asio::mutable_buffers_1& buffer_sequence)
  {
    init_native_buffer(buffer_, Buffer(buffer_sequence));
    total_buffer_size_ = buffer_sequence.size();
  }

  native_buffer_type* buffers()
  {
    return &buffer_;
  }

  std::size_t count() const
  {
    return 1;
  }

  std::size_t total_size() const
  {
    return total_buffer_size_;
  }

  registered_buffer_id registered_id() const
  {
    return registered_buffer_id();
  }

  bool all_empty() const
  {
    return total_buffer_size_ == 0;
  }

  static bool all_empty(const asio::mutable_buffers_1& buffer_sequence)
  {
    return buffer_sequence.size() == 0;
  }

  static void validate(const asio::mutable_buffers_1& buffer_sequence)
  {
    buffer_sequence.data();
  }

  static Buffer first(const asio::mutable_buffers_1& buffer_sequence)
  {
    return Buffer(buffer_sequence);
  }

  enum { linearisation_storage_size = 1 };

  static Buffer linearise(const asio::mutable_buffers_1& buffer_sequence,
      const Buffer&)
  {
    return Buffer(buffer_sequence);
  }

private:
  native_buffer_type buffer_;
  std::size_t total_buffer_size_;
};

template <typename Buffer>
class buffer_sequence_adapter<Buffer, asio::const_buffers_1>
  : buffer_sequence_adapter_base
{
public:
  enum { is_single_buffer = true };
  enum { is_registered_buffer = false };

  explicit buffer_sequence_adapter(
      const asio::const_buffers_1& buffer_sequence)
  {
    init_native_buffer(buffer_, Buffer(buffer_sequence));
    total_buffer_size_ = buffer_sequence.size();
  }

  native_buffer_type* buffers()
  {
    return &buffer_;
  }

  std::size_t count() const
  {
    return 1;
  }

  std::size_t total_size() const
  {
    return total_buffer_size_;
  }

  registered_buffer_id registered_id() const
  {
    return registered_buffer_id();
  }

  bool all_empty() const
  {
    return total_buffer_size_ == 0;
  }

  static bool all_empty(const asio::const_buffers_1& buffer_sequence)
  {
    return buffer_sequence.size() == 0;
  }

  static void validate(const asio::const_buffers_1& buffer_sequence)
  {
    buffer_sequence.data();
  }

  static Buffer first(const asio::const_buffers_1& buffer_sequence)
  {
    return Buffer(buffer_sequence);
  }

  enum { linearisation_storage_size = 1 };

  static Buffer linearise(const asio::const_buffers_1& buffer_sequence,
      const Buffer&)
  {
    return Buffer(buffer_sequence);
  }

private:
  native_buffer_type buffer_;
  std::size_t total_buffer_size_;
};

#endif // !defined(ASIO_NO_DEPRECATED)
///===================================================================================================================================================================== 

template <typename Buffer>
class buffer_sequence_adapter<Buffer, asio::mutable_registered_buffer>  // 上面的特化模板【可变注册缓冲】
  : buffer_sequence_adapter_base
{
public:
  enum { is_single_buffer = true }; //单缓冲
  enum { is_registered_buffer = true }; //注册缓冲区

  explicit buffer_sequence_adapter(
      const asio::mutable_registered_buffer& buffer_sequence)
  {
    init_native_buffer(buffer_, buffer_sequence.buffer());
    total_buffer_size_ = buffer_sequence.size();
    registered_id_ = buffer_sequence.id();
  }

  native_buffer_type* buffers()
  {
    return &buffer_;
  }

  std::size_t count() const //count就是1
  {
    return 1;
  }

  std::size_t total_size() const
  {
    return total_buffer_size_;
  }

  registered_buffer_id registered_id() const
  {
    return registered_id_;
  }

  bool all_empty() const
  {
    return total_buffer_size_ == 0;
  }

  static bool all_empty(
      const asio::mutable_registered_buffer& buffer_sequence)
  {
    return buffer_sequence.size() == 0;
  }

  static void validate(
      const asio::mutable_registered_buffer& buffer_sequence)
  {
    buffer_sequence.data();
  }

  static Buffer first(
      const asio::mutable_registered_buffer& buffer_sequence)
  {
    return Buffer(buffer_sequence.buffer());
  }

  enum { linearisation_storage_size = 1 };

  static Buffer linearise(
      const asio::mutable_registered_buffer& buffer_sequence,
      const Buffer&)
  {
    return Buffer(buffer_sequence.buffer());
  }

private:
  native_buffer_type buffer_;   //单独的本地缓冲结构 
  std::size_t total_buffer_size_;
  registered_buffer_id registered_id_; //还有一个注册缓冲区的id
};

///===================================================================================================================================================================== 

template <typename Buffer>
class buffer_sequence_adapter<Buffer, asio::const_registered_buffer>
  : buffer_sequence_adapter_base
{
public:
  enum { is_single_buffer = true };
  enum { is_registered_buffer = true };

  explicit buffer_sequence_adapter(
      const asio::const_registered_buffer& buffer_sequence)
  {
    init_native_buffer(buffer_, buffer_sequence.buffer());
    total_buffer_size_ = buffer_sequence.size();
    registered_id_ = buffer_sequence.id();
  }

  native_buffer_type* buffers()
  {
    return &buffer_;
  }

  std::size_t count() const
  {
    return 1;
  }

  std::size_t total_size() const
  {
    return total_buffer_size_;
  }

  registered_buffer_id registered_id() const
  {
    return registered_id_;
  }

  bool all_empty() const
  {
    return total_buffer_size_ == 0;
  }

  static bool all_empty(
      const asio::const_registered_buffer& buffer_sequence)
  {
    return buffer_sequence.size() == 0;
  }

  static void validate(
      const asio::const_registered_buffer& buffer_sequence)
  {
    buffer_sequence.data();
  }

  static Buffer first(
      const asio::const_registered_buffer& buffer_sequence)
  {
    return Buffer(buffer_sequence.buffer());
  }

  enum { linearisation_storage_size = 1 };

  static Buffer linearise(
      const asio::const_registered_buffer& buffer_sequence,
      const Buffer&)
  {
    return Buffer(buffer_sequence.buffer());
  }

private:
  native_buffer_type buffer_;
  std::size_t total_buffer_size_;
  registered_buffer_id registered_id_;
};

///===================================================================================================================================================================== 

template <typename Buffer, typename Elem>
class buffer_sequence_adapter<Buffer, boost::array<Elem, 2> >
  : buffer_sequence_adapter_base
{
public:
  enum { is_single_buffer = false };
  enum { is_registered_buffer = false };

  explicit buffer_sequence_adapter(
      const boost::array<Elem, 2>& buffer_sequence)
  {
    init_native_buffer(buffers_[0], Buffer(buffer_sequence[0]));
    init_native_buffer(buffers_[1], Buffer(buffer_sequence[1]));
    total_buffer_size_ = buffer_sequence[0].size() + buffer_sequence[1].size();
  }

  native_buffer_type* buffers()
  {
    return buffers_;
  }

  std::size_t count() const
  {
    return 2;
  }

  std::size_t total_size() const
  {
    return total_buffer_size_;
  }

  registered_buffer_id registered_id() const
  {
    return registered_buffer_id();
  }

  bool all_empty() const
  {
    return total_buffer_size_ == 0;
  }

  static bool all_empty(const boost::array<Elem, 2>& buffer_sequence)
  {
    return buffer_sequence[0].size() == 0 && buffer_sequence[1].size() == 0;
  }

  static void validate(const boost::array<Elem, 2>& buffer_sequence)
  {
    buffer_sequence[0].data();
    buffer_sequence[1].data();
  }

  static Buffer first(const boost::array<Elem, 2>& buffer_sequence)
  {
    return Buffer(buffer_sequence[0].size() != 0
        ? buffer_sequence[0] : buffer_sequence[1]);
  }

  enum { linearisation_storage_size = 8192 };

  static Buffer linearise(const boost::array<Elem, 2>& buffer_sequence,
      const asio::mutable_buffer& storage)
  {
    if (buffer_sequence[0].size() == 0)
      return Buffer(buffer_sequence[1]);
    if (buffer_sequence[1].size() == 0)
      return Buffer(buffer_sequence[0]);
    return Buffer(storage.data(),
        asio::buffer_copy(storage, buffer_sequence));
  }

private:
  native_buffer_type buffers_[2];
  std::size_t total_buffer_size_;
};

///===================================================================================================================================================================== 
#if defined(ASIO_HAS_STD_ARRAY)

template <typename Buffer, typename Elem>
class buffer_sequence_adapter<Buffer, std::array<Elem, 2> > //传入大小为2的std数组【不太清楚为什么大小为2】
  : buffer_sequence_adapter_base
{
public:
  enum { is_single_buffer = false };    // 非单缓冲
  enum { is_registered_buffer = false };    //也非注册缓冲

  explicit buffer_sequence_adapter(
      const std::array<Elem, 2>& buffer_sequence)
  {
    init_native_buffer(buffers_[0], Buffer(buffer_sequence[0]));
    init_native_buffer(buffers_[1], Buffer(buffer_sequence[1]));
    total_buffer_size_ = buffer_sequence[0].size() + buffer_sequence[1].size();
  }

  native_buffer_type* buffers()
  {
    return buffers_;
  }

  std::size_t count() const //传进来的大小为2，那么传出去的大小也为2
  {
    return 2;
  }

  std::size_t total_size() const
  {
    return total_buffer_size_;
  }

  registered_buffer_id registered_id() const
  {
    return registered_buffer_id();
  }

  bool all_empty() const
  {
    return total_buffer_size_ == 0;
  }

  static bool all_empty(const std::array<Elem, 2>& buffer_sequence)
  {
    return buffer_sequence[0].size() == 0 && buffer_sequence[1].size() == 0;
  }

  static void validate(const std::array<Elem, 2>& buffer_sequence)
  {
    buffer_sequence[0].data();
    buffer_sequence[1].data();
  }

  static Buffer first(const std::array<Elem, 2>& buffer_sequence)
  {
    return Buffer(buffer_sequence[0].size() != 0
        ? buffer_sequence[0] : buffer_sequence[1]);
  }

  enum { linearisation_storage_size = 8192 };   //最大长度8192

  static Buffer linearise(const std::array<Elem, 2>& buffer_sequence,
      const asio::mutable_buffer& storage)
  {
    if (buffer_sequence[0].size() == 0)
      return Buffer(buffer_sequence[1]);
    if (buffer_sequence[1].size() == 0)
      return Buffer(buffer_sequence[0]);
    return Buffer(storage.data(),
        asio::buffer_copy(storage, buffer_sequence));
  }

private:
  native_buffer_type buffers_[2];
  std::size_t total_buffer_size_;
};

#endif // defined(ASIO_HAS_STD_ARRAY)

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "asio/detail/impl/buffer_sequence_adapter.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_DETAIL_BUFFER_SEQUENCE_ADAPTER_HPP
