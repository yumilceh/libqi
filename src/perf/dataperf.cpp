/*
** Author(s):
** - Nicolas Cornu <ncornu@aldebaran-robotics.com>
**
** Copyright (C) 2012-2013 Aldebaran Robotics
*/

#include "dataperf_p.hpp"

namespace qi
{
  DataPerfPrivate::DataPerfPrivate()
    : benchmarkName(""), wallClockElapsed(0), cpuElapsed(0), fLoopCount(0), fMsgSize(0), variable("")
  { }

  DataPerf::DataPerf()
    : _p(new DataPerfPrivate)
  { }

  DataPerf::~DataPerf()
  {
    delete _p;
  }

  void DataPerf::start(const std::string& benchmarkName, unsigned long loopCount, unsigned long msgSize, const std::string& variable)
  {
    _p->benchmarkName = benchmarkName;
    _p->fLoopCount = loopCount;
    _p->fMsgSize = msgSize;
    _p->variable = variable;
    _p->cpuTime.restart();
    _p->fStartTime = qi::SteadyClock::now();
  }

  void DataPerf::stop() {
    _p->cpuElapsed = _p->cpuTime.elapsed();
    using second = boost::chrono::duration<double, boost::ratio<1,1>>;
    _p->wallClockElapsed = qi::durationSince<second>(_p->fStartTime).count();
  }

  std::string DataPerf::getBenchmarkName() const
  {
    return _p->benchmarkName;
  }

  std::string DataPerf::getVariable() const
  {
    return _p->variable;
  }

  unsigned long DataPerf::getMsgSize() const
  {
    return _p->fMsgSize;
  }

  double DataPerf::getPeriod() const
  {
    return (double)(_p->wallClockElapsed) * 1000.0 * 1000.0 / (_p->fLoopCount);
  }

  double DataPerf::getCpu() const
  {
    return (double)(_p->cpuElapsed) / (double)(_p->wallClockElapsed) * 100;
  }

  double DataPerf::getMsgPerSecond() const
  {
    return 1.0 / ((_p->wallClockElapsed) / (1.0 * (_p->fLoopCount)));
  }

  double DataPerf::getMegaBytePerSecond() const
  {
    if (_p->fMsgSize > 0)
      return (getMsgPerSecond() * _p->fMsgSize) / (1024.0 * 1024.0);

    return -1;
  }
}
