//
// detail/impl/service_registry.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IMPL_SERVICE_REGISTRY_IPP
#define ASIO_DETAIL_IMPL_SERVICE_REGISTRY_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <vector>
#include "asio/detail/service_registry.hpp"
#include "asio/detail/throw_exception.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

service_registry::service_registry(execution_context& owner)    //初始化时对服务注册器【父执行上下文，而第一个服务指针置为nullptr】
  : owner_(owner),
    first_service_(0)
{
}

service_registry::~service_registry()
{
}

void service_registry::shutdown_services()
{
  execution_context::service* service = first_service_; //拿一个指针指向first_service
  while (service)
  {
    service->shutdown();    //根据链表，逐次关闭
    service = service->next_;
  }
}

void service_registry::destroy_services()
{
  while (first_service_)
  {
    execution_context::service* next_service = first_service_->next_;   
    destroy(first_service_);    //摧毁服务时也是一样，按照链表次序逐次摧毁
    first_service_ = next_service;
  }
}

void service_registry::notify_fork(execution_context::fork_event fork_ev)
{
  // Make a copy of all of the services while holding the lock. We don't want
  // to hold the lock while calling into each service, as it may try to call
  // back into this class.
  std::vector<execution_context::service*> services;    //新建一个向量
  {
    asio::detail::mutex::scoped_lock lock(mutex_); // 【拿到service_registry的互斥锁】
    execution_context::service* service = first_service_; 
    while (service)
    {
      services.push_back(service);
      service = service->next_;
    }   
  } //使用{}来进行scope限定【主要是那个scoped_lock的应用，出去自动解锁】

  // If processing the fork_prepare event, we want to go in reverse order of
  // service registration, which happens to be the existing order of the
  // services in the vector. For the other events we want to go in the other
  // direction.
  std::size_t num_services = services.size(); // 拿到vector的size
  if (fork_ev == execution_context::fork_prepare)   //【根据fork_prepare判断该顺序还是逆序进行服务注册】
    for (std::size_t i = 0; i < num_services; ++i)
      services[i]->notify_fork(fork_ev);
  else
    for (std::size_t i = num_services; i > 0; --i)
      services[i - 1]->notify_fork(fork_ev);
}

void service_registry::init_key_from_id(execution_context::service::key& key,
    const execution_context::id& id)    //根据execution_context::service::key这个结构体进行初始化
{
  key.type_info_ = 0;      //我们知道里面std::type_info 保有一个类型的实现指定信息，包括类型的名称和比较二个类型相等的方法或相对顺序。【uuid】
  key.id_ = &id;    //而id类其实是一个空类 【(我觉得其唯一的作用就是拿到那个地址)】
}

bool service_registry::keys_match(
    const execution_context::service::key& key1,
    const execution_context::service::key& key2)
{
  if (key1.id_ && key2.id_)
    if (key1.id_ == key2.id_)
      return true;  // 要是两者id_都不为null且相等时 返回真 【还不清楚id_这个的用法】
  if (key1.type_info_ && key2.type_info_)
    if (*key1.type_info_ == *key2.type_info_)
      return true;  // 要是两者type_info_都不为null且相等 返回真
  return false;
}

void service_registry::destroy(execution_context::service* service)
{
  delete service; //释放传入的service
}

execution_context::service* service_registry::do_use_service(
    const execution_context::service::key& key,
    factory_type factory, void* owner)
{
  asio::detail::mutex::scoped_lock lock(mutex_);    //先对注册器加锁

  // First see if there is an existing service object with the given key.
  execution_context::service* service = first_service_;
  while (service)   //链表顺序查找，找到了就返回
  {
    if (keys_match(service->key_, key))
      return service;
    service = service->next_;
  }

  // Create a new service object. The service registry's mutex is not locked
  // at this time to allow for nested calls into this function from the new
  // service's constructor.
  lock.unlock();    //查找完毕，没找到，先解锁
  auto_service_ptr new_service = { factory(owner) }; //【这里就使用调用的工厂类进行创建[factory_type factory]，注意我们execution_context::service中我们将service_registry设成了友元类】
  new_service.ptr_->key_ = key; //同时设定对应的key
  lock.lock(); //重新上锁【防止别人也创建了相同类型的服务】
  
  // Check that nobody else created another service object of the same type
  // while the lock was released.
  service = first_service_;
  while (service)
  {
    if (keys_match(service->key_, key))
      return service;
    service = service->next_;
  }

  // Service was successfully initialised, pass ownership to registry.
  new_service.ptr_->next_ = first_service_; //最新创建的增加到链表开头
  first_service_ = new_service.ptr_;
  new_service.ptr_ = 0; //将指针设为0，以防出了scope就自动销毁了
  return first_service_;
}

void service_registry::do_add_service(
    const execution_context::service::key& key,
    execution_context::service* new_service)    //注意我们的key最后这个地址扔给了服务的key_
{
  if (&owner_ != &new_service->context())
    asio::detail::throw_exception(invalid_service_owner()); //保证添加的服务的执行上下文应该[与service_registry]相同

  asio::detail::mutex::scoped_lock lock(mutex_);    //lock_guard

  // Check if there is an existing service object with the given key.   【加锁查询服务是否存在】
  execution_context::service* service = first_service_;
  while (service)
  {
    if (keys_match(service->key_, key))
      asio::detail::throw_exception(service_already_exists());  //已经存在就抛异常
    service = service->next_;
  }

  // Take ownership of the service object.
  new_service->key_ = key;
  new_service->next_ = first_service_;  //新添加的服务放到链表开头
  first_service_ = new_service;
}

bool service_registry::do_has_service(
    const execution_context::service::key& key) const
{
  asio::detail::mutex::scoped_lock lock(mutex_); //加锁

  execution_context::service* service = first_service_;
  while (service)
  {
    if (keys_match(service->key_, key))
      return true;      //查询有没有这个服务，有就为真
    service = service->next_;
  }

  return false;
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_IMPL_SERVICE_REGISTRY_IPP
