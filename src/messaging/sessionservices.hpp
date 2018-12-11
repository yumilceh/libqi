#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SESSIONSERVICES_HPP_
#define _SRC_SESSIONSERVICES_HPP_

#include <qi/future.hpp>
#include <vector>
#include <boost/thread/mutex.hpp>
#include <qi/session.hpp>
#include <qi/atomic.hpp>

namespace qi {

  struct ServicesRequest {
  public:
    ServicesRequest()
    {}

    ServicesRequest(qi::Promise< std::vector<qi::ServiceInfo> > promise, Session::ServiceLocality locality)
      : promise(promise)
      , locality(locality)
    {}

    qi::Promise< std::vector<qi::ServiceInfo> > promise;
    Session::ServiceLocality                    locality;
  };

  class ServiceDirectoryClient;
  class ObjectRegistrar;
  class Session_Services {
  public:
    Session_Services(ServiceDirectoryClient *sdClient, ObjectRegistrar *server)
      : _sdClient(sdClient)
      , _server(server)
    {}

    qi::Future< std::vector<qi::ServiceInfo> > services(Session::ServiceLocality locality);

  protected:
    //FutureInterface
    void onFutureFinished(qi::Future<qi::ServiceInfoVector> value, long requestId);

  protected:
    ServicesRequest *request(long requestId);
    void             removeRequest(long requestId);

  protected:
    using ServicesRequestPtrMap = std::map<int, ServicesRequest*>;

    std::map<int, ServicesRequest*>  _request;
    boost::mutex                      _requestMutex;
    qi::Atomic<int>                  _requestIndex;

    ServiceDirectoryClient *_sdClient;
    ObjectRegistrar        *_server;
  };

}

#endif  // _SRC_SESSIONSERVICES_HPP_
