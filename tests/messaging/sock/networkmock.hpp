#pragma once
#ifndef _QI_TESTS_MESSAGING_NETWORKMOCK_HPP
#define _QI_TESTS_MESSAGING_NETWORKMOCK_HPP
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <chrono>
#include <boost/shared_ptr.hpp>
#include <qi/messaging/sock/traits.hpp>
#include <qi/messaging/sock/error.hpp>
#include <qi/messaging/sock/common.hpp>
#include <ka/moveoncopy.hpp>
#include <ka/utility.hpp>
#include <ka/macroregular.hpp>

/// @file
/// Contains the implementation of the Network concept for unit tests.
///
/// See qi/messaging/sock/concept.hpp

/// Contains mock types for unit tests.
namespace mock
{
  /// Models in the simplest possible way the Network concept.
  struct Network
  {
    struct socket_option_no_delay_type
    {
      bool value;
    };
    struct accept_option_reuse_address_type
    {
      bool value;
    };
    struct error_code_type
    {
      enum value_type
      {
        success = 0,
        badAddress,
        networkUnreachable,
        hostNotFound,
        ownerDead,
        operationAborted,
        sslErrors,
        connectionRefused,
        fault,
        messageSize,
        shutdown,
        unknown
      } _value;
      std::string _message;
      error_code_type(value_type c = success) : _value(c) {}
      explicit operator bool() const {return _value != success;}
      value_type value() const {return _value;}
      std::string message() const
      {
        switch (_value)
        {
        case success: return "success";
        case badAddress: return "Bad address";
        case networkUnreachable: return "networkUnreachable";
        case hostNotFound: return "hostNotFound";
        case ownerDead: return "ownerDead";
        case operationAborted: return "operationAborted";
        case sslErrors: return "sslErrors";
        case connectionRefused: return "connectionRefused";
        case fault: return "fault";
        case messageSize: return "messageSize";
        case shutdown: return "shutdown";
        case unknown: return "unknown";
        }
        throw std::runtime_error("error_code_type::message(): unknown code.");
      }
      KA_GENERATE_FRIEND_REGULAR_OPS_2(error_code_type, _value, _message)
    };
    struct io_service_type
    {
      mutable qi::Strand _strand;

      template<typename Proc>
      auto wrap(Proc&& p) const
        -> decltype(ka::compose(ka::constant_function(), _strand.schedulerFor(ka::fwd<Proc>(p))))
      {
        // If `Proc`'s return type is `R`, then `_strand.schedulerFor(p)` returns a function object that
        // returns a `qi::Future<R>`. But this `wrap` method must return a function object that returns `void`
        // (see `NetIoService` concept).
        // So we compose the stranded procedure with a procedure that does nothing and returns `void` (namely
        // `PolymorphicConstantFunction<void>{}`).
        return ka::compose(ka::constant_function(), _strand.schedulerFor(ka::fwd<Proc>(p)));
      }
    };
    struct ssl_context_type
    {
      enum class method {sslv23};
      method m;
      ssl_context_type() = default;        // The 2 ctors are to avoid the
      ssl_context_type(method m) : m(m) {} // "not initialized" warning about m.
    };
    struct ssl_verify_mode_type
    {
    };
    struct _endpoint
    {
      struct _address
      {
        bool _isV6;
        std::string _value;
        _address(bool b = false, std::string v = "") : _isV6(b), _value(v) {}
        bool is_v6() const {return _isV6;}
        std::string to_string() const {return _value;}
        KA_GENERATE_FRIEND_REGULAR_OPS_2(_address, _isV6, _value)
      } _addr;
      _endpoint() = default;
      _address address() const {return _addr;}
      unsigned short port() const {return 0u;}
      struct protocol_t {};
      protocol_t protocol() const {return {};}
      KA_GENERATE_FRIEND_REGULAR_OPS_1(_endpoint, _addr)
    };
    struct _resolver_entry
    {
      _endpoint _e;
      _endpoint endpoint() const {return _e;}
      KA_GENERATE_FRIEND_REGULAR_OPS_1(_resolver_entry, _e)
    };
    using _anyHandler = std::function<void (error_code_type)>;
    struct ssl_socket_type
    {
      enum class handshake_type
      {
        client,
        server
      };
      struct lowest_layer_type
      {
        enum class shutdown_type {shutdown_both};
        static const int max_connections = 42;
        using endpoint_type = _endpoint;
        void set_option(socket_option_no_delay_type) {}

        using _anyAsyncConnecter = std::function<void (_resolver_entry, _anyHandler)>;
        static _anyAsyncConnecter async_connect;

        using _anyCanceler = std::function<void ()>;
        static _anyCanceler cancel;

        using _anyShutdowner = std::function<void (shutdown_type, error_code_type&)>;
        static _anyShutdowner _shutdown;

        static inline void shutdown(shutdown_type type, error_code_type& erc)
        {
          _shutdown(type, erc);
        }

        static inline void shutdown(shutdown_type type)
        {
          error_code_type erc;
          shutdown(type, erc);
          if (erc) throw std::runtime_error(erc.message());
        }

        using _anyCloser = std::function<void (error_code_type&)>;
        static _anyCloser close;

