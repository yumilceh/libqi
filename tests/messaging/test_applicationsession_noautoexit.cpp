/*
 ** Author(s):
 **  - Ly William Chhim <lwchhim@aldebaran-robotics.com>
 **
 ** Copyright (C) 2013 Aldebaran Robotics
 */

#include <gtest/gtest.h>

#include <qi/applicationsession.hpp>
#include <thread>
#include <chrono>

static bool _stopped = false;
static qi::ApplicationSession* _app;
static qi::SessionPtr _sd;
static char **_argv = nullptr;
static int _argc = 3;

void onStop()
{
  _stopped = true;
}

TEST(QiApplicationSessionNoAutoExit, defaultConnect)
{
  ASSERT_FALSE(_app->session()->isConnected());
  _app->startSession();
  ASSERT_TRUE(_app->session()->isConnected());
  ASSERT_EQ(_sd->endpoints()[0].str(), _app->session()->url());

  ASSERT_FALSE(_stopped);
  _sd->close();
  std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
  ASSERT_FALSE(_stopped);
}

TEST(QiApplicationSessionNoAutoConnect, checkArgs)
{
  ASSERT_EQ(3, _app->argc());
  EXPECT_EQ(std::string("no"), _app->argv()[0]);
  EXPECT_EQ(std::string("options"), _app->argv()[1]);
  EXPECT_EQ(std::string("given"), _app->argv()[2]);

  ASSERT_EQ(3, _argc);
  EXPECT_EQ(std::string("no"), _argv[0]);
  EXPECT_EQ(std::string("options"), _argv[1]);
  EXPECT_EQ(std::string("given"), _argv[2]);
  EXPECT_EQ(0, _argv[3]);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  _sd = qi::makeSession();
  _sd->listenStandalone("tcp://127.0.0.1:0");
  _argv = new char*[4];
  strcpy((_argv[0] = new char[4]), "no");
  strcpy((_argv[1] = new char[10]), "options");
  strcpy((_argv[2] = new char[10]), "given");
  _argv[3] = 0;

  qi::ApplicationSession app(_argc, _argv, qi::ApplicationSession::Option_NoAutoExit, _sd->endpoints()[0]);
  _app = &app;
  app.atStop(&onStop);
  return RUN_ALL_TESTS();
}
