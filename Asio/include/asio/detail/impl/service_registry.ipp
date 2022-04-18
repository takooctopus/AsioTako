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

service_registry::service_registry(execution_context& owner)    //��ʼ��ʱ�Է���ע��������ִ�������ģ�����һ������ָ����Ϊnullptr��
  : owner_(owner),
    first_service_(0)
{
}

service_registry::~service_registry()
{
}

void service_registry::shutdown_services()
{
  execution_context::service* service = first_service_; //��һ��ָ��ָ��first_service
  while (service)
  {
    service->shutdown();    //����������ιر�
    service = service->next_;
  }
}

void service_registry::destroy_services()
{
  while (first_service_)
  {
    execution_context::service* next_service = first_service_->next_;   
    destroy(first_service_);    //�ݻٷ���ʱҲ��һ�����������������δݻ�
    first_service_ = next_service;
  }
}

void service_registry::notify_fork(execution_context::fork_event fork_ev)
{
  // Make a copy of all of the services while holding the lock. We don't want
  // to hold the lock while calling into each service, as it may try to call
  // back into this class.
  std::vector<execution_context::service*> services;    //�½�һ������
  {
    asio::detail::mutex::scoped_lock lock(mutex_); // ���õ�service_registry�Ļ�������
    execution_context::service* service = first_service_; 
    while (service)
    {
      services.push_back(service);
      service = service->next_;
    }   
  } //ʹ��{}������scope�޶�����Ҫ���Ǹ�scoped_lock��Ӧ�ã���ȥ�Զ�������

  // If processing the fork_prepare event, we want to go in reverse order of
  // service registration, which happens to be the existing order of the
  // services in the vector. For the other events we want to go in the other
  // direction.
  std::size_t num_services = services.size(); // �õ�vector��size
  if (fork_ev == execution_context::fork_prepare)   //������fork_prepare�жϸ�˳����������з���ע�᡿
    for (std::size_t i = 0; i < num_services; ++i)
      services[i]->notify_fork(fork_ev);
  else
    for (std::size_t i = num_services; i > 0; --i)
      services[i - 1]->notify_fork(fork_ev);
}

void service_registry::init_key_from_id(execution_context::service::key& key,
    const execution_context::id& id)    //����execution_context::service::key����ṹ����г�ʼ��
{
  key.type_info_ = 0;      //����֪������std::type_info ����һ�����͵�ʵ��ָ����Ϣ���������͵����ƺͱȽ϶���������ȵķ��������˳�򡣡�uuid��
  key.id_ = &id;    //��id����ʵ��һ������ ��(�Ҿ�����Ψһ�����þ����õ��Ǹ���ַ)��
}

bool service_registry::keys_match(
    const execution_context::service::key& key1,
    const execution_context::service::key& key2)
{
  if (key1.id_ && key2.id_)
    if (key1.id_ == key2.id_)
      return true;  // Ҫ������id_����Ϊnull�����ʱ ������ ���������id_������÷���
  if (key1.type_info_ && key2.type_info_)
    if (*key1.type_info_ == *key2.type_info_)
      return true;  // Ҫ������type_info_����Ϊnull����� ������
  return false;
}

void service_registry::destroy(execution_context::service* service)
{
  delete service; //�ͷŴ����service
}

execution_context::service* service_registry::do_use_service(
    const execution_context::service::key& key,
    factory_type factory, void* owner)
{
  asio::detail::mutex::scoped_lock lock(mutex_);    //�ȶ�ע��������

  // First see if there is an existing service object with the given key.
  execution_context::service* service = first_service_;
  while (service)   //����˳����ң��ҵ��˾ͷ���
  {
    if (keys_match(service->key_, key))
      return service;
    service = service->next_;
  }

  // Create a new service object. The service registry's mutex is not locked
  // at this time to allow for nested calls into this function from the new
  // service's constructor.
  lock.unlock();    //������ϣ�û�ҵ����Ƚ���
  auto_service_ptr new_service = { factory(owner) }; //�������ʹ�õ��õĹ�������д���[factory_type factory]��ע������execution_context::service�����ǽ�service_registry�������Ԫ�ࡿ
  new_service.ptr_->key_ = key; //ͬʱ�趨��Ӧ��key
  lock.lock(); //������������ֹ����Ҳ��������ͬ���͵ķ���
  
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
  new_service.ptr_->next_ = first_service_; //���´��������ӵ�����ͷ
  first_service_ = new_service.ptr_;
  new_service.ptr_ = 0; //��ָ����Ϊ0���Է�����scope���Զ�������
  return first_service_;
}

void service_registry::do_add_service(
    const execution_context::service::key& key,
    execution_context::service* new_service)    //ע�����ǵ�key��������ַ�Ӹ��˷����key_
{
  if (&owner_ != &new_service->context())
    asio::detail::throw_exception(invalid_service_owner()); //��֤��ӵķ����ִ��������Ӧ��[��service_registry]��ͬ

  asio::detail::mutex::scoped_lock lock(mutex_);    //lock_guard

  // Check if there is an existing service object with the given key.   ��������ѯ�����Ƿ���ڡ�
  execution_context::service* service = first_service_;
  while (service)
  {
    if (keys_match(service->key_, key))
      asio::detail::throw_exception(service_already_exists());  //�Ѿ����ھ����쳣
    service = service->next_;
  }

  // Take ownership of the service object.
  new_service->key_ = key;
  new_service->next_ = first_service_;  //����ӵķ���ŵ�����ͷ
  first_service_ = new_service;
}

bool service_registry::do_has_service(
    const execution_context::service::key& key) const
{
  asio::detail::mutex::scoped_lock lock(mutex_); //����

  execution_context::service* service = first_service_;
  while (service)
  {
    if (keys_match(service->key_, key))
      return true;      //��ѯ��û����������о�Ϊ��
    service = service->next_;
  }

  return false;
}

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_IMPL_SERVICE_REGISTRY_IPP