        using _native_handle = int;
        _native_handle native_handle() {return {};}

        _endpoint _e;
        _endpoint remote_endpoint() const {return _e;}
      };
      io_service_type* _io;
      ssl_socket_type(io_service_type& io, ssl_context_type) : _io(&io) {}
      io_service_type& get_io_service() {return *_io;}
      void set_verify_mode(ssl_verify_mode_type) {}

      using _anyAsyncHandshaker = std::function<void (handshake_type, _anyHandler)>;
      static _anyAsyncHandshaker async_handshake;

      lowest_layer_type _lowest_layer;
      lowest_layer_type& lowest_layer() {return _lowest_layer;}

      struct next_layer_type {} _next_layer;
      next_layer_type& next_layer() {return _next_layer;}
    };
    struct acceptor_type
    {
      acceptor_type(io_service_type& io) : _io(io) {}
      io_service_type& _io;
      using _anyAsyncAccepter = std::function<void (ssl_socket_type::next_layer_type&, _anyHandler)>;

      io_service_type& get_io_service() {return _io;}
      void open(_endpoint::protocol_t) {}
      bool is_open() const {return true;}
      void set_option(accept_option_reuse_address_type) {}
      void bind(_endpoint) {}
      void listen(int, error_code_type&) {}
      void close(error_code_type&) {}
      static _anyAsyncAccepter async_accept;
      _endpoint local_endpoint(error_code_type&) const { return {}; }
    };
    struct resolver_type
    {
      struct query
      {
        enum flags {all_matching};
        std::string _host, _port;
        flags _flags;
        query(std::string host, std::string port, flags f = all_matching)
          : _host(host), _port(port), _flags(f)
        {
        }
      };
      struct iterator
      {
        using value_type = _resolver_entry;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        static _resolver_entry* _sentinel;
        _resolver_entry** _p;
        iterator(_resolver_entry** p = &_sentinel) : _p{p} {}
        value_type operator*() {return **_p;}
        friend value_type src(iterator x) {return *x;}
        iterator& operator++() {++_p; return *this;}
        bool operator==(iterator b) const {return _p == b._p || (!*_p && !*b._p);}
        bool operator!=(iterator b) const {return !(*this == b);}
      };
      io_service_type& _io;
      resolver_type(io_service_type& io) : _io(io) {}

      using _anyResolveHandler = std::function<void (error_code_type, iterator)>;
      using _anyAsyncResolver = std::function<void (query, _anyResolveHandler)>;
      static _anyAsyncResolver async_resolve;
      void cancel() {}
      io_service_type& get_io_service() {return _io;}
    };

    static io_service_type& defaultIoService()
    {
      static qi::sock::IoService<mock::Network> io;
      return io;
    }

    static ssl_verify_mode_type sslVerifyNone()
    {
      return {};
    }

    using H = ssl_socket_type::lowest_layer_type::_native_handle;
    static void setSocketNativeOptions(H, int) {}

    struct _mutable_buffer_sequence
    {
      unsigned char *begin, *end;
    };
    struct _const_buffer_sequence
    {
      const unsigned char *begin, *end;
    };
    static _mutable_buffer_sequence buffer(void* data, std::size_t maxSizeInBytes)
    {
      auto b = (unsigned char*)data;
      return {b, b + maxSizeInBytes};
    }
    static _const_buffer_sequence buffer(const void* data, std::size_t maxSizeInBytes)
    {
      auto b = (const unsigned char*)data;
      return {b, b + maxSizeInBytes};
    }

    using const_buffer_type = _const_buffer_sequence;
    using _anyTransferHandler = std::function<void (error_code_type, std::size_t)>;

    template<typename NetSslSocket>
    using _anyAsyncReaderSocket = std::function<void (NetSslSocket&, _mutable_buffer_sequence, _anyTransferHandler)>;

    template<typename NetSslSocket>
    using _anyAsyncWriterSocket = std::function<void (NetSslSocket&, const std::vector<_const_buffer_sequence>&, _anyTransferHandler)>;

    template<typename NetSslSocket>
    struct SocketFunctions
    {
      static _anyAsyncReaderSocket<NetSslSocket> _async_read_socket;
      static _anyAsyncWriterSocket<NetSslSocket> _async_write_socket;
    };

    template<typename NetTransferHandler, typename NetSslSocket>
    static void async_read(NetSslSocket& s, _mutable_buffer_sequence b, NetTransferHandler h)
    {
      SocketFunctions<NetSslSocket>::_async_read_socket(s, b, h);
    }

    template<typename NetSslSocket, typename NetTransferHandler>
    static void async_write(NetSslSocket& s, const std::vector<_const_buffer_sequence>& b, NetTransferHandler h)
    {
      SocketFunctions<NetSslSocket>::_async_write_socket(s, b, h);
    }

    using _anyAsyncReaderNextLayer = std::function<void (ssl_socket_type::next_layer_type&, _mutable_buffer_sequence, _anyTransferHandler)>;
    static _anyAsyncReaderNextLayer _async_read_next_layer;

