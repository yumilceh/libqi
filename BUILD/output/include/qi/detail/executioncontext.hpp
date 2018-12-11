#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_EXECUTION_CONTEXT_HPP_
#define _QI_EXECUTION_CONTEXT_HPP_

#include <type_traits>
#include <boost/function.hpp>
#include <qi/api.hpp>
#include <qi/clock.hpp>
#include <ka/typetraits.hpp>

namespace qi
{

template <typename T>
class Future;

namespace detail
{

  // This class is kind of a hack to deprecate the old versions of async/post etc without deprecating the correct use of
  // the new ones. This class is just a strong alias to boost::function
  template <typename T>
  struct Function : boost::function<T>
  {
    template<class Arg>
    Function(Arg&& arg,
        typename std::enable_if<!std::is_same<typename std::decay<Arg>::type, Function>::value>::type* = nullptr)
      : boost::function<T>(std::forward<Arg>(arg))
    {}

    Function(const Function&) = default;
  };

}

enum class CancelOption     ///< If cancel have been requested for the associated scheduled task...
{ AllowSkipExecution        ///< ... allow the execution context to not execute the task.
, AlwaysSkipExecution       ///< ... the execution context must not execute the task.
, NeverSkipExecution        ///< ... the executino context must still execute the task.
};

/// Represent execution behaviour options attached to a task that must be interpreted by an ExecutionContext.
struct ExecutionOptions
{
  /** Specifies the behaviour when the task is scheduled but not executed
      and it have been marked as cancel-requested.
  */
  CancelOption onCancelRequested;
};

BOOST_CONSTEXPR
inline ExecutionOptions defaultExecutionOptions() BOOST_NOEXCEPT
{
  return { CancelOption::AllowSkipExecution };
}


class QI_API ExecutionContext
{
public:
  virtual ~ExecutionContext() {}

  // DEPRECATED STUFF
  /// call a callback asynchronously to be executed on tp
  /// @deprecated since 2.5
  QI_API_DEPRECATED_MSG(Use 'asyncAt' instead)
  virtual qi::Future<void> async(const boost::function<void()>& callback, qi::SteadyClockTimePoint tp) = 0;
  /// call a callback asynchronously to be executed in delay
  /// @deprecated since 2.5
  QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
  virtual qi::Future<void> async(const boost::function<void()>& callback, qi::Duration delay) = 0;

  /// call a callback asynchronously to be executed in delay
  /// @deprecated since 2.5
  template <typename R>
  QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
  typename boost::disable_if<std::is_same<R, void>, qi::Future<R> >::type
  async(const boost::function<R()>& callback, qi::Duration delay);
  /// call a callback asynchronously to be executed on tp
  /// @deprecated since 2.5
  template <typename R>
  QI_API_DEPRECATED_MSG(Use 'asyncAt' instead)
  typename boost::disable_if<std::is_same<R, void>, qi::Future<R> >::type
  async(const boost::function<R()>& callback, qi::SteadyClockTimePoint tp);

  /// @deprecated since 2.5
  template <typename R>
  QI_API_DEPRECATED_MSG(Use 'async' without explicit return type template arguement instead)
  qi::Future<R> async(const detail::Function<R()>& callback)
  {
    return asyncDelay(callback, qi::Duration(0));
  }
  // END OF DEPRECATED STUFF

  /// post a callback to be executed as soon as possible
  template <typename F>
  void post(F&& callback, ExecutionOptions options = defaultExecutionOptions());

  /// call a callback asynchronously to be executed on tp
  template <typename F, typename R = ka::Decay<decltype(std::declval<F>()())>>
  qi::Future<R> asyncAt(F&& callback, qi::SteadyClockTimePoint tp, ExecutionOptions options = defaultExecutionOptions());
  /// call a callback asynchronously to be executed in delay
  template <typename F, typename R = ka::Decay<decltype(std::declval<F>()())>>
  qi::Future<R> asyncDelay(F&& callback, qi::Duration delay, ExecutionOptions options = defaultExecutionOptions());

  template <typename F>
  auto async(F&& callback, ExecutionOptions options = defaultExecutionOptions())
    -> decltype(asyncDelay(std::forward<F>(callback), qi::Duration(0), options))
  {
    return asyncDelay(std::forward<F>(callback), qi::Duration(0), options);
  }

  /// return true if the current thread is in this context
  virtual bool isInThisContext() const = 0;

protected:
  virtual void postImpl(boost::function<void()> callback, ExecutionOptions options) = 0;
  virtual qi::Future<void> asyncAtImpl(boost::function<void()> cb, qi::SteadyClockTimePoint tp, ExecutionOptions options) = 0;
  virtual qi::Future<void> asyncDelayImpl(boost::function<void()> cb, qi::Duration delay, ExecutionOptions options) = 0;
};

}

