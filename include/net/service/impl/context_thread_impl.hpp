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

/**
 * @file context_thread_impl.hpp
 * @brief This file defines the asynchronous service.
 */
#pragma once
#ifndef CPPNET_CONTEXT_THREAD_IMPL_HPP
#define CPPNET_CONTEXT_THREAD_IMPL_HPP
#include "net/service/context_thread.hpp"

#include <stdexec/execution.hpp>
namespace net::service {
template <ServiceLike Service>
auto basic_context_thread<Service>::stop() noexcept -> void
{
  auto socket = std::exchange(timers.sockets[1], timers.INVALID_SOCKET);
  io::socket::close(socket);
  state = STOPPED;
  state.notify_all();
}

template <ServiceLike Service>
template <typename... Args>
auto basic_context_thread<Service>::start(Args &&...args) -> void
{
  auto lock = std::lock_guard{mtx_};
  if (state != PENDING)
    throw std::invalid_argument("context_thread already started");

  auto &sockets = timers.sockets;
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets.data()))
  {
    throw std::system_error({errno, std::system_category()},
                            "Failed to initialize socketpair");
  }

  auto error = std::error_code();
  server_ = std::thread([&] {
    auto service = Service{std::forward<Args>(args)...};
    const auto token = scope.get_stop_token();

    isr(poller.emplace(sockets[0]), [&] {
      using namespace std::chrono;

      auto sigmask_ = sigmask.exchange(0);
      for (int signum = 0; auto mask = (sigmask_ >> signum); ++signum)
      {
        if (mask & (1 << 0))
          service.signal_handler(signum);
      }

      if (sigmask_ & (1 << terminate))
      {
        scope.request_stop();
        timers.add(1s, [&](auto) { service.signal_handler(terminate); }, 1s);
      }

      return !token.stop_requested();
    });

    error = service.start(*this);
    if (error)
    {
      signal(terminate);
    }
    else
    {
      state = STARTED;
      state.notify_all();
    }

    run();
    stop();
  });

  state.wait(PENDING);
  if (error)
    throw std::system_error(error, "service failed to start");
}

template <ServiceLike Service>
basic_context_thread<Service>::~basic_context_thread()
{
  if (state > PENDING)
  {
    signal(terminate);
    server_.join();
  }
}
} // namespace net::service
#endif // CPPNET_CONTEXT_THREAD_IMPL_HPP
