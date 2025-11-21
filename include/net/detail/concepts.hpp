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
 * @file concepts.hpp
 * @brief This file defines concepts for cppnet.
 */
#pragma once
#ifndef CPPNET_CONCEPT_HPP
#define CPPNET_CONCEPT_HPP
#include <concepts>
#include <system_error>
// Forward declarations
namespace net::service {
struct async_context;
} // namespace net::service

/**
 * @namespace net
 * @brief The root namespace for all cppnet components.
 */
namespace net {
/** @brief A concept to validate the C++ BasicLockable requirement. */
template <typename Lock>
concept BasicLockable = requires(Lock lock) {
  { lock.lock() } -> std::same_as<void>;
  { lock.unlock() } -> std::same_as<void>;
};

/**
 * @brief Constrains types that behave like an application or service.
 * @details ServiceLike functions can mutate state in the asynchronous
 * context.
 * @note A ServiceLike may call `ctx.scope.request_stop()` any time it
 * runs.
 */
template <typename S>
concept ServiceLike = requires(S service, service::async_context &ctx) {
  { service.signal_handler(1) } noexcept -> std::same_as<void>;
  { service.start(ctx) } noexcept -> std::same_as<std::error_code>;
};

/** @brief This namespace is for timers and interrupts. */
namespace timers {
/** @brief A concept for constraining interrupt sources. */
template <typename Tag>
concept InterruptSource = requires(const Tag tag) {
  { tag.interrupt() } noexcept -> std::same_as<void>;
};
} // namespace timers.

} // namespace net
#endif // CPPNET_CONCEPT_HPP
