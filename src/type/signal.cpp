/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <atomic>
#include <map>
#include <numeric>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/make_shared.hpp>

#include <qi/signal.hpp>
#include <qi/anyvalue.hpp>
#include <qi/anyobject.hpp>
#include <qi/assert.hpp>
#include <qi/numeric.hpp>
#include <ka/algorithm.hpp>

#include "signal_p.hpp"

qiLogCategory("qitype.signal");

namespace qi {

  SignalBasePrivate::~SignalBasePrivate()
  {
    {
      boost::recursive_mutex::scoped_lock lock(mutex);
      onSubscribers = SignalBase::OnSubscribers();
    }
    disconnectAll();
  }

  Future<bool> SignalBasePrivate::disconnect(const SignalLink& l)
  {
    SignalSubscriber subscriber;
    Future<void> callingOnSubscribers{nullptr};
    SignalBase::OnSubscribers onSubscribersToCall;
    {
      // Acquire signal mutex
      boost::recursive_mutex::scoped_lock sigLock(mutex);
      SignalSubscriberMap::iterator it = subscriberMap.find(l);
      if (it == subscriberMap.end())
      {
        qiLogWarning() << "disconnect: No subscription found for SignalLink " << l << ".";
        return Future<bool>{false};
      }
      subscriber = it->second;
      // Remove from map (but SignalSubscriber object still good)
      subscriberMap.erase(it);
      if (subscriberMap.empty() && onSubscribers)
        onSubscribersToCall = onSubscribers;
      // Ensure no call on subscriber occurs once this function returns
      subscriber._p->enabled = false;
    }
    if (onSubscribersToCall)
      callingOnSubscribers = onSubscribersToCall(false);

    return callingOnSubscribers.andThen([](void*) { return true; });
  }

  Future<bool> SignalBasePrivate::disconnectAll()
  {
    return disconnectAllStep(true);
  }

  Future<bool> SignalBasePrivate::disconnectAllStep(bool overallSuccess)
  {
    SignalLink link;
    FutureBarrier<bool> barrier;
    while (true)
    {
      {
        boost::recursive_mutex::scoped_lock sl(mutex);
        SignalSubscriberMap::iterator it = subscriberMap.begin();
        if (it == subscriberMap.end())
          break;
        link = it->first;
      }
      // allow for multiple disconnects to occur at the same time, we must not
      // keep the lock
      barrier.addFuture(disconnect(link));
    }
    return barrier.future().andThen([](const std::vector<Future<bool>>& successes)
    {
      for (const auto& success: successes)
        if (!success.value()) return false;
      return true;
    });
  }

  SignalSubscriberPrivate::SignalSubscriberPrivate() = default;
  SignalSubscriberPrivate::~SignalSubscriberPrivate() = default;

QI_WARNING_PUSH()
QI_WARNING_DISABLE(4996, deprecated-declarations) // ignore linkId deprecation warnings
  SignalSubscriber::SignalSubscriber()
    : _p(std::make_shared<SignalSubscriberPrivate>())
    , linkId(_p->linkId)
  {
  }

  SignalSubscriber::SignalSubscriber(const SignalSubscriber& other) = default;
  SignalSubscriber& SignalSubscriber::operator=(const SignalSubscriber& other) = default;
QI_WARNING_POP()

  SignalSubscriber::SignalSubscriber(const AnyObject& target, unsigned int method)
    : SignalSubscriber()
  { // The slot has its own threading model: use sync call type (default)
    _p->target.reset(new AnyWeakObject(target));
    _p->method = method;
  }

  SignalSubscriber::SignalSubscriber(AnyFunction func, MetaCallType model)
     : SignalSubscriber()
  {
    _p->handler = func;
    _p->threadingModel = model;
  }

  SignalSubscriber::SignalSubscriber(AnyFunction func, ExecutionContext* ec)
    : SignalSubscriber()
  { // The execution context will reschedule the call like it wants, use sync call type
    _p->handler = func;
    _p->executionContext = ec;
  }

  SignalSubscriber::~SignalSubscriber() = default;

  static std::atomic<SignalLink> linkUid{1};

  void SignalBase::setCallType(MetaCallType callType)
  {
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock lock(_p->mutex);
    _p->defaultCallType = callType;
  }

