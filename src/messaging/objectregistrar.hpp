#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_OBJECTREGISTRAR_HPP_
#define _SRC_OBJECTREGISTRAR_HPP_

#include <string>
#include <set>
#include <boost/thread/recursive_mutex.hpp>
#include <qi/api.hpp>
#include <qi/session.hpp>
#include <qi/atomic.hpp>
#include "server.hpp"

namespace qi {

  class GenericObject;
  class ServiceDirectoryClient;

  //hold all information about a service
  struct BoundService {
    ~BoundService();

    std::string   name;
    qi::AnyObject object;
    unsigned int  id;
    ServiceInfo   serviceInfo;
  };


  /** the Server 0 object
   * handle metaObject request, event connection/disconnection
   */
  class ObjectRegistrar : private Server {

  public:
    ObjectRegistrar(ServiceDirectoryClient *sdClient, bool enforceAuth = false);
    virtual ~ObjectRegistrar();

    void close();
    // Get ready to accept new objects/sockets after a close
    void open();

    //register/unregister services
    qi::Future<unsigned int>     registerService(const std::string &name, qi::AnyObject obj);
    qi::Future<void>             unregisterService(unsigned int idx);
    void                         updateServiceInfo();

    //list services
    std::vector<qi::ServiceInfo>  registeredServices();
    qi::ServiceInfo               registeredService(const std::string &service);
    qi::AnyObject                 registeredServiceObject(const std::string &service);

    // Add an existing running socket to the list
    void registerSocket(MessageSocketPtr socket);

    using Server::setAuthProviderFactory;
    using Server::listen;
    using Server::setIdentity;
    using Server::endpoints;

  private:
    //0 on error
    unsigned int   objectId(const std::string &name);

  private:
    //Future
    void onFutureFinished(qi::Future<unsigned int> future, int id, qi::Promise<unsigned int> result);

  private:
    using BoundServiceMap = std::map<unsigned int, BoundService>;
    using RegisterServiceMap = std::map<int, std::pair<qi::AnyObject, qi::ServiceInfo>>;
    using ServiceNameToIndexMap = std::map<std::string, unsigned int>;

  public:
    BoundServiceMap                     _services;
    boost::mutex                        _servicesMutex;

    ServiceNameToIndexMap               _serviceNameToIndex;
    boost::mutex                        _serviceNameToIndexMutex;


    //used by registerService
    RegisterServiceMap                  _registerServiceRequest;
    qi::Atomic<int>                    _registerServiceRequestIndex;
    boost::mutex                        _registerServiceRequestMutex;

    ServiceDirectoryClient             *_sdClient;
    const std::string                   _id;

    friend class Session_SD;
  };

}

QI_TYPE_NOT_CONSTRUCTIBLE(qi::ObjectRegistrar);


#endif  // _SRC_OBJECTREGISTRAR_HPP_