    template<typename NetTransferHandler>
    static void async_read(ssl_socket_type::next_layer_type& s, _mutable_buffer_sequence b, NetTransferHandler h)
    {
      _async_read_next_layer(s, b, h);
    }

    using _anyAsyncWriterNextLayer = std::function<void (ssl_socket_type::next_layer_type&, const std::vector<_const_buffer_sequence>&, _anyTransferHandler)>;
    static _anyAsyncWriterNextLayer _async_write_next_layer;

    template<typename NetTransferHandler>
    static void async_write(ssl_socket_type::next_layer_type& s, const std::vector<_const_buffer_sequence>& b, NetTransferHandler h)
    {
      _async_write_next_layer(s, b, h);
    }
  };

} // namespace mock

namespace qi {
namespace sock {

template<>
inline ErrorCode<mock::Network> success<ErrorCode<mock::Network>>()
{
  return {};
}

template<>
inline ErrorCode<mock::Network> badAddress<ErrorCode<mock::Network>>()
{
  return {ErrorCode<mock::Network>::badAddress};
}

template<>
inline ErrorCode<mock::Network> networkUnreachable<ErrorCode<mock::Network>>()
{
  return {ErrorCode<mock::Network>::networkUnreachable};
}

template<>
inline ErrorCode<mock::Network> ownerDead<ErrorCode<mock::Network>>()
{
  return {ErrorCode<mock::Network>::ownerDead};
}

template<>
inline ErrorCode<mock::Network> operationAborted<ErrorCode<mock::Network>>()
{
  return {ErrorCode<mock::Network>::operationAborted};
}

template<>
inline ErrorCode<mock::Network> hostNotFound<ErrorCode<mock::Network>>()
{
  return {ErrorCode<mock::Network>::hostNotFound};
}

template<>
inline ErrorCode<mock::Network> fault<ErrorCode<mock::Network>>()
{
  return {ErrorCode<mock::Network>::fault};
}

template<>
inline ErrorCode<mock::Network> messageSize<ErrorCode<mock::Network>>()
{
  return {ErrorCode<mock::Network>::messageSize};
}

template<>
inline ErrorCode<mock::Network> connectionRefused<ErrorCode<mock::Network>>()
{
  return {ErrorCode<mock::Network>::connectionRefused};
}

template<>
inline ErrorCode<mock::Network> shutdown<ErrorCode<mock::Network>>()
{
  return {ErrorCode<mock::Network>::shutdown};
}

}} // namespace qi::sock

namespace mock
{
  using N = Network;
  using S = qi::sock::SslSocket<N>;
  using _LowestLayer = N::ssl_socket_type::lowest_layer_type;

  inline void defaultAsyncResolve(N::resolver_type::query q, N::resolver_type::_anyResolveHandler h)
  {
    std::thread{[=] {
      static N::_resolver_entry entryIpV4{{{false, q._host}}};
      static N::_resolver_entry entryIpV6{{{true, q._host}}};
      static N::_resolver_entry* a[] = {&entryIpV4, &entryIpV6, nullptr};
      h(qi::sock::success<N::error_code_type>(), N::resolver_type::iterator{a});
    }}.join();
  }

  inline void defaultAsyncConnect(N::_resolver_entry, N::_anyHandler h)
  {
    std::thread{[=] {
      h(N::error_code_type{});
    }}.join();
  }

  inline void defaultCancel()
  {
  }

  inline void defaultShutdown(_LowestLayer::shutdown_type, N::error_code_type&)
  {
  }

  inline void defaultClose(N::error_code_type&)
  {
  }

  inline void defaultAsyncHandshake(N::ssl_socket_type::handshake_type, N::_anyHandler h)
  {
    std::thread{[=] {
      h(N::error_code_type{});
    }}.join();
  }

  inline void defaultAsyncAccept(N::ssl_socket_type::next_layer_type&, N::_anyHandler h)
  {
    std::thread{[=] {
      h(N::error_code_type{});
    }}.join();
  }

  template<typename NetSslSocket>
  inline void defaultAsyncReadSocket(NetSslSocket&, N::_mutable_buffer_sequence b, N::_anyTransferHandler h)
  {
    std::thread{[=] {
      h(N::error_code_type{}, 0u);
    }}.detach();
  }

  inline void defaultAsyncReadNextLayer(N::ssl_socket_type::next_layer_type&, N::_mutable_buffer_sequence b, N::_anyTransferHandler h)
  {
    std::thread{[=] {
      h(N::error_code_type{}, 0u);
    }}.detach();
  }

  template<typename NetSslSocket>
  inline void defaultAsyncWriteSocket(NetSslSocket&, const std::vector<N::_const_buffer_sequence>&, N::_anyTransferHandler h)
  {
    std::thread{[=] {
      h(N::error_code_type{}, 0u);
    }}.detach();
  }

  inline void defaultAsyncWriteNextLayer(N::ssl_socket_type::next_layer_type&, const std::vector<N::_const_buffer_sequence>&, N::_anyTransferHandler h)
  {
    std::thread{[=] {
      h(N::error_code_type{}, 0u);
    }}.detach();
  }

} // namespace mock

#endif // _QI_TESTS_MESSAGING_NETWORKMOCK_HPP
