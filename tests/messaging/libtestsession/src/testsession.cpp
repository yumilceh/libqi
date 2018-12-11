/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <testsession/testsession.hpp>
#include "testsession_p.hpp"
#include <qi/testutils/testutils.hpp>

using namespace qi;

TestSession::TestSession(const std::string &serviceDirectoryUrl, bool listen, TestMode::Mode mode)
{
  _p = new TestSessionPrivate(serviceDirectoryUrl, mode, listen);
}

TestSession::~TestSession()
{
  delete _p;
}

qi::SessionPtr TestSession::session()
{
  return _p->session();
}
