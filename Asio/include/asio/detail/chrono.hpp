//
// detail/chrono.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_CHRONO_HPP
#define ASIO_DETAIL_CHRONO_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_STD_CHRONO)
# include <chrono>
#elif defined(ASIO_HAS_BOOST_CHRONO)
# include <boost/chrono/system_clocks.hpp>
#endif // defined(ASIO_HAS_BOOST_CHRONO)

namespace asio {
namespace chrono {

#if defined(ASIO_HAS_STD_CHRONO)
using std::chrono::duration;	//一段时间
using std::chrono::time_point;	//时间点
using std::chrono::duration_cast;	//时间段转换
using std::chrono::nanoseconds;		//纳秒
using std::chrono::microseconds;	//微秒
using std::chrono::milliseconds;	//毫秒
using std::chrono::seconds;	//秒
using std::chrono::minutes;	//分钟
using std::chrono::hours;	//小时
using std::chrono::time_point_cast; //时间点转换
#if defined(ASIO_HAS_STD_CHRONO_MONOTONIC_CLOCK)
typedef std::chrono::monotonic_clock steady_clock;
#else // defined(ASIO_HAS_STD_CHRONO_MONOTONIC_CLOCK)
using std::chrono::steady_clock;
#endif // defined(ASIO_HAS_STD_CHRONO_MONOTONIC_CLOCK)
using std::chrono::system_clock;
using std::chrono::high_resolution_clock;	//高精度时钟
#elif defined(ASIO_HAS_BOOST_CHRONO)
using boost::chrono::duration;
using boost::chrono::time_point;
using boost::chrono::duration_cast;
using boost::chrono::nanoseconds;
using boost::chrono::microseconds;
using boost::chrono::milliseconds;
using boost::chrono::seconds;
using boost::chrono::minutes;
using boost::chrono::hours;
using boost::chrono::time_point_cast;
using boost::chrono::system_clock;
using boost::chrono::steady_clock;
using boost::chrono::high_resolution_clock;
#endif // defined(ASIO_HAS_BOOST_CHRONO)

} // namespace chrono
} // namespace asio

#endif // ASIO_DETAIL_CHRONO_HPP
