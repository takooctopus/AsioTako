//
// TAKO：执行上下文相关的函数【这个相比另外的应该算更上层一些】
// impl/execution_context.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IMPL_EXECUTION_CONTEXT_HPP
#define ASIO_IMPL_EXECUTION_CONTEXT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/handler_type_requirements.hpp"
#include "asio/detail/scoped_ptr.hpp"   //管理生命周期的指针
#include "asio/detail/service_registry.hpp" //服务注册链表类

#include "asio/detail/push_options.hpp"

namespace asio {

#if !defined(GENERATING_DOCUMENTATION)

template <typename Service> //模板函数，要调用的服务类型名放到<>中
inline Service& use_service(execution_context& e)
{
  // Check that Service meets the necessary type requirements.
  (void)static_cast<execution_context::service*>(static_cast<Service*>(0)); //使用static_cast保证【nullptr => Service* => execution_context::service*】

  return e.service_registry_->template use_service<Service>();  //调用的下面的模板函数【具体的嘛我们直到use_service】
}

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename Service, typename... Args>   //模板函数，要调用的服务类型名放到<>中，这个是多参
Service& make_service(execution_context& e, ASIO_MOVE_ARG(Args)... args)
{
  detail::scoped_ptr<Service> svc(
      new Service(e, ASIO_MOVE_CAST(Args)(args)...));   //新建一个服务
  e.service_registry_->template add_service<Service>(svc.get());    //添加这个服务到执行上下文e中的注册链表中【失败会抛异常】
  Service& result = *svc;   //获取这个服务的所有权
  svc.release(); //释放所有权
  return result;    //返回加入的那个service
}

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename Service>
Service& make_service(execution_context& e)
{
  detail::scoped_ptr<Service> svc(new Service(e));
  e.service_registry_->template add_service<Service>(svc.get());
  Service& result = *svc;
  svc.release();
  return result;
}

#define ASIO_PRIVATE_MAKE_SERVICE_DEF(n) \
  template <typename Service, ASIO_VARIADIC_TPARAMS(n)> \
  Service& make_service(execution_context& e, \
      ASIO_VARIADIC_MOVE_PARAMS(n)) \
  { \
    detail::scoped_ptr<Service> svc( \
        new Service(e, ASIO_VARIADIC_MOVE_ARGS(n))); \
    e.service_registry_->template add_service<Service>(svc.get()); \
    Service& result = *svc; \
    svc.release(); \
    return result; \
  } \
  /**/
  ASIO_VARIADIC_GENERATE(ASIO_PRIVATE_MAKE_SERVICE_DEF)
#undef ASIO_PRIVATE_MAKE_SERVICE_DEF

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)

template <typename Service>
inline void add_service(execution_context& e, Service* svc) //添加服务【服务已生成】
{
  // Check that Service meets the necessary type requirements.
  (void)static_cast<execution_context::service*>(static_cast<Service*>(0)); //先保证Service*能转化成基类的指针

  e.service_registry_->template add_service<Service>(svc);  //直接添加
}

template <typename Service>
inline bool has_service(execution_context& e)   //判断服务存不存在
{
  // Check that Service meets the necessary type requirements.
  (void)static_cast<execution_context::service*>(static_cast<Service*>(0)); //先保证Service*能转化成基类的指针

  return e.service_registry_->template has_service<Service>();  //去判断
}

#endif // !defined(GENERATING_DOCUMENTATION)

inline execution_context& execution_context::service::context() //返回其所属的上下文
{
  return owner_;    
}

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_EXECUTION_CONTEXT_HPP
