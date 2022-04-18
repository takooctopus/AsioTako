//
// TAKO: 服务注册器
// detail/service_registry.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_SERVICE_REGISTRY_HPP
#define ASIO_DETAIL_SERVICE_REGISTRY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <typeinfo>
#include "asio/detail/mutex.hpp"    //互斥锁【下面根据平台实现不同实现】
#include "asio/detail/noncopyable.hpp"  //不可拷贝
#include "asio/detail/type_traits.hpp"  //类型萃取
#include "asio/execution_context.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

class io_context;

namespace detail {

template <typename T>
class typeid_wrapper {};    //为什么要使用一个空模板类？【因为编译器对这个模板类会生成很多实现，我们最后是要取&typeid(typeid_wrapper<Service>)，我们可以预见的是，对于不同的Service来说，其typeid返回的相当于uuid是不一样的，那么这个用于比较 execution_context::service.key.type_info_时就具有类的唯一性，可以使用这个去判断是不是同一个类[讲道理，要是c++直接有guid或者uuid相关的方法就方便的多了]】

class service_registry
  : private noncopyable
{
public:
  // Constructor. 【构造函数】
  ASIO_DECL service_registry(execution_context& owner);

  // Destructor.  【析构函数】
  ASIO_DECL ~service_registry();

  // Shutdown all services. 【关闭所有服务】
  ASIO_DECL void shutdown_services();

  // Destroy all services. 【销毁所有服务】
  ASIO_DECL void destroy_services();

  // Notify all services of a fork event. 【向所有服务通知一个进程复制事件】
  ASIO_DECL void notify_fork(execution_context::fork_event fork_ev);

  // Get the service object corresponding to the specified service type. Will
  // create a new service object automatically if no such object already
  // exists. Ownership of the service object is not transferred to the caller.
  template <typename Service>  //【模板，传入的服务放到<>中】
  Service& use_service();

  // Get the service object corresponding to the specified service type. Will
  // create a new service object automatically if no such object already
  // exists. Ownership of the service object is not transferred to the caller.
  // This overload is used for backwards compatibility with services that
  // inherit from io_context::service.
  template <typename Service>
  Service& use_service(io_context& owner);

  // Add a service object. Throws on error, in which case ownership of the
  // object is retained by the caller.
  template <typename Service>
  void add_service(Service* new_service);

  // Check whether a service object of the specified type already exists.
  template <typename Service>
  bool has_service() const;

private:
  // Initalise a service's key when the key_type typedef is not available. 【初始化一个服务的key[当key类型不可用时]】
  template <typename Service>
  static void init_key(execution_context::service::key& key, ...);

#if !defined(ASIO_NO_TYPEID)
  // Initalise a service's key when the key_type typedef is available. 【对于key_type是Service基类的特化】
  template <typename Service>
  static void init_key(execution_context::service::key& key,
      typename enable_if<
        is_base_of<typename Service::key_type, Service>::value>::type*);
#endif // !defined(ASIO_NO_TYPEID)

  // Initialise a service's key based on its id. 【根据执行上下文服务的key和执行上下文自己的id来进行服务的初始化】
  ASIO_DECL static void init_key_from_id(
      execution_context::service::key& key,
      const execution_context::id& id);

#if !defined(ASIO_NO_TYPEID)
  // Initialise a service's key based on its id.
  template <typename Service>
  static void init_key_from_id(execution_context::service::key& key,
      const service_id<Service>& /*id*/);
#endif // !defined(ASIO_NO_TYPEID)

  // Check if a service matches the given id. 【判断两个上下文的key是不是相等】
  ASIO_DECL static bool keys_match(
      const execution_context::service::key& key1,
      const execution_context::service::key& key2);

  // The type of a factory function used for creating a service instance.【定义的函数指针(execution_context::service) => 】
  typedef execution_context::service*(*factory_type)(void*); //【！我们这里能用factory是因为我们将service_registry设置成了service的友元类】

  // Factory function for creating a service instance.  【工厂类方法模板，仅限于这个类中使用create()就直接创建了】
  template <typename Service, typename Owner>
  static execution_context::service* create(void* owner);

  // Destroy a service instance. 【释放传入的对应service内存】
  ASIO_DECL static void destroy(execution_context::service* service);

  // Helper class to manage service pointers.
  struct auto_service_ptr;
  friend struct auto_service_ptr;
  struct auto_service_ptr   //帮助类，使用一个service*进行构建，在退出时自动销毁内存
  {
    execution_context::service* ptr_;
    ~auto_service_ptr() { destroy(ptr_); }
  };

  // Get the service object corresponding to the specified service key. Will
  // create a new service object automatically if no such object already
  // exists. Ownership of the service object is not transferred to the caller.
  ASIO_DECL execution_context::service* do_use_service(
      const execution_context::service::key& key,
      factory_type factory, void* owner);

  // Add a service object. Throws on error, in which case ownership of the
  // object is retained by the caller.
  // 【增加一个服务到注册器】
  ASIO_DECL void do_add_service(
      const execution_context::service::key& key,
      execution_context::service* new_service);

  // Check whether a service object with the specified key already exists.
  // 【查询有没有这个服务】
  ASIO_DECL bool do_has_service(
      const execution_context::service::key& key) const;

  // Mutex to protect access to internal data. 【一个mutex】
  mutable asio::detail::mutex mutex_;

  // The owner of this service registry and the services it contains. 【service registry 的拥有者引用[执行上下文]】
  execution_context& owner_;

  // The first service in the list of contained services. 【执行上下文指针[中的public类 service]】
  execution_context::service* first_service_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#include "asio/detail/impl/service_registry.hpp"
#if defined(ASIO_HEADER_ONLY)
# include "asio/detail/impl/service_registry.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_DETAIL_SERVICE_REGISTRY_HPP
