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
/**
 * @brief A threaded asynchronous service.
 *
 * This class runs the provided service in a separate thread
 * with an asynchronous context.
 *
 * @tparam Service The service to run.
 */
template <ServiceLike Service> class context_thread : public async_context {
public:
  /** @brief Default constructor. */
  context_thread() = default;
  /** @brief Deleted copy constructor. */
  context_thread(const context_thread &) = delete;
  /** @brief Deleted move constructor. */
  context_thread(context_thread &&) = delete;
  /** @brief Deleted copy assignment. */
  auto operator=(const context_thread &) -> context_thread & = delete;
  /** @brief Deleted move assignment. */
  auto operator=(context_thread &&) -> context_thread & = delete;

  /**
   * @brief Start the asynchronous service.
   * @details This starts the provided service in a separate thread
   * with the provided asynchronous context.
   * @tparam Args Argument types for constructing the Service.
   * @param args The arguments to forward to the Service constructor.
   */
  template <typename... Args> auto start(Args &&...args) -> void;

  /** @brief The destructor signals the thread before joining it. */
  ~context_thread();

private:
  /** @brief The thread that serves the asynchronous service. */
  std::thread server_;
  /** @brief Mutex for thread-safety. */
  std::mutex mtx_;
  /** @brief Flag that guards against starting a thread twice. */
  bool started_{false};

  /** @brief Called when the async_service is stopped. */
  auto stop() noexcept -> void;
};

} // namespace net::service

#include "impl/context_thread_impl.hpp" // IWYU pragma: export

#endif // CPPNET_CONTEXT_THREAD_HPP