  void SignalBase::operator()(
      qi::AutoAnyReference p1,
      qi::AutoAnyReference p2,
      qi::AutoAnyReference p3,
      qi::AutoAnyReference p4,
      qi::AutoAnyReference p5,
      qi::AutoAnyReference p6,
      qi::AutoAnyReference p7,
      qi::AutoAnyReference p8)
  {
    qi::AutoAnyReference* vals[8]= {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8};
    std::vector<qi::AnyReference> params;
    for (unsigned i = 0; i < 8; ++i)
      if (vals[i]->isValid())
        params.push_back(*vals[i]);
    qi::Signature signature = qi::makeTupleSignature(params);

    auto mct = [&] () {
      QI_ASSERT(_p);
      boost::recursive_mutex::scoped_lock lock(_p->mutex);
      if (signature != _p->signature)
      {
        qiLogError() << "Dropping emit: signature mismatch: "
                     << signature.toString() << " " << _p->signature.toString();
        return MetaCallType_Auto;
      }
      return _p->defaultCallType;
    }();

    trigger(params, mct);
  }

  void SignalBase::trigger(const GenericFunctionParameters& params, MetaCallType callType)
  {
    QI_ASSERT(_p);
    SignalBase::Trigger trigger;
    {
      boost::recursive_mutex::scoped_lock lock(_p->mutex);
      trigger = _p->triggerOverride;
    }
    if (trigger)
      trigger(params, callType);
    else
      callSubscribers(params, callType);
  }

  void SignalBase::setTriggerOverride(Trigger t)
  {
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock lock(_p->mutex);
    _p->triggerOverride = t;
  }

  void SignalBase::setOnSubscribers(OnSubscribers onSubscribers)
  {
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock lock(_p->mutex);
    _p->onSubscribers = onSubscribers;
  }

  void SignalBase::callSubscribers(const GenericFunctionParameters& params, MetaCallType callType)
  {
    MetaCallType mct = callType;
    QI_ASSERT(_p);

    SignalSubscriberMap copy;
    {
      boost::recursive_mutex::scoped_lock lock(_p->mutex);
      if (mct == qi::MetaCallType_Auto)
        mct = _p->defaultCallType;

      copy = _p->subscriberMap;
    }
    qiLogDebug() << this << " Invoking signal subscribers: " << copy.size();
    for (auto& i: copy)
    {
      qiLogDebug() << this << " Invoking signal subscriber";
      SignalSubscriber s = i.second; // holds the subscription alive
      s.call(params, mct);
    }
    qiLogDebug() << this << " done invoking signal subscribers";
  }

  void SignalSubscriber::callImpl(const GenericFunctionParameters& args)
  {
    if (!_p->enabled)
      return;

    // do not throw
    bool mustDisconnect = false;
    try
    {
      _p->handler(args);
    }
    catch (const qi::PointerLockException&)
    {
      qiLogDebug() << "PointerLockFailure excepton, will disconnect";
      mustDisconnect = true;
    }
    catch (const std::exception& e)
    {
      qiLogWarning() << "Exception caught from signal subscriber: " << e.what();
    }
    catch (...)
    {
      qiLogWarning() << "Unknown exception caught from signal subscriber";
    }

    if (mustDisconnect)
    {
      // if enabled is false, we are already disconnected
      if (_p->enabled)
      {
        auto sbp = _p->source.lock();
        if(sbp)
          sbp->disconnect(_p->linkId).wait();
      }
    }
  }

