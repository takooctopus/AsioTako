//
// 套接字类型
// detail/socket_types.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_SOCKET_TYPES_HPP
#define ASIO_DETAIL_SOCKET_TYPES_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_WINDOWS_RUNTIME)
// Empty.
#elif defined(ASIO_WINDOWS) || defined(__CYGWIN__)
# if defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
#  error WinSock.h has already been included
# endif // defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
# if defined(__BORLANDC__)
#  include <stdlib.h> // Needed for __errno
#  if !defined(_WSPIAPI_H_)
#   define _WSPIAPI_H_
#   define ASIO_WSPIAPI_H_DEFINED
#  endif // !defined(_WSPIAPI_H_)
# endif // defined(__BORLANDC__)
# include <winsock2.h>
# include <ws2tcpip.h>
# if defined(WINAPI_FAMILY)
#  if ((WINAPI_FAMILY & WINAPI_PARTITION_DESKTOP) != 0)
#   include <windows.h>
#  endif // ((WINAPI_FAMILY & WINAPI_PARTITION_DESKTOP) != 0)
# endif // defined(WINAPI_FAMILY)
# if !defined(ASIO_WINDOWS_APP)
#  include <mswsock.h>
# endif // !defined(ASIO_WINDOWS_APP)
# if defined(ASIO_WSPIAPI_H_DEFINED)
#  undef _WSPIAPI_H_
#  undef ASIO_WSPIAPI_H_DEFINED
# endif // defined(ASIO_WSPIAPI_H_DEFINED)
# if !defined(ASIO_NO_DEFAULT_LINKED_LIBS)
#  if defined(UNDER_CE)
#   pragma comment(lib, "ws2.lib")
#  elif defined(_MSC_VER) || defined(__BORLANDC__)
#   pragma comment(lib, "ws2_32.lib")
#   if !defined(ASIO_WINDOWS_APP)
#    pragma comment(lib, "mswsock.lib")
#   endif // !defined(ASIO_WINDOWS_APP)
#  endif // defined(_MSC_VER) || defined(__BORLANDC__)
# endif // !defined(ASIO_NO_DEFAULT_LINKED_LIBS)
# include "asio/detail/old_win_sdk_compat.hpp"
#else
# include <sys/ioctl.h>
# if (defined(__MACH__) && defined(__APPLE__)) \
   || defined(__FreeBSD__) || defined(__NetBSD__) \
   || defined(__OpenBSD__) || defined(__linux__) \
   || defined(__EMSCRIPTEN__)
#  include <poll.h>	//poll.h
# elif !defined(__SYMBIAN32__)
#  include <sys/poll.h>	//没有定义塞班
# endif
# include <sys/types.h> //【定义了基本数据类型pthread_t、pthread_mutex_t、in_addr_t、pid_t等等】
# include <sys/stat.h>	// 文件设备等【主要函数stat(pathname, buf)可以获取文件信息，像id啊，编号啊，保护模式啊，用户组id啊，总体尺寸啊】
# include <fcntl.h>	//常用的原型函数完成编程中对文件的打开、数据写入、数据读取、关闭文件的操作
# if defined(__hpux)
#  include <sys/time.h>
# endif
# if !defined(__hpux) || defined(__SELECT)
#  include <sys/select.h> // select当然，我们不喜欢select
# endif
# include <sys/socket.h> // 套接字
# include <sys/uio.h>	// 矢量io操作
# include <sys/un.h>	// UNIX域套接字定义
# include <netinet/in.h>	// INTERNET地址族
# if !defined(__SYMBIAN32__)
#  include <netinet/tcp.h> // 传输控制协议定义
# endif
# include <arpa/inet.h>	//INTERNET定义
# include <netdb.h>	//网络数据库操作
# include <net/if.h>	//套接字本地接口
# include <limits.h>	//定义各种数据类型最值的常量
# if defined(__sun)
#  include <sys/filio.h>
#  include <sys/sockio.h>
# endif
#endif

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

