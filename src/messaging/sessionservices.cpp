/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "sessionservices.hpp"
#include "servicedirectoryclient.hpp"
#include "objectregistrar.hpp"

namespace qi {


  void Session_Services::onFutureFinished(qi::Future<ServiceInfoVector> value, long requestId) {
    ServicesRequest *rq = request(requestId);
    if (!rq)
      return;
    if (value.hasError()) {
      rq->promise.setError(value.error());
      removeRequest(requestId);
      return;
    }
    std::vector<qi::ServiceInfo> result;
    if (rq->locality == qi::Session::ServiceLocality_All)
    {
      result = _server->registeredServices();
    }
    result.insert(result.end(), value.value().begin(), value.value().end());
    rq->promise.setValue(result);
    removeRequest(requestId);
  }

  ServicesRequest *Session_Services::request(long requestId) {
    {
      boost::mutex::scoped_lock sl(_requestMutex);
      std::map<int, ServicesRequest*>::iterator it;
      it = _request.find(requestId);
      if (it != _request.end())
        return it->second;
    }
    return 0;
  }

  void Session_Services::removeRequest(long requestId) {
    {
      boost::mutex::scoped_lock sl(_requestMutex);
      std::map<int, ServicesRequest*>::iterator it;
      it = _request.find(requestId);
      if (it != _request.end()) {
        delete it->second;
        _request.erase(it);
      }
    }
  }

  qi::Future< std::vector<qi::ServiceInfo> > Session_Services::services(Session::ServiceLocality locality)
  {
    if (locality == Session::ServiceLocality_Local) {
      qi::Promise<std::vector<qi::ServiceInfo> > promise;
      promise.setValue(_server->registeredServices());
      return promise.future();
    }

    return  _sdClient->services();
  }

}
