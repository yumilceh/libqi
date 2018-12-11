/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "testsession_p.hpp"
#include <qi/log.hpp>

qiLogCategory("TestSession");

TestSessionPrivate::TestSessionPrivate(const std::string &serviceDirectoryUrl, TestMode::Mode mode, bool listen)
{
  _mode = mode;
  _session = qi::makeSession();
  _manager = new SessionInitializer();
  _manager->setUp(_session, serviceDirectoryUrl, _mode, listen);
}

TestSessionPrivate::~TestSessionPrivate()
{
  try
  {
    _manager->tearDown(_session, _mode);
  }
  catch (const std::exception& e)
  {
    qiLogError() << "Error closing the session: " << e.what();
  }

  delete _manager;
}

qi::SessionPtr TestSessionPrivate::session()
{
  return _session;
}
