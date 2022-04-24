//
// TAKO:win下面任务段指针的函数
// detail/impl/win_tss_ptr.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IMPL_WIN_TSS_PTR_IPP
#define ASIO_DETAIL_IMPL_WIN_TSS_PTR_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_WINDOWS)

#include "asio/detail/throw_error.hpp"
#include "asio/detail/win_tss_ptr.hpp"
#include "asio/error.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

DWORD win_tss_ptr_create()
{
#if defined(UNDER_CE)
  const DWORD out_of_indexes = 0xFFFFFFFF;
#else
  const DWORD out_of_indexes = TLS_OUT_OF_INDEXES;  //最大限制32位[u32]
#endif

  DWORD tss_key = ::TlsAlloc(); //要使用TLS，先调用TlsAlloc() 
  if (tss_key == out_of_indexes)    //得在范围内，出了就是分配错了
  {
    DWORD last_error = ::GetLastError();
    asio::error_code ec(last_error,
        asio::error::get_system_category());
    asio::detail::throw_error(ec, "tss");
  }
  return tss_key;   //返回指示字段dword
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_WINDOWS)

#endif // ASIO_DETAIL_IMPL_WIN_TSS_PTR_IPP
