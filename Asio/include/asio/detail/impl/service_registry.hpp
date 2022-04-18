//
// TAKO：服务注册[链表]的一些操作函数
// detail/impl/service_registry.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IMPL_SERVICE_REGISTRY_HPP
#define ASIO_DETAIL_IMPL_SERVICE_REGISTRY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/push_options.hpp"

namespace asio {
namespace detail {

// 这个文件的模板类Service就是Service_registry的service，也是execution_context::service，当然可能并非这个基类【我们看文件就知道service后缀的文件多的很啊】

template <typename Service> //【传入的服务类型放到<>中】
Service& service_registry::use_service()
{
  execution_context::service::key key;  //服务的key
  init_key<Service>(key, 0);    //我们初始化key
  factory_type factory = &service_registry::create<Service, execution_context>; //factory_type就是我们的服务所对应的execution::service的protected构造函数
  return *static_cast<Service*>(do_use_service(key, factory, &owner_)); //最后返回的是一个service*
}

template <typename Service>
Service& service_registry::use_service(io_context& owner)   
{
  execution_context::service::key key;
  init_key<Service>(key, 0);    //初始化
  factory_type factory = &service_registry::create<Service, io_context>;    //我们将这个服务的父上下文 换成io_context【因为模板使用的io_context】【是新实例化的】
  return *static_cast<Service*>(do_use_service(key, factory, &owner)); //要是没服务我们还得创建一个service，放到我们的链表首地址上，找到了就直接返回地址
}

template <typename Service>
void service_registry::add_service(Service* new_service)
{
  execution_context::service::key key;  //新建一个key对象
  init_key<Service>(key, 0);    // 添加服务我们也不知道现在的id_，所以默认为nullptr[0]
  return do_add_service(key, new_service); //【最后key要被赋值给new_service.key_】
}

template <typename Service>
bool service_registry::has_service() const
{
  execution_context::service::key key;  //初始化执行上下文下面服务的key【回想一下key的组成，一个type_info_，一个id_】
  init_key<Service>(key, 0);    //我们type_info_设置成对应的typeid_wrapper模板特化的uuid，判断的话直接将id_设置成nullptr
  return do_has_service(key);   
}

template <typename Service>
inline void service_registry::init_key(
    execution_context::service::key& key, ...)
{
  init_key_from_id(key, Service::id);   //我们可以看到这个Service::id ？？？
}

#if !defined(ASIO_NO_TYPEID)
template <typename Service>
void service_registry::init_key(execution_context::service::key& key,
    typename enable_if<
      is_base_of<typename Service::key_type, Service>::value>::type*)
{
  key.type_info_ = &typeid(typeid_wrapper<Service>);    //使用这个来标识这个模板特化类唯一的uuid
  key.id_ = 0;  //一样，初始化为nullptr
}

template <typename Service>
void service_registry::init_key_from_id(execution_context::service::key& key,
    const service_id<Service>& /*id*/)
{
  key.type_info_ = &typeid(typeid_wrapper<Service>); //使用这个来标识这个模板特化类唯一的uuid
  key.id_ = 0;  // 初始化key.id_ => nullptr
}
#endif // !defined(ASIO_NO_TYPEID)

template <typename Service, typename Owner> // Service是服务 Owner是什么=>execution_contex
execution_context::service* service_registry::create(void* owner)
{
  return new Service(*static_cast<Owner*>(owner));  //新实例化一个服务
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_IMPL_SERVICE_REGISTRY_HPP
