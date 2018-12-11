#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>

#include <qi/application.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/type/objectfactory.hpp>
#include <qi/session.hpp>
#include <testsession/testsessionpair.hpp>

#include "taskgenerator.hpp"

qiLogCategory("Test.Task");

class TestTask: public ::testing::Test
{
public:
  TestTask()
  {
    static bool init = false;
    static qi::AnyObject tgs;
    if (!init)
    {
      std::vector<std::string> objs = qi::loadObject("task");
      qiLogInfo() << "loadobject gave " << objs.size();
      if (objs.size() != 1)
        throw std::runtime_error("Expected one object in taskService");
      tgs = qi::createObject("TaskGenerator");
      if (!tgs)
        throw std::runtime_error("No TaskGenerator service found");
      init = true;
    }
    taskGenService = tgs;
  }

protected:
  void SetUp()
  {
    p.server()->registerService("taskGen", taskGenService);
    taskGenClient = p.client()->service("taskGen");
    ASSERT_TRUE(taskGenClient);
    taskGenProxy = qi::Object<TaskGenerator>(taskGenClient);
  }

  void TearDown()
  {
    taskGenClient = qi::AnyObject();
    taskGenProxy.reset();
  }

public:
  TestSessionPair      p;
  qi::AnyObject        taskGenService;
  qi::AnyObject        taskGenClient;
  qi::Object<TaskGenerator> taskGenProxy; // specialized version of taskClient
};

TEST_F(TestTask, Basic)
{
  ASSERT_EQ(0U, taskGenProxy->taskCount());
}

TEST_F(TestTask, AnyTask)
{
  qi::details::printMetaObject(std::cerr, taskGenProxy.metaObject());
  qi::AnyObject task = taskGenProxy.call<qi::AnyObject>("newTask", "coin");
  ASSERT_TRUE(task);
  std::string n = task.call<std::string>("getName");

  ASSERT_EQ(n, "coin");

  unsigned count = taskGenProxy->taskCount();
  ASSERT_EQ(1U, count);

  task.call<void>("setParam","foo");
  task.call<void>("step", 42);
  std::string lr = task.call<std::string>("getLastResult");
  ASSERT_EQ("coin foo 42 1", lr);
  // Release the task, ensure it gets destroyed
  qiLogVerbose() << "Releasing task and waiting";
  task.reset();
  qi::os::msleep(400);
  qiLogVerbose() << "Done";
  count = taskGenProxy->taskCount();
  ASSERT_EQ(0U, count);
}

TEST_F(TestTask, Task)
{
  qi::details::printMetaObject(std::cerr, taskGenProxy.metaObject());
  //qi::AnyObject gtask = taskGenProxy->newTask("coin");
  //qi::Object<Task> task = gtask; // taskGenProxy->newTask("coin");
  qi::Object<Task> task = taskGenProxy->newTask("coin");
  //task2.asT();
  ASSERT_TRUE(task);
  std::string n = task.call<std::string>("getName");
  ASSERT_EQ(n, "coin");
  Task& rawTask = task.asT();
  n = rawTask.getName();
  ASSERT_EQ(n, "coin");
  n = task->getName();

  ASSERT_EQ(n, "coin");

  unsigned count = taskGenProxy->taskCount();
  ASSERT_EQ(1U, count);

  task->setParam("foo");
  task->step(42);
  std::string lr = task->getLastResult();
  ASSERT_EQ("coin foo 42 1", lr);
  // Release the task, ensure it gets destroyed
  qiLogVerbose() << "Resetting task";
  task.reset();
  qiLogVerbose() << "Reset done";
  qi::os::msleep(400);
  qiLogVerbose() << "Wait done";
  count = taskGenProxy->taskCount();
  ASSERT_EQ(0U, count);
}

TEST_F(TestTask, Signals)
{
  qi::Object<Task> task = taskGenProxy->newTask("coin");
  ASSERT_TRUE(task);
  qi::Promise<std::string> p;
  task->onParamChanged.connect(
    boost::bind(&qi::Promise<std::string>::setValue, p, _1));
  task->setParam("foo");
  ASSERT_EQ("foo", p.future().value());
  std::vector<qi::Object<Task> > tasks = taskGenProxy->tasks();
  ASSERT_EQ(1U, tasks.size());
  p.reset();
  tasks[0]->setParam("bar");
  ASSERT_EQ("bar", p.future().value());
  tasks.clear();
  task.reset();
  qi::os::msleep(400);
  unsigned count = taskGenProxy->taskCount();
  ASSERT_EQ(0U, count);
}

struct Context
{
  qi::Session*          session;
  qi::AnyObject         taskGenClient;
  qi::Object<TaskGenerator>   taskGenProxy;
  std::vector<qi::Object<Task> > tasks;
};

void on_step(const std::string& s, qi::Atomic<int>& i)
{
  //std::cerr << "##STEP " << s << std::endl;
  ++i;
}

TEST_F(TestTask, ManyManyTasks)
{
  long long time = qi::os::ustime();
  unsigned NCLIENTS = 10;
  unsigned NTASKS = 10;
  if (getenv("NCLIENTS"))
    NCLIENTS = strtol(getenv("NCLIENTS"), 0, 0);
  if (getenv("NTASKS"))
    NTASKS = strtol(getenv("NTASKS"), 0, 0);
  unsigned TOTAL_CLIENTS = NTASKS * NCLIENTS;
  std::vector<Context> clients;
  for (unsigned i=0; i<NCLIENTS; ++i)
  {
    Context c;
    c.session = new qi::Session();
    c.session->connect(p.serviceDirectoryEndpoints()[0]);
    c.taskGenClient = c.session->service("taskGen");
    c.taskGenProxy = qi::Object<TaskGenerator>(taskGenClient);
    clients.push_back(c);
  }
  qiLogInfo() << "Setup clients: " << (qi::os::ustime() - time) << std::endl;
  time = qi::os::ustime();
  qi::Atomic<int> counter;
  for (unsigned i=0; i<NCLIENTS; ++i)
  {
    for (unsigned j=0; j<NTASKS; ++j)
    {
      qi::Object<Task> task = clients[i].taskGenProxy->newTask(
        boost::lexical_cast<std::string>(i*NTASKS + j));
      clients[i].tasks.push_back(task);
      task->onStep.connect(
        boost::bind(on_step, _1, boost::ref(counter)));
    }
  }
  ASSERT_EQ(NCLIENTS*NTASKS, taskGenProxy->taskCount());
  qiLogInfo() << "Setup tasks: " << (qi::os::ustime() - time) << std::endl;
  time = qi::os::ustime();
  taskGenProxy->step(1);
  for (unsigned step=0; step<1000 && *counter < TOTAL_CLIENTS; ++step)
    qi::os::msleep(5);
  ASSERT_EQ(*counter, TOTAL_CLIENTS);
  std::cerr << "Run: " << (qi::os::ustime() - time) << std::endl;
  // Teardown
  for (unsigned i=0; i<NCLIENTS; ++i)
  {
    delete clients[i].session;
  }
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  TestMode::forceTestMode(TestMode::Mode_SD);
  TestMode::initTestMode(argc, argv);
  return RUN_ALL_TESTS();
}
