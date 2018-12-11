#ifndef KA_ERRORHANDLING_HPP
#define KA_ERRORHANDLING_HPP
#pragma once
#include <stdexcept>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception/exception.hpp>
#include "macroregular.hpp"
#include "utility.hpp"

namespace ka {
  /// Polymorphic procedure that handles an exception through a dedicated handler
  /// and rethrows the exception as-is.
  ///
  /// Warning: Must be called in a `catch` clause, otherwise `std::terminate` will
  ///   be called (because there is no exception to rethrow).
  ///
  /// Example: Logs in error log and rethrows the exception.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// using Log = ExceptionLogError<std::string>;
  /// handle_exception_rethrow<Log> log_rethrow{Log{"logCategory", "doStuff"}};
  /// // ...
  /// auto res = invoke_catch(log_rethrow, doStuff, arg0, arg1)
  /// // ...
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Note: Members are public because there's no invariant to maintain.
  ///
  /// PolymorphicProcedure<void (T), void ()> Proc
  template<typename Proc>
  struct handle_exception_rethrow {
    Proc handle_exception;
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_1(handle_exception_rethrow, handle_exception)
  // PolymorphicProcedure<void (T), void()>:
    template<typename T>
    void operator()(T&& e) const {
      handle_exception(std::forward<T>(e));
      throw;
    }
    void operator()() const {
      handle_exception();
      throw;
    }
  };

  /// Polymorphic function that maps an exception type to an arbitrary value.
  ///
  /// Handled exceptions are std::exception and boost::exception. If the exception
  /// is unknown (in the `catch (...)` case for example), you can call the nullary
  /// overload.
  ///
  /// Example: Turns a function that throws exceptions into a mathematical total function.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// exception_value<int> code{std_code, boost_code, unknown_code};
  /// int res = invoke_catch(code, do_stuff, arg0, arg1)
  /// // test res for errors
  /// // ...
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// There is no constraint on T.
  template<typename T>
  struct exception_value {
    T std_value, boost_value, unknown_value;
  // Regular (if T is):
    KA_GENERATE_FRIEND_REGULAR_OPS_3(exception_value, std_value, boost_value, unknown_value)
  // PolymorphicFunction<T (std::exception), T (boost::exception), T ()>:
    T operator()(std::exception const&) const {
      return std_value;
    }
    T operator()(boost::exception const&) const {
      return boost_value;
    }
    T operator()() const {
      return unknown_value;
    }
  };

  /// Polymorphic function that maps an exception to its message.
  ///
  /// Handled exceptions are std::exception and boost::exception. If the exception
  /// is unknown (in the `catch (...)` case for example), you can call the nullary
  /// overload.
  ///
  /// Example: Transforming a function throwing exceptions into a total function
  ///   returning error messages.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // Instead of throwing exceptions, we want the function to return the
  /// // potential error message.
  ///
  /// auto f = []() {my_function(); return boost::optional<std::string>{};};
  ///
  /// if (auto const error = invoke_catch(exception_message{}, f)) {
  ///   // handle error
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct exception_message {
    static char const* unknown_error() {
      return "unknown error";
    }
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(exception_message)
  // PolymorphicFunction<std::string (std::exception), std::string (boost::exception), std::string ()>:
    std::string operator()(std::exception const& e) const {
      return e.what();
    }
    std::string operator()(boost::exception const& e) const {
      return boost::diagnostic_information_what(e);
    }
    std::string operator()() const {
      return unknown_error();
    }
  };

  /// Invokes a procedure in a try-catch clause and delegates exception handling
  /// to a dedicated handler.
  ///
  /// std::exceptions and boost::exceptions are passed as-is to the handler.
  /// In all other cases, the handler is called without parameter.
  ///
  /// This function can be used for example to log the exception and rethrow, to
  /// translate the exception to a value (useful to compose functions in a
  /// purely functional manner), etc.
  ///
  /// See exception_value, handle_exception_rethrow.
  ///
  /// Note: The exception types are hardcoded instead of specifying them via
  ///   a variadic list because, in a variadic template approach catching the
  ///   exceptions E0, E1 and any exception would generate this kind of code:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// try {
  ///   try {
  ///     try {
  ///       return f(args...);
  ///     } catch (E0 const& e0) {
  ///       return handle_exception(e0);
  ///     }
  ///   } catch (E1 const& e1) {
  ///     return handle_exception(e1);
  ///   }
  /// } catch (...) {
  ///   return handle_exception();
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///   which is not equivalent to this handwritten code:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///  try {
  ///    return f(args...);
  ///  } catch (E0 const& e0) {
  ///    return handle_exception(e0);
  ///  } catch (E1 const& e1) {
  ///    return handle_exception(e1);
  ///  } catch (...) {
  ///    return handle_exception();
  ///  }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///   because if the exception handler rethrows the exception, the "catch all"
  ///   clause will always be called.
  ///   So it seems, there is no way in C++ to specify in a compile-time way a
  ///   list of exceptions to catch...
  ///
  /// PolymorphicProcedure<Ret (std::exception const&), Ret (boost::exception const&), Ret ()> Proc,
  /// Procedure<Ret (Args...)> F
  template<typename Proc, typename F, typename... Args>
  auto invoke_catch(Proc&& handle_exception, F&& f, Args&&... args)
      // TODO: Remove this when migrating to C++14.
      -> decltype(fwd<F>(f)(fwd<Args>(args)...)) {
    try {
      return fwd<F>(f)(fwd<Args>(args)...);
    } catch (std::exception const& e) {
      return fwd<Proc>(handle_exception)(e);
    } catch (boost::exception const& e) {
      return fwd<Proc>(handle_exception)(e);
    } catch (...) {
      return fwd<Proc>(handle_exception)();
    }
  }

} // namespace ka

#endif // KA_ERRORHANDLING_HPP