  void SignalSubscriber::call(const GenericFunctionParameters& args, MetaCallType callType)
  {
    // this is held alive by caller
    if (_p->handler)
    {
      bool async = true;
      if (_p->threadingModel != MetaCallType_Auto)
        async = (_p->threadingModel == MetaCallType_Queued);
      else if (callType != MetaCallType_Auto)
        async = (callType == MetaCallType_Queued);

      qiLogDebug() << "subscriber call async=" << async <<" ct " << callType <<" tm " << _p->threadingModel;
      ExecutionContext* executionContext = _p->executionContext;
      if (executionContext || async)
      {
        // We will check enabled when we will be scheduled in the target
        // thread, and we hold this SignalSubscriber alive, so no need to
        // explicitly track the asynccall

        // courtesy-check of el, but it should be kept alive longuer than us
        if (!executionContext)
        {
          executionContext = getEventLoop();
          if (!executionContext) // this is an assert basicaly, no sense trying to do something clever.
            throw std::runtime_error("Event loop was destroyed");
        }

        auto subscriberCopy = *this;
        std::shared_ptr<GenericFunctionParameters> argsCopy{ new auto(args.copy()), [](GenericFunctionParameters* object) {
          object->destroy(); // see GenericFunctionParameters::copy() for details
          delete object;
        } };

        executionContext->post([subscriberCopy, argsCopy] () mutable{
          subscriberCopy.callImpl(*argsCopy);
        });

      }
      else
      {
        callImpl(args);
      }
    }
    else if (_p->target)
    {
      AnyObject lockedTarget = _p->target->lock();
      if (!lockedTarget)
      {
        if (_p->enabled)
        {
          // see above
          auto sbp = _p->source.lock();
          if (sbp)
            sbp->disconnect(_p->linkId).wait();
        }
      }
      else // no need to keep anything locked, whatever happens this is not used
        lockedTarget.metaPost(_p->method, args);
    }
  }

  SignalSubscriber SignalSubscriber::setCallType(MetaCallType ct)
  {
    _p->threadingModel = ct;
    return *this;
  }

  SignalLink SignalSubscriber::link() const
  {
    return _p->linkId;
  }

  SignalSubscriber::operator SignalLink() const
  {
    return _p->linkId;
  }

  SignalSubscriber SignalBase::connect(AnyObject o, unsigned int slot)
  {
    return connect(SignalSubscriber(o, slot));
  }

  Signature SignalSubscriber::signature() const
  {
    if (_p->handler)
    {
      if (_p->handler.functionType() == dynamicFunctionTypeInterface())
        return Signature(); // no arity checking is possible
      else
        return _p->handler.parametersSignature();
    }
    else if (_p->target)
    {
      AnyObject locked = _p->target->lock();
      if (!locked)
        return Signature();
      const MetaMethod* ms = locked.metaObject().method(_p->method);
      if (!ms)
      {
        qiLogWarning() << "Method " << _p->method <<" not found.";
        return Signature();
      }
      else
        return ms->parametersSignature();
    }
    else
      return Signature();
  }

  SignalSubscriber SignalBase::connect(const SignalSubscriber& src)
  {
    return connectAsync(src).value();
  }

  Future<SignalSubscriber> SignalBase::connectAsync(const SignalSubscriber& src)
  {
    qiLogDebug() << this << " connecting new subscriber";
    QI_ASSERT(_p);
    // Check arity. Does not require to acquire weakLock.
    // Convert the number of children to int because we expect it will never be bigger than INT_MAX.
    const auto signalArity = qi::numericConvert<int>(signature().children().size());
    auto subscriberArity = -1;
    Signature subscriberSignature = src.signature();
    if (subscriberSignature.isValid())
      subscriberArity = static_cast<int>(subscriberSignature.children().size());

    if (signature() != "m" && subscriberSignature.isValid())
    {
      if (signalArity != subscriberArity)
      {
        std::stringstream s;
        s << "Subscriber has incorrect arity (expected maximum "
          << signalArity << " , got " << subscriberArity << ")";
        throw std::runtime_error(s.str());
      }
      if (signature().isConvertibleTo(subscriberSignature) == 0.f)
      {
        std::stringstream s;
        s << "Subscriber is not compatible to signal : "
          << signature().toString() << " vs " << subscriberSignature.toString();
        throw std::runtime_error(s.str());
      }
    }

    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    bool first = _p->subscriberMap.empty();
    SignalLink res = ++linkUid;
    SignalSubscriber& subscriberInMap = _p->subscriberMap[res];
    subscriberInMap = src;
    subscriberInMap._p->linkId = res;
    subscriberInMap._p->source = this->_p;
    Future<void> callingOnSubscribers{nullptr};
    if (first && _p->onSubscribers)
    {
      qiLogDebug() << this << " calling onSubscribers";
      callingOnSubscribers = _p->onSubscribers(true).andThen([&](void*)
      {
        qiLogDebug() << this << " onSubscribers called";
      });
    }
    else
    {
      qiLogDebug() << this << " not calling onSubscribers";
    }

    // Return a copy asynchronously. Too bad it makes few allocations.
    SignalSubscriber subscriberToReturn = subscriberInMap;
    return callingOnSubscribers.andThen([=](void*)
    {
      qiLogDebug() << this << " connected";
      return subscriberToReturn;
    });
  }

