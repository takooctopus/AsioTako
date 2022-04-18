//
// TAKO������ע��[����]��һЩ��������
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

// ����ļ���ģ����Service����Service_registry��service��Ҳ��execution_context::service����Ȼ���ܲ���������ࡾ���ǿ��ļ���֪��service��׺���ļ���ĺܰ���

template <typename Service> //������ķ������ͷŵ�<>�С�
Service& service_registry::use_service()
{
  execution_context::service::key key;  //�����key
  init_key<Service>(key, 0);    //���ǳ�ʼ��key
  factory_type factory = &service_registry::create<Service, execution_context>; //factory_type�������ǵķ�������Ӧ��execution::service��protected���캯��
  return *static_cast<Service*>(do_use_service(key, factory, &owner_)); //��󷵻ص���һ��service*
}

template <typename Service>
Service& service_registry::use_service(io_context& owner)   
{
  execution_context::service::key key;
  init_key<Service>(key, 0);    //��ʼ��
  factory_type factory = &service_registry::create<Service, io_context>;    //���ǽ��������ĸ������� ����io_context����Ϊģ��ʹ�õ�io_context��������ʵ�����ġ�
  return *static_cast<Service*>(do_use_service(key, factory, &owner)); //Ҫ��û�������ǻ��ô���һ��service���ŵ����ǵ������׵�ַ�ϣ��ҵ��˾�ֱ�ӷ��ص�ַ
}

template <typename Service>
void service_registry::add_service(Service* new_service)
{
  execution_context::service::key key;  //�½�һ��key����
  init_key<Service>(key, 0);    // ��ӷ�������Ҳ��֪�����ڵ�id_������Ĭ��Ϊnullptr[0]
  return do_add_service(key, new_service); //�����keyҪ����ֵ��new_service.key_��
}

template <typename Service>
bool service_registry::has_service() const
{
  execution_context::service::key key;  //��ʼ��ִ����������������key������һ��key����ɣ�һ��type_info_��һ��id_��
  init_key<Service>(key, 0);    //����type_info_���óɶ�Ӧ��typeid_wrapperģ���ػ���uuid���жϵĻ�ֱ�ӽ�id_���ó�nullptr
  return do_has_service(key);   
}

template <typename Service>
inline void service_registry::init_key(
    execution_context::service::key& key, ...)
{
  init_key_from_id(key, Service::id);   //���ǿ��Կ������Service::id ������
}

#if !defined(ASIO_NO_TYPEID)
template <typename Service>
void service_registry::init_key(execution_context::service::key& key,
    typename enable_if<
      is_base_of<typename Service::key_type, Service>::value>::type*)
{
  key.type_info_ = &typeid(typeid_wrapper<Service>);    //ʹ���������ʶ���ģ���ػ���Ψһ��uuid
  key.id_ = 0;  //һ������ʼ��Ϊnullptr
}

template <typename Service>
void service_registry::init_key_from_id(execution_context::service::key& key,
    const service_id<Service>& /*id*/)
{
  key.type_info_ = &typeid(typeid_wrapper<Service>); //ʹ���������ʶ���ģ���ػ���Ψһ��uuid
  key.id_ = 0;  // ��ʼ��key.id_ => nullptr
}
#endif // !defined(ASIO_NO_TYPEID)

template <typename Service, typename Owner> // Service�Ƿ��� Owner��ʲô=>execution_contex
execution_context::service* service_registry::create(void* owner)
{
  return new Service(*static_cast<Owner*>(owner));  //��ʵ����һ������
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_IMPL_SERVICE_REGISTRY_HPP
