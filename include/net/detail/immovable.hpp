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
 * @file immovable.hpp
 * @brief This file defines immovable.
 */
#pragma once
#ifndef CPPNET_IMMOVABLE_HPP
#define CPPNET_IMMOVABLE_HPP
/** @brief This namespace provides internal cppnet implementation details. */
namespace net::detail {
/**
 * @brief This struct can be used as a base class to make derived
 *        classes immovable.
 */
struct immovable {
  /** @brief Default constructor. */
  immovable() = default;
  /** @brief Deleted copy constructor. */
  immovable(const immovable &) = delete;
  /** @brief Deleted move constructor. */
  immovable(immovable &&) = delete;
  /** @brief Deleted copy assignment. */
  auto operator=(const immovable &) -> immovable & = delete;
  /** @brief Deleted move assignment. */
  auto operator=(immovable &&) -> immovable & = delete;
  /** @brief Default destructor. */
  ~immovable() = default;
};
} // namespace net::detail
#endif // CPPNET_IMMOVABLE_HPP
