/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _TESTS_LIBTESTSESSION_TRAFFICGENERATOR_HPP_
#define _TESTS_LIBTESTSESSION_TRAFFICGENERATOR_HPP_

#include <vector>
#include <map>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <qi/session.hpp>

void __chaosThread(void *data);

struct Behavior
{
  qi::SessionPtr session;
  std::string   service;
  boost::mutex *mutex;
};

class TrafficGenerator
{
public:
  TrafficGenerator();
  ~TrafficGenerator();

public:
  bool generateCommonTraffic(const std::vector<qi::SessionPtr>& sessions, const std::string &serviceName);
  bool generateSpam(std::vector<qi::SessionPtr> &sessions);

  bool stopTraffic();

public:
  std::map<boost::mutex *, boost::thread *> _clients;
};

#endif // !_TESTS_LIBTESTSESSION_TRAFFICGENERATOR_HPP_
