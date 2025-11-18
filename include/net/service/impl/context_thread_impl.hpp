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
auto context_thread<Service>::stop() noexcept -> void
{
  auto socket = timers.sockets[1];
  timers.sockets[1] = timers.INVALID_SOCKET;
  if (socket != timers.INVALID_SOCKET)
    io::socket::close(socket);
  state = STOPPED;
}

template <ServiceLike Service>
template <typename... Args>
auto context_thread<Service>::start(Args &&...args) -> void
{
  auto lock = std::lock_guard{mtx_};
  if (started_)
    throw std::invalid_argument("context_thread can't be started twice.");

  server_ = std::thread([&]() noexcept {
    using namespace detail;
    using namespace io::socket;
    using namespace std::chrono;

    auto service = Service{std::forward<Args>(args)...};
    auto &sockets = timers.sockets;
    if (!socketpair(AF_UNIX, SOCK_STREAM, 0, sockets.data()))
    {
      const auto token = scope.get_stop_token();

      isr(poller.emplace(sockets[0]), [&]() noexcept {
        auto sigmask_ = sigmask.exchange(0);
        for (int signum = 0; auto mask = (sigmask_ >> signum); ++signum)
        {
          if (mask & (1 << 0))
            service.signal_handler(signum);
        }

        if (sigmask_ & (1 << terminate))
        {
          scope.request_stop();
          timers.add(
              seconds(1),
              [&](timers::timer_id) { service.signal_handler(terminate); },
              seconds(1));
        }

        return !token.stop_requested();
      });

      service.start(static_cast<async_context &>(*this));
      state = STARTED;

      if (token.stop_requested())
      {
        state = STOPPED;
        signal(terminate);
      }

      state.notify_all();
      run();
    }

    stop();
    state.notify_all();
  });

  started_ = true;
}

template <ServiceLike Service> context_thread<Service>::~context_thread()
{
  if (!started_)
    return;

  signal(terminate);
  server_.join();
}
} // namespace net::service
#endif // CPPNET_CONTEXT_THREAD_IMPL_HPP
