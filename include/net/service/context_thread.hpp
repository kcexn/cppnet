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
 * @file context_thread.hpp
 * @brief This file declares an asynchronous service.
 */
#pragma once
#ifndef CPPNET_CONTEXT_THREAD_HPP
#define CPPNET_CONTEXT_THREAD_HPP
#include "async_context.hpp"

#include <mutex>
#include <thread>
/** @brief This namespace is for network services. */
namespace net::service {
/** @brief internal service implementation details. */
namespace detail {
/** @brief Null service that services as a ServiceLike placeholder. */
struct null_service {
  /**
   * @brief signal handling placeholder.
   * @param signum The signal number to handle.
   */
  static auto signal_handler(int signum) noexcept -> void {};
  /**
   * @brief start placeholder.
   * @param ctx The asynchronous context to start on.
   * @returns A successful error code.
   */
  static auto start(async_context &ctx) noexcept -> std::error_code
  {
    return {};
  };
};
} // namespace detail

/**
 * @brief A threaded asynchronous service.
 *
 * This class runs the provided service in a separate thread
 * with an asynchronous context.
 *
 * @tparam Service The service to run.
 */
template <ServiceLike Service>
class basic_context_thread : public async_context {
public:
  /** @brief Default constructor. */
  basic_context_thread() = default;
  /** @brief Deleted copy constructor. */
  basic_context_thread(const basic_context_thread &) = delete;
  /** @brief Deleted move constructor. */
  basic_context_thread(basic_context_thread &&) = delete;
  /** @brief Deleted copy assignment. */
  auto
  operator=(const basic_context_thread &) -> basic_context_thread & = delete;
  /** @brief Deleted move assignment. */
  auto operator=(basic_context_thread &&) -> basic_context_thread & = delete;

  /**
   * @brief Start the asynchronous service.
   * @details This starts the provided service in a separate thread
   * with the provided asynchronous context.
   * @tparam Args Argument types for constructing the Service.
   * @param args The arguments to forward to the Service constructor.
   */
  template <typename... Args> auto start(Args &&...args) -> void;

  /** @brief The destructor signals the thread before joining it. */
  ~basic_context_thread();

private:
  /** @brief The thread that serves the asynchronous service. */
  std::thread server_;
  /** @brief Mutex for thread-safety. */
  std::mutex mtx_;

  /** @brief Called when the async_service is stopped. */
  auto stop() noexcept -> void;
};

/**
 * @brief Default context thread is a null service thread.
 * @details The null service thread implements a service which does
 * nothing. A null service context_thread is used for starting an
 * asynchronous context in its own thread and adding services to it
 * manually. A null service context_thread is useful for implementing
 * asynchronous network clients.
 */
using context_thread = basic_context_thread<detail::null_service>;

} // namespace net::service

#include "impl/context_thread_impl.hpp" // IWYU pragma: export

#endif // CPPNET_CONTEXT_THREAD_HPP