#include <qi/detail/future_fwd.hpp>

namespace qi
{

namespace detail
{

template <typename T>
class DelayedPromise: public Promise<T>
{
public:
  void setup(boost::function<void (qi::Promise<T>)> cancelCallback, FutureCallbackType async = FutureCallbackType_Async)
  {
    Promise<T>::setup(cancelCallback, async);
  }
};

template <typename R>
void setValue(qi::Promise<R>& p, const boost::function<R()>& f)
{
  p.setValue(f());
}

template <>
inline void setValue<void>(qi::Promise<void>& p, const boost::function<void()>& f)
{
  f();
  p.setValue(0);
}

template <typename R>
void callAndSet(qi::Promise<R> p, boost::function<R()> f)
{
  try
  {
    setValue<R>(p, f);
  }
  catch (const std::exception& e)
  {
    p.setError(e.what());
  }
  catch(...)
  {
    p.setError("unknown exception");
  }
}
template <typename R>
void checkCanceled(qi::Future<void> f, qi::Promise<R> p)
{
  if (f.wait() == FutureState_Canceled)
    p.setCanceled();
  // Nothing to do for other states.
}

}

template <typename R>
typename boost::disable_if<std::is_same<R, void>,
                            qi::Future<R> >::type
    ExecutionContext::async(const boost::function<R()>& callback,
                            qi::Duration delay)
{
  detail::DelayedPromise<R> promise;
  qi::Future<void> f = async(boost::function<void()>(boost::bind(
                                 detail::callAndSet<R>, promise, callback)),
                             delay);
  promise.setup(
      boost::bind(&detail::futureCancelAdapter<void>,
                  boost::weak_ptr<detail::FutureBaseTyped<void> >(f.impl())));
  f.connect(boost::bind(&detail::checkCanceled<R>, _1, promise), FutureCallbackType_Sync);
  return promise.future();
}

template <typename R>
typename boost::disable_if<std::is_same<R, void>,
                            qi::Future<R> >::type
    ExecutionContext::async(const boost::function<R()>& callback,
                            qi::SteadyClockTimePoint tp)
{
  detail::DelayedPromise<R> promise;
  qi::Future<void> f = async(boost::function<void()>(boost::bind(
                                 detail::callAndSet<R>, promise, callback)),
                             tp);
  promise.setup(
      boost::bind(&detail::futureCancelAdapter<void>,
                  boost::weak_ptr<detail::FutureBaseTyped<void> >(f.impl())));
  f.connect(boost::bind(&detail::checkCanceled<R>, _1, promise), FutureCallbackType_Sync);
  return promise.future();
}

template <typename F>
void ExecutionContext::post(F&& callback, ExecutionOptions options)
{
  postImpl(std::forward<F>(callback), options);
}

template <typename ReturnType, typename Callback>
struct ToPost
{
  detail::DelayedPromise<ReturnType> promise;
  Callback callback;

  ToPost(Callback cb) :
    callback(std::move(cb))
  {}

  void operator()()
  {
    detail::callAndSet<ReturnType>(std::move(promise), std::move(callback));
  }
};

template <typename F, typename R>
Future<R> ExecutionContext::asyncAt(F&& callback, qi::SteadyClockTimePoint tp, ExecutionOptions options)
{
  ToPost<R, typename std::decay<F>::type> topost(std::forward<F>(callback));
  auto promise = topost.promise;
  qi::Future<void> f = asyncAtImpl(std::move(topost), tp, options);
  promise.setup(boost::bind(&detail::futureCancelAdapter<void>,
                            boost::weak_ptr<detail::FutureBaseTyped<void> >(f.impl())));
  f.connect(boost::bind(&detail::checkCanceled<R>, _1, promise), FutureCallbackType_Sync);
  return promise.future();
}

template <typename F, typename R>
Future<R> ExecutionContext::asyncDelay(F&& callback, qi::Duration delay, ExecutionOptions options)
{
  ToPost<R, typename std::decay<F>::type> topost(std::forward<F>(callback));
  auto promise = topost.promise;
  qi::Future<void> f = asyncDelayImpl(std::move(topost), delay, options);
  promise.setup(boost::bind(&detail::futureCancelAdapter<void>,
                            boost::weak_ptr<detail::FutureBaseTyped<void> >(f.impl())));
  f.connect(boost::bind(&detail::checkCanceled<R>, _1, promise), FutureCallbackType_Sync);
  return promise.future();
}
}

#endif
