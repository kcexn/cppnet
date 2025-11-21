// Copyright 2025 Kevin Exton
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// NOLINTBEGIN
#pragma once
#include <exception>
#ifndef CPPNET_TEST_TCP_CLIENT_FIXTURE_HPP
#define CPPNET_TEST_TCP_CLIENT_FIXTURE_HPP
#include "net/service/context_thread.hpp"
#include "test_tcp_fixture.hpp"

#include <gtest/gtest.h>
#include <io/io.hpp>

#include <netdb.h>

using namespace net::service;

struct tcp_echo_clients {
  /** @brief The sender for the asynchronous operations. */
  struct sender {
    using sender_concept = stdexec::sender_t;
    using completion_signatures =
        stdexec::completion_signatures<stdexec::set_value_t(std::string),
                                       stdexec::set_error_t(std::error_code)>;

    template <typename Receiver> struct state {
      using socket_dialog = async_context::socket_dialog;

      auto send_message(const socket_dialog &sock) noexcept -> void
      {
        using namespace stdexec;
        try
        {
          using namespace io::socket;
          using stdexec::sender;

          auto msg = socket_message<sockaddr_in>{.buffers = message};
          sender auto echo =
              io::sendmsg(sock, msg, 0) |
              let_value([&, sock, msg](auto) mutable {
                return io::recvmsg(sock, msg, 0);
              }) |
              let_value([&](auto) mutable noexcept {
                set_value(std::move(receiver), std::move(message));
                return just();
              }) |
              let_error([&, sock](auto &&error) mutable noexcept {
                if constexpr (std::is_same_v<std::decay_t<decltype(error)>,
                                             std::exception_ptr>)
                {
                  try
                  {
                    std::rethrow_exception(
                        std::forward<decltype(error)>(error));
                  }
                  catch (const std::bad_alloc &)
                  {
                    set_error(
                        std::move(receiver),
                        std::make_error_code(std::errc::not_enough_memory));
                  }
                  catch (...)
                  {
                    set_error(
                        std::move(receiver),
                        std::make_error_code(std::errc::state_not_recoverable));
                  }
                }
                else
                {
                  set_error(std::move(receiver),
                            std::forward<decltype(error)>(error));
                }
                return just();
              });
          ctx->scope.spawn(std::move(echo));
        }
        catch (const std::bad_alloc &)
        {
          set_error(std::move(receiver),
                    std::make_error_code(std::errc::not_enough_memory));
        }
        catch (...)
        {
          set_error(std::move(receiver),
                    std::make_error_code(std::errc::state_not_recoverable));
        }
      }

      auto try_connect(struct addrinfo *result,
                       struct addrinfo *rp) noexcept -> void
      {
        using namespace stdexec;
        try
        {
          using stdexec::sender;

          if (rp == nullptr)
          {
            return set_error(
                std::move(receiver),
                std::make_error_code(std::errc::address_not_available));
          }

          auto sock = ctx->poller.emplace(rp->ai_family, rp->ai_socktype,
                                          rp->ai_protocol);
          dst = std::span(reinterpret_cast<std::byte *>(rp->ai_addr),
                          rp->ai_addrlen);
          sender auto conn = io::connect(sock, dst) |
                             then([&, result, sock](auto) noexcept {
                               freeaddrinfo(result);
                               send_message(sock);
                             }) |
                             upon_error([&, result, rp](auto) noexcept {
                               return try_connect(result, rp->ai_next);
                             });
          ctx->scope.spawn(std::move(conn));
        }
        catch (const std::bad_alloc &)
        {
          set_error(std::move(receiver),
                    std::make_error_code(std::errc::not_enough_memory));
        }
        catch (...)
        {
          set_error(std::move(receiver),
                    std::make_error_code(std::errc::state_not_recoverable));
        }
      }

      auto start() noexcept -> void
      {
        using namespace io::socket;

        auto service = std::array<char, 6>();
        std::to_chars(service.begin(), service.end(), port);

        struct addrinfo hints = {.ai_family = AF_INET,
                                 .ai_socktype = SOCK_STREAM,
                                 .ai_protocol = IPPROTO_TCP};
        struct addrinfo *result = nullptr;
        getaddrinfo(hostname, service.data(), &hints, &result);

        // try_connect initiates the echo message.
        try_connect(result, result);
        // interrupt() wakes up the context event loop so that it can receive
        // a new client operation.
        ctx->interrupt();
      }

      /** @brief The message to send to the echo server. */
      std::string message;
      /** @brief The destination socket address. */
      io::socket::socket_address<sockaddr_in6> dst;
      /** @brief The hostname of the intended echo server. */
      const char *hostname;
      /** @brief The asynchronous context. */
      async_context *ctx;
      /** @brief The receiver to send the final value to. */
      Receiver receiver;
      /** @brief The destination port number of the intended echo server. */
      unsigned short port;
    };

    template <typename Receiver>
    auto connect(Receiver &&receiver) noexcept -> state<Receiver>
    {
      return {.message = std::move(message),
              .dst = {},
              .hostname = hostname,
              .ctx = ctx,
              .receiver = std::forward<Receiver>(receiver),
              .port = port};
    }

    /** @brief The message to send to the echo server. */
    std::string message;
    /** @brief The hostname of the intended echo server. */
    const char *hostname;
    /** @brief The asynchronous context. */
    async_context *ctx;
    /** @brief The destination port number of the intended echo server. */
    unsigned short port;
  };

  /** @brief A TCP echo client. */
  struct client {
    /** @brief The hostname of the intended echo server. */
    std::string hostname;
    /** @brief Asynchronous context. */
    async_context *ctx;
    /** @brief The destination port number of the intended echo server. */
    unsigned short port;

    auto send(std::string message) const noexcept -> sender
    {
      return {.message = std::move(message),
              .hostname = hostname.c_str(),
              .ctx = ctx,
              .port = port};
    }
  };

  auto make_client(async_context &ctx, std::string hostname = {},
                   unsigned short port = 0) const noexcept -> client
  {
    return {.hostname = std::move(hostname),
            .ctx = std::addressof(ctx),
            .port = port};
  }
};

class AsyncTcpEchoClientTests : public ::testing::Test {
protected:
  using clients_type = tcp_echo_clients;
  using client_context = context_thread;
  using server_type = basic_context_thread<tcp_echo_service>;

  template <typename T> using socket_address = io::socket::socket_address<T>;

  auto SetUp() -> void override
  {
    client_ctx = std::make_unique<client_context>();
    clients_v4 = std::make_unique<clients_type>();
    server_v4 = std::make_unique<server_type>();

    unsigned short min_port = 6000;
    unsigned short port = min_port + (std::rand() % (UINT16_MAX - min_port));

    addr_v4->sin_family = AF_INET;
    addr_v4->sin_port = htons(port);

    server_v4->start(addr_v4);
    client_ctx->start();
  };

  std::unique_ptr<clients_type> clients_v4;

  socket_address<sockaddr_in> addr_v4;
  std::unique_ptr<server_type> server_v4;
  std::unique_ptr<client_context> client_ctx;
};

#endif // CPPNET_TEST_TCP_CLIENT_FIXTURE_HPP
// NOLINTEND