#if defined(ASIO_WINDOWS_RUNTIME)
const int max_addr_v4_str_len = 256;
const int max_addr_v6_str_len = 256;
typedef unsigned __int32 u_long_type;
typedef unsigned __int16 u_short_type;
struct in4_addr_type { u_long_type s_addr; };
struct in4_mreq_type { in4_addr_type imr_multiaddr, imr_interface; };
struct in6_addr_type { unsigned char s6_addr[16]; };
struct in6_mreq_type { in6_addr_type ipv6mr_multiaddr;
  unsigned long ipv6mr_interface; };
struct socket_addr_type { int sa_family; };
struct sockaddr_in4_type { int sin_family;
  in4_addr_type sin_addr; u_short_type sin_port; };
struct sockaddr_in6_type { int sin6_family;
  in6_addr_type sin6_addr; u_short_type sin6_port;
  u_long_type sin6_flowinfo; u_long_type sin6_scope_id; };
struct sockaddr_storage_type { int ss_family;
  unsigned char ss_bytes[128 - sizeof(int)]; };
struct addrinfo_type { int ai_flags;
  int ai_family, ai_socktype, ai_protocol;
  int ai_addrlen; const void* ai_addr;
  const char* ai_canonname; addrinfo_type* ai_next; };
struct linger_type { u_short_type l_onoff, l_linger; };
typedef u_long_type ioctl_arg_type;
typedef int signed_size_type;
# define ASIO_OS_DEF(c) ASIO_OS_DEF_##c
# define ASIO_OS_DEF_AF_UNSPEC 0
# define ASIO_OS_DEF_AF_INET 2
# define ASIO_OS_DEF_AF_INET6 23
# define ASIO_OS_DEF_SOCK_STREAM 1
# define ASIO_OS_DEF_SOCK_DGRAM 2
# define ASIO_OS_DEF_SOCK_RAW 3
# define ASIO_OS_DEF_SOCK_SEQPACKET 5
# define ASIO_OS_DEF_IPPROTO_IP 0
# define ASIO_OS_DEF_IPPROTO_IPV6 41
# define ASIO_OS_DEF_IPPROTO_TCP 6
# define ASIO_OS_DEF_IPPROTO_UDP 17
# define ASIO_OS_DEF_IPPROTO_ICMP 1
# define ASIO_OS_DEF_IPPROTO_ICMPV6 58
# define ASIO_OS_DEF_FIONBIO 1
# define ASIO_OS_DEF_FIONREAD 2
# define ASIO_OS_DEF_INADDR_ANY 0
# define ASIO_OS_DEF_MSG_OOB 0x1
# define ASIO_OS_DEF_MSG_PEEK 0x2
# define ASIO_OS_DEF_MSG_DONTROUTE 0x4
# define ASIO_OS_DEF_MSG_EOR 0 // Not supported.
# define ASIO_OS_DEF_SHUT_RD 0x0
# define ASIO_OS_DEF_SHUT_WR 0x1
# define ASIO_OS_DEF_SHUT_RDWR 0x2
# define ASIO_OS_DEF_SOMAXCONN 0x7fffffff
# define ASIO_OS_DEF_SOL_SOCKET 0xffff
# define ASIO_OS_DEF_SO_BROADCAST 0x20
# define ASIO_OS_DEF_SO_DEBUG 0x1
# define ASIO_OS_DEF_SO_DONTROUTE 0x10
# define ASIO_OS_DEF_SO_KEEPALIVE 0x8
# define ASIO_OS_DEF_SO_LINGER 0x80
# define ASIO_OS_DEF_SO_OOBINLINE 0x100
# define ASIO_OS_DEF_SO_SNDBUF 0x1001
# define ASIO_OS_DEF_SO_RCVBUF 0x1002
# define ASIO_OS_DEF_SO_SNDLOWAT 0x1003
# define ASIO_OS_DEF_SO_RCVLOWAT 0x1004
# define ASIO_OS_DEF_SO_REUSEADDR 0x4
# define ASIO_OS_DEF_TCP_NODELAY 0x1
# define ASIO_OS_DEF_IP_MULTICAST_IF 2
# define ASIO_OS_DEF_IP_MULTICAST_TTL 3
# define ASIO_OS_DEF_IP_MULTICAST_LOOP 4
# define ASIO_OS_DEF_IP_ADD_MEMBERSHIP 5
# define ASIO_OS_DEF_IP_DROP_MEMBERSHIP 6
# define ASIO_OS_DEF_IP_TTL 7
# define ASIO_OS_DEF_IPV6_UNICAST_HOPS 4
# define ASIO_OS_DEF_IPV6_MULTICAST_IF 9
# define ASIO_OS_DEF_IPV6_MULTICAST_HOPS 10
# define ASIO_OS_DEF_IPV6_MULTICAST_LOOP 11
# define ASIO_OS_DEF_IPV6_JOIN_GROUP 12
# define ASIO_OS_DEF_IPV6_LEAVE_GROUP 13
# define ASIO_OS_DEF_AI_CANONNAME 0x2
# define ASIO_OS_DEF_AI_PASSIVE 0x1
# define ASIO_OS_DEF_AI_NUMERICHOST 0x4
# define ASIO_OS_DEF_AI_NUMERICSERV 0x8
# define ASIO_OS_DEF_AI_V4MAPPED 0x800
# define ASIO_OS_DEF_AI_ALL 0x100
# define ASIO_OS_DEF_AI_ADDRCONFIG 0x400
#elif defined(ASIO_WINDOWS) || defined(__CYGWIN__)
typedef SOCKET socket_type;
const SOCKET invalid_socket = INVALID_SOCKET; //(u64)-1
const int socket_error_retval = SOCKET_ERROR; //-1
const int max_addr_v4_str_len = 256;	//ipv4带点名字最长256字符长度
const int max_addr_v6_str_len = 256;	//ipv6带点名字最长256字符长度
typedef sockaddr socket_addr_type;	// win下面的sock_addr【16byte】
typedef in_addr in4_addr_type;	//【通用地址结构体16byte】
typedef ip_mreq in4_mreq_type;	//组播结构体[多播组的IP地址，本机的IP地址]
typedef sockaddr_in sockaddr_in4_type; //sockaddr_in ipv4别名【16byte】
# if defined(ASIO_HAS_OLD_WIN_SDK)
typedef in6_addr_emulation in6_addr_type;
typedef ipv6_mreq_emulation in6_mreq_type;
typedef sockaddr_in6_emulation sockaddr_in6_type;
typedef sockaddr_storage_emulation sockaddr_storage_type;
typedef addrinfo_emulation addrinfo_type;
# else
typedef in6_addr in6_addr_type; //ipv6地址
typedef ipv6_mreq in6_mreq_type; //ipv6组播地址
typedef sockaddr_in6 sockaddr_in6_type;	//sockaddr_in6【28byte[注意win下面unsigned long 是u32, linux下 unsigned long 是u64]】
typedef sockaddr_storage sockaddr_storage_type; //【128byte】
typedef addrinfo addrinfo_type; //<netdb.h> addrinfo结构主要在网络编程解析hostname时使用 getaddrinfo(const char, const char, const struct addrinfo, struct addrinfo*) => freeaddrinfo(struct addrinfo*) 记得拿了要释放掉
# endif
typedef ::linger linger_type; // 【延迟linger的结构体】【win下u16+u16】
typedef unsigned long ioctl_arg_type; //u64 
typedef u_long u_long_type; //u64
typedef u_short u_short_type;	//u16
typedef int signed_size_type; //s32
struct sockaddr_un_type { u_short sun_family; char sun_path[108]; }; //u16 + u8*108 = 110byte 本机传输的sockaddr
# define ASIO_OS_DEF(c) ASIO_OS_DEF_##c //使用宏来创建【ip和socket相关】常量
# define ASIO_OS_DEF_AF_UNSPEC AF_UNSPEC
# define ASIO_OS_DEF_AF_INET AF_INET
# define ASIO_OS_DEF_AF_INET6 AF_INET6	// 地址协议
# define ASIO_OS_DEF_SOCK_STREAM SOCK_STREAM
# define ASIO_OS_DEF_SOCK_DGRAM SOCK_DGRAM
# define ASIO_OS_DEF_SOCK_RAW SOCK_RAW
# define ASIO_OS_DEF_SOCK_SEQPACKET SOCK_SEQPACKET //数据类型
# define ASIO_OS_DEF_IPPROTO_IP IPPROTO_IP
# define ASIO_OS_DEF_IPPROTO_IPV6 IPPROTO_IPV6
# define ASIO_OS_DEF_IPPROTO_TCP IPPROTO_TCP
# define ASIO_OS_DEF_IPPROTO_UDP IPPROTO_UDP
# define ASIO_OS_DEF_IPPROTO_ICMP IPPROTO_ICMP
# define ASIO_OS_DEF_IPPROTO_ICMPV6 IPPROTO_ICMPV6 //指定socket创建协议
# define ASIO_OS_DEF_FIONBIO FIONBIO	// 允许或者禁止套接字口的非阻塞模式【允许则为0，不允许则为1】
# define ASIO_OS_DEF_FIONREAD FIONREAD	//  ioctl(0,FIONREAD,&nread); 能得到緩衝區裡面有多少字節要被讀取。值放在 nread裡面了。
# define ASIO_OS_DEF_INADDR_ANY INADDR_ANY // u32 0 => 0.0.0.0
# define ASIO_OS_DEF_MSG_OOB MSG_OOB //	发送紧急数据 send(sockfd,＂X＂, 1,MSG_OOB) [RFC6093已经建议不要在继续使用紧急数据了]
# define ASIO_OS_DEF_MSG_PEEK MSG_PEEK // ssize_t recv(int s, void *buf, size_t len, int flags) flags设置成MSG_PEEK，buf中的数据不被移除 ,多进程需要读取相同数据的时候可以使用
# define ASIO_OS_DEF_MSG_DONTROUTE MSG_DONTROUTE // send函数使用，告诉IP协议，目的主机在本地网络上，不需要查找路由表。
# define ASIO_OS_DEF_MSG_EOR 0 // Not supported on Windows.
# define ASIO_OS_DEF_SHUT_RD SD_RECEIVE // int shutdown(SOCKET s, int howto);  【Windows】关闭接收操作，也就是断开输入流
# define ASIO_OS_DEF_SHUT_WR SD_SEND	// int shutdown(SOCKET s, int howto);  【Windows】关闭发送操作，也就是断开输出流
# define ASIO_OS_DEF_SHUT_RDWR SD_BOTH	// int shutdown(SOCKET s, int howto);  【Windows】同时关闭接收和发送操作	
# define ASIO_OS_DEF_SOMAXCONN SOMAXCONN	// 系统中每一个端口最大的监听队列的长度,这是个全局的参数,默认值为128.限制了每个端口接收新tcp连接侦听队列的大小。 listen(ListenSocket, SOMAXCONN)
# define ASIO_OS_DEF_SOL_SOCKET SOL_SOCKET	// int setsockopt(int socket, int level[SOL_SOCKET], int option_name, const void *option_value, size_t option_len); 设置套接字级别=>option_namey【SO_DEBUG, SO_REUSEADDR, SO_DONTROUTE, SO_BROADCAST, SO_SNDBUF, SO_RCVBUF, SO_KEEPALIVE, SO_OOBINLINE, SO_NO_CHECK, SO_PRIORITY, SO_LINGER, SO_PASSCRED, SO_TIMESTAMP, SO_RCVLOWAT, SO_RCVTIMEO, SO_SNDTIMEO, SO_BINDTODEVICE, SO_ATTACH_FILTER, SO_DETACH_FILTER】 具体具体去看
# define ASIO_OS_DEF_SO_BROADCAST SO_BROADCAST // 允许或禁止发送广播数据 当option_value不等于0时，允许，否则，禁止。它实际所做的工作是在sock->sk->sk_flag中置或清 SOCK_BROADCAST位。
# define ASIO_OS_DEF_SO_DEBUG SO_DEBUG // 当option_value不等于0时，打开调试信息，否则，关闭调试信息。仅仅TCP支持。当打开此选项时，内核对TCP在此套接口所发送和接收的所有分组跟踪详细信息。这些信息保存在内核的环形缓冲区内，可由程序trpt进行检查。
# define ASIO_OS_DEF_SO_DONTROUTE SO_DONTROUTE //打开或关闭路由查找功能 当option_value不等于0时，打开；否则，关闭。此选项规定发出的分组将旁路底层协议的正常路由机制。该选项经常由路由守护进程（routed和gated）用来旁路路由表（路由表不正确的情况下），强制一个分组从某个特定接口发出。
# define ASIO_OS_DEF_SO_KEEPALIVE SO_KEEPALIVE //套接字保活  如果协议是TCP，并且当前的套接字状态不是侦听(listen)或关闭(close)，那么，当option_value不是零时，启用TCP保活定时 器，否则关闭保活定时器。打开此选项后，如果2小时内在此套接口上没有任何数据交换，TCP就会自动给对方发一个保持存活探测分节，结果如下：1.对方以期望的ACK响应，则一切正常，应用程序得不到通知；2.对方以RST响应，套接口的待处理错误被置为ECONNRESET，套接口本身则被关闭；3.对方对探测分节无任何响应，经过重试都没有任何响应，套接口的待处理错误被置为ETIMEOUT，套接口本身被关闭；若接收到一个ICMP错误作为某个探测分节的响应，则返回相应错误。
# define ASIO_OS_DEF_SO_LINGER SO_LINGER //如果选择此选项，close或 shutdown将等到所有套接字里排队的消息成功发送或到达延迟时间后才会返回。否则，调用将立即返回。
# define ASIO_OS_DEF_SO_OOBINLINE SO_OOBINLINE //紧急数据放入普通数据流 该操作根据option_value的值置或清sock->sk->sk_flag中的SOCK_URGINLINE位。带外数据将被保留在正常的输入队列中（即在线存放）。当发生这种情况时，接收函数的MSG_OOB标志不能用来读带外数据。
# define ASIO_OS_DEF_SO_SNDBUF SO_SNDBUF //设置发送缓冲区的大小 发送缓冲区的大小是有上下限的，其上限为256 * (sizeof(struct sk_buff) + 256)，下限为2048字节。该操作将sock->sk->sk_sndbuf设置为val * 2，之所以要乘以2，是防止大数据量的发送，突然导致缓冲区溢出。最后，该操作完成后，因为对发送缓冲的大小 作了改变，要检查sleep队列，如果有进程正在等待写，将它们唤醒。
# define ASIO_OS_DEF_SO_RCVBUF SO_RCVBUF // 设置接收缓冲区的大小。接收缓冲区大小的上下限分别是：256 * (sizeof(struct sk_buff) + 256)和256字节。该操作将sock->sk->sk_rcvbuf设置为val * 2。
# define ASIO_OS_DEF_SO_SNDLOWAT SO_SNDLOWAT //指定缓冲区中的最小字节数，直到套接字层将数据传递给协议 默认为2048字节
# define ASIO_OS_DEF_SO_RCVLOWAT SO_RCVLOWAT //接收缓冲区中的数据必须达到规定数量，内核才通知进程“可读”。默认为1字节
# define ASIO_OS_DEF_SO_REUSEADDR SO_REUSEADDR //打开或关闭地址复用功能
# define ASIO_OS_DEF_TCP_NODELAY TCP_NODELAY //禁用了Nagle算法，允许小包的发送。对于延时敏感型，同时数据传输量比较小的应用，开启TCP_NODELAY选项无疑是一个正确的选择
# define ASIO_OS_DEF_IP_MULTICAST_IF IP_MULTICAST_IF // 设置组播的默认默认网络接口，会从给定的网络接口发送，另一个网络接口会忽略此数据
# define ASIO_OS_DEF_IP_MULTICAST_TTL IP_MULTICAST_TTL // 设置多播组数据的TTL值 [0-255]
# define ASIO_OS_DEF_IP_MULTICAST_LOOP IP_MULTICAST_LOOP // 禁止组播数据回送
# define ASIO_OS_DEF_IP_ADD_MEMBERSHIP IP_ADD_MEMBERSHIP //在指定接口上加入组播组
# define ASIO_OS_DEF_IP_DROP_MEMBERSHIP IP_DROP_MEMBERSHIP //退出组播组
# define ASIO_OS_DEF_IP_TTL IP_TTL	//设置主机发送数据包的生存时间
# define ASIO_OS_DEF_IPV6_UNICAST_HOPS IPV6_UNICAST_HOPS // 类似于IPv4的IP_TTL，它的设置指定发送到套接口上的外出数据报的缺省跳限，而它的获取则返回内核将用于套接口的跳限值。为了从接收到的IPv6数据报中得到真实的跳限字段，要求使用IPV6_HOPLIMIT套接口选项。
# define ASIO_OS_DEF_IPV6_MULTICAST_IF IPV6_MULTICAST_IF //指定外出接口
# define ASIO_OS_DEF_IPV6_MULTICAST_HOPS IPV6_MULTICAST_HOPS //指定外出跳限
# define ASIO_OS_DEF_IPV6_MULTICAST_LOOP IPV6_MULTICAST_LOOP //指定是否回馈
# define ASIO_OS_DEF_IPV6_JOIN_GROUP IPV6_JOIN_GROUP	//加入多播组
# define ASIO_OS_DEF_IPV6_LEAVE_GROUP IPV6_LEAVE_GROUP //离开多播组
# define ASIO_OS_DEF_AI_CANONNAME AI_CANONNAME //int getaddrinfo( const char *hostname, const char *service, const struct addrinfo *hints, struct addrinfo **result ); addrinfo的标识 在函数所返回的第一个addrinfo结构中的ai_cannoname成员中，应该包含一个以空字符结尾的字符串，字符串的内容是节点名的正规名
# define ASIO_OS_DEF_AI_PASSIVE AI_PASSIVE //同上,当此标志置位时，表示调用者将在bind()函数调用中使用返回的地址结构。当此标志不置位时，表示将在connect()函数调用中使用。当节点名位 NULL，且此标志置位，则返回的地址将是通配地址。如果节点名NULL，且此标志不置位，则返回的地址将是回环地址。
# define ASIO_OS_DEF_AI_NUMERICHOST AI_NUMERICHOST // 同上当此标志置位时，此标志表示调用中的节点名必须是一个数字地址字符串。
# if defined(AI_NUMERICSERV)
#  define ASIO_OS_DEF_AI_NUMERICSERV AI_NUMERICSERV //以端口号返回服务.
# else
#  define ASIO_OS_DEF_AI_NUMERICSERV 0
# endif
# if defined(AI_V4MAPPED)
#  define ASIO_OS_DEF_AI_V4MAPPED AI_V4MAPPED //如果没有找到IPv6地址, 则返回映射到IPv6格式的IPv6地址.
# else
#  define ASIO_OS_DEF_AI_V4MAPPED 0
# endif
# if defined(AI_ALL)
#  define ASIO_OS_DEF_AI_ALL AI_ALL //查找IPv4和IPv6地址(仅用于AI_V4MAPPED).
# else
#  define ASIO_OS_DEF_AI_ALL 0
# endif
# if defined(AI_ADDRCONFIG)
#  define ASIO_OS_DEF_AI_ADDRCONFIG AI_ADDRCONFIG //查询配置的地址类型(IPv4或IPv6)
# else
#  define ASIO_OS_DEF_AI_ADDRCONFIG 0
# endif
# if defined (_WIN32_WINNT)
const int max_iov_len = 64;
# else
const int max_iov_len = 16;
# endif
#else
// linux下面的socket定义
typedef int socket_type;	//这里采用int
const int invalid_socket = -1;	//-1
const int socket_error_retval = -1;	//-1
const int max_addr_v4_str_len = INET_ADDRSTRLEN; //ipv4字符串长度xxx.xxx.xxx.xxx:65535\0【linux中是16，win中是22】
#if defined(INET6_ADDRSTRLEN)
const int max_addr_v6_str_len = INET6_ADDRSTRLEN + 1 + IF_NAMESIZE; //linux ipv6字符串长度 【1111:1111:1111:1111:1111:1111:111.111.111.111 => 46[win下65]】 【% => 1】 【接口长度】 
#else // defined(INET6_ADDRSTRLEN)
const int max_addr_v6_str_len = 256; //最大地址字符串长度256
#endif // defined(INET6_ADDRSTRLEN)
typedef sockaddr socket_addr_type; //定义通用sockaddr
typedef in_addr in4_addr_type; //ipv4d地址
# if defined(__hpux)
// HP-UX doesn't provide ip_mreq when _XOPEN_SOURCE_EXTENDED is defined.
struct in4_mreq_type	//ipv4多播地址
{
  struct in_addr imr_multiaddr;
  struct in_addr imr_interface;
};
# else
typedef ip_mreq in4_mreq_type;
# endif
typedef sockaddr_in sockaddr_in4_type;	//sockaddr_in 16byte
typedef in6_addr in6_addr_type;	//ipv6地址
typedef ipv6_mreq in6_mreq_type;	//ipv6多播
typedef sockaddr_in6 sockaddr_in6_type; //sockaddr_in6 28byte
typedef sockaddr_storage sockaddr_storage_type; //256byte
typedef sockaddr_un sockaddr_un_type; // 主要用于同一个主机中的本地Local socket 域参数应该是PF_LOCAL或者PF_UNIX 本地套接字的通讯类型应该是SOCK_STREAM或SOCK_DGRAM，协议为默认协议 sockfd = socket(PF_LOCAL, SOCK_STREAM, 0);
typedef addrinfo addrinfo_type; //可以获取地址信息
typedef ::linger linger_type; // 延迟
typedef int ioctl_arg_type; // 
typedef uint32_t u_long_type; //
typedef uint16_t u_short_type; // 
#if defined(ASIO_HAS_SSIZE_T)
typedef ssize_t signed_size_type; //
#else // defined(ASIO_HAS_SSIZE_T)
typedef int signed_size_type; //
#endif // defined(ASIO_HAS_SSIZE_T)
# define ASIO_OS_DEF(c) ASIO_OS_DEF_##c	//宏定义参数
# define ASIO_OS_DEF_AF_UNSPEC AF_UNSPEC	
# define ASIO_OS_DEF_AF_INET AF_INET
# define ASIO_OS_DEF_AF_INET6 AF_INET6	//地址协议
# define ASIO_OS_DEF_SOCK_STREAM SOCK_STREAM
# define ASIO_OS_DEF_SOCK_DGRAM SOCK_DGRAM
# define ASIO_OS_DEF_SOCK_RAW SOCK_RAW
# define ASIO_OS_DEF_SOCK_SEQPACKET SOCK_SEQPACKET //socket协议
# define ASIO_OS_DEF_IPPROTO_IP IPPROTO_IP
# define ASIO_OS_DEF_IPPROTO_IPV6 IPPROTO_IPV6
# define ASIO_OS_DEF_IPPROTO_TCP IPPROTO_TCP
# define ASIO_OS_DEF_IPPROTO_UDP IPPROTO_UDP
# define ASIO_OS_DEF_IPPROTO_ICMP IPPROTO_ICMP
# define ASIO_OS_DEF_IPPROTO_ICMPV6 IPPROTO_ICMPV6 //采用哪一种协议
# define ASIO_OS_DEF_FIONBIO FIONBIO
# define ASIO_OS_DEF_FIONREAD FIONREAD
# define ASIO_OS_DEF_INADDR_ANY INADDR_ANY
# define ASIO_OS_DEF_MSG_OOB MSG_OOB
# define ASIO_OS_DEF_MSG_PEEK MSG_PEEK
# define ASIO_OS_DEF_MSG_DONTROUTE MSG_DONTROUTE
# define ASIO_OS_DEF_MSG_EOR MSG_EOR
# define ASIO_OS_DEF_SHUT_RD SHUT_RD	// int shutdown(int sock, int howto);  【Linux】 断开输入流
# define ASIO_OS_DEF_SHUT_WR SHUT_WR	// int shutdown(int sock, int howto);  【Linux】 断开输出流
# define ASIO_OS_DEF_SHUT_RDWR SHUT_RDWR	// int shutdown(int sock, int howto);  【Linux】 同时断开 I/O 流
# define ASIO_OS_DEF_SOMAXCONN SOMAXCONN
# define ASIO_OS_DEF_SOL_SOCKET SOL_SOCKET
# define ASIO_OS_DEF_SO_BROADCAST SO_BROADCAST
# define ASIO_OS_DEF_SO_DEBUG SO_DEBUG
# define ASIO_OS_DEF_SO_DONTROUTE SO_DONTROUTE
# define ASIO_OS_DEF_SO_KEEPALIVE SO_KEEPALIVE
# define ASIO_OS_DEF_SO_LINGER SO_LINGER
# define ASIO_OS_DEF_SO_OOBINLINE SO_OOBINLINE
# define ASIO_OS_DEF_SO_SNDBUF SO_SNDBUF
# define ASIO_OS_DEF_SO_RCVBUF SO_RCVBUF
# define ASIO_OS_DEF_SO_SNDLOWAT SO_SNDLOWAT
# define ASIO_OS_DEF_SO_RCVLOWAT SO_RCVLOWAT
# define ASIO_OS_DEF_SO_REUSEADDR SO_REUSEADDR
# define ASIO_OS_DEF_TCP_NODELAY TCP_NODELAY
# define ASIO_OS_DEF_IP_MULTICAST_IF IP_MULTICAST_IF
# define ASIO_OS_DEF_IP_MULTICAST_TTL IP_MULTICAST_TTL
# define ASIO_OS_DEF_IP_MULTICAST_LOOP IP_MULTICAST_LOOP
# define ASIO_OS_DEF_IP_ADD_MEMBERSHIP IP_ADD_MEMBERSHIP
# define ASIO_OS_DEF_IP_DROP_MEMBERSHIP IP_DROP_MEMBERSHIP
# define ASIO_OS_DEF_IP_TTL IP_TTL
# define ASIO_OS_DEF_IPV6_UNICAST_HOPS IPV6_UNICAST_HOPS
# define ASIO_OS_DEF_IPV6_MULTICAST_IF IPV6_MULTICAST_IF
# define ASIO_OS_DEF_IPV6_MULTICAST_HOPS IPV6_MULTICAST_HOPS
# define ASIO_OS_DEF_IPV6_MULTICAST_LOOP IPV6_MULTICAST_LOOP
# define ASIO_OS_DEF_IPV6_JOIN_GROUP IPV6_JOIN_GROUP
# define ASIO_OS_DEF_IPV6_LEAVE_GROUP IPV6_LEAVE_GROUP
# define ASIO_OS_DEF_AI_CANONNAME AI_CANONNAME
# define ASIO_OS_DEF_AI_PASSIVE AI_PASSIVE
# define ASIO_OS_DEF_AI_NUMERICHOST AI_NUMERICHOST
# if defined(AI_NUMERICSERV)
#  define ASIO_OS_DEF_AI_NUMERICSERV AI_NUMERICSERV
# else
#  define ASIO_OS_DEF_AI_NUMERICSERV 0
# endif
// Note: QNX Neutrino 6.3 defines AI_V4MAPPED, AI_ALL and AI_ADDRCONFIG but
// does not implement them. Therefore they are specifically excluded here.
# if defined(AI_V4MAPPED) && !defined(__QNXNTO__)
#  define ASIO_OS_DEF_AI_V4MAPPED AI_V4MAPPED
# else
#  define ASIO_OS_DEF_AI_V4MAPPED 0
# endif
# if defined(AI_ALL) && !defined(__QNXNTO__)
#  define ASIO_OS_DEF_AI_ALL AI_ALL
# else
#  define ASIO_OS_DEF_AI_ALL 0
# endif
# if defined(AI_ADDRCONFIG) && !defined(__QNXNTO__)
#  define ASIO_OS_DEF_AI_ADDRCONFIG AI_ADDRCONFIG
# else
#  define ASIO_OS_DEF_AI_ADDRCONFIG 0
# endif
# if defined(IOV_MAX)
const int max_iov_len = IOV_MAX;
# else
// POSIX platforms are not required to define IOV_MAX.
const int max_iov_len = 16;
# endif
#endif
const int custom_socket_option_level = 0xA5100000;
const int enable_connection_aborted_option = 1;
const int always_fail_option = 2;

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_SOCKET_TYPES_HPP