  void SignalBase::createNewTrackLink(int& id, SignalLink*& pLink)
  {
    id = ++_p->trackId;
    {
      boost::recursive_mutex::scoped_lock l(_p->mutex);
      pLink = &_p->trackMap[id];
    }
  }

  void SignalBase::disconnectTrackLink(int id)
  {
    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    TrackMap::iterator it = _p->trackMap.find(id);
    if (it == _p->trackMap.end())
      return;

    _p->subscriberMap.erase(it->second);
    _p->trackMap.erase(it);
  }

  ExecutionContext* SignalBase::executionContext() const
  {
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    return _p->execContext;
  }

  void SignalBase::clearExecutionContext()
  {
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    _p->execContext = nullptr;
  }

  bool SignalBase::disconnectAll()
  {
    QI_ASSERT(_p);
    return _p->disconnectAll().value();
  }

  Future<bool> SignalBase::disconnectAllAsync()
  {
    QI_ASSERT(_p);
    return _p->disconnectAll();
  }

  SignalBase::SignalBase(const qi::Signature& sig, OnSubscribers onSubscribers)
    : SignalBase(sig, nullptr, std::move(onSubscribers))
  {
  }

  SignalBase::SignalBase(const Signature& sig, ExecutionContext* execContext,
                         SignalBase::OnSubscribers onSubscribers)
    : _p(new SignalBasePrivate)
  {
    //Dynamic mean AnyArguments here.
    if (sig.type() != qi::Signature::Type_Dynamic && sig.type() != qi::Signature::Type_Tuple)
      throw std::runtime_error("Signal signature should be tuple, or AnyArguments");
    _p->onSubscribers = onSubscribers;
    _p->signature = sig;
    _p->execContext = execContext;
  }

  SignalBase::SignalBase(OnSubscribers onSubscribers)
    : SignalBase(nullptr, std::move(onSubscribers))
  {
  }

  SignalBase::SignalBase(ExecutionContext* execContext, OnSubscribers onSubscribers)
    : _p(new SignalBasePrivate)
  {
    _p->onSubscribers = onSubscribers;
    _p->execContext = execContext;
  }

  qi::Signature SignalBase::signature() const
  {
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock lock(_p->mutex);
    return _p->signature;
  }

  void SignalBase::_setSignature(const qi::Signature& s)
  {
    boost::recursive_mutex::scoped_lock lock(_p->mutex);
    _p->signature = s;
  }


  bool SignalBase::disconnect(const SignalLink &link) {
    QI_ASSERT(_p);
    return _p->disconnect(link).value();
  }

  Future<bool> SignalBase::disconnectAsync(const SignalLink &link) {
    QI_ASSERT(_p);
    return _p->disconnect(link);
  }

  SignalBase::~SignalBase() = default;

  std::vector<SignalSubscriber> SignalBase::subscribers()
  {
    std::vector<SignalSubscriber> res;
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    for (const auto& i: _p->subscriberMap)
      res.push_back(i.second);
    return res;
  }

  bool SignalBase::hasSubscribers()
  {
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    return !_p->subscriberMap.empty();
  }

  SignalSubscriber SignalBase::connect(AnyObject obj, const std::string& slot)
  {
    const MetaObject& mo = obj.metaObject();
    const MetaSignal* sig = mo.signal(slot);
    if (sig)
      return connect(SignalSubscriber(obj, sig->uid()));
    std::vector<MetaMethod> method = mo.findMethod(slot);
    if (method.empty())
      throw std::runtime_error("No match found for slot " + slot);
    if (method.size() > 1)
      throw std::runtime_error("Ambiguous slot name " + slot);
    return connect(SignalSubscriber(obj, method.front().uid()));
  }

  const SignalLink SignalBase::invalidSignalLink =
      std::numeric_limits<SignalLink>::max();

}
