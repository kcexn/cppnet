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
 * @file with_lock.hpp
 * @brief This file defines with_lock.
 */
#pragma once
#ifndef CPPNET_WITH_LOCK_HPP
#define CPPNET_WITH_LOCK_HPP
#include "concepts.hpp"

#include <mutex>
/** @brief This namespace provides internal cppnet implementation details. */
namespace net::detail {
/**
 * @brief Runs the supplied functor while holding the acquired lock.
 * @tparam Lock A type that satisfies the BasicLockable named requirement.
 * @tparam Fn The functor type.
 * @param mtx The lock for thread-safety.
 * @param func The functor to invoke.
 * @returns The return value of func.
 */
template <BasicLockable Lock, typename Fn>
  requires std::is_invocable_v<Fn>
auto with_lock(Lock &mtx, Fn &&func) -> decltype(auto)
{
  auto guard = std::lock_guard(mtx);
  return std::forward<Fn>(func)();
}

} // namespace net::detail
#endif // CPPNET_WITH_LOCK_HPP
