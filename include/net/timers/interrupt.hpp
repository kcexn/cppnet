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
 * @file interrupt.hpp
 * @brief This file declares interrupt related types..
 */
#pragma once
#ifndef CPPNET_INTERRUPT_HPP
#define CPPNET_INTERRUPT_HPP
#include "net/detail/concepts.hpp"

#include <io/io.hpp>
/** @brief This namespace is for timers and interrupts. */
namespace net::timers {
/** @brief A socketpair interrupt source. */
struct socketpair_interrupt_source_t {
  /** @brief The native socket type. */
  using socket_type = io::socket::native_socket_type;
  /** @brief The invalid socket constant. */
  static constexpr auto INVALID_SOCKET = io::socket::INVALID_SOCKET;
  /** @brief The socket pair. */
  std::array<socket_type, 2> sockets{INVALID_SOCKET, INVALID_SOCKET};
  /**
   * @brief The interrupt method.
   * @details This method is needed to comply with the InterruptSource
   * concept.
   */
  inline auto interrupt() const noexcept -> void;
};

/**
 * @brief An interrupt is an immediately run timer event.
 * @tparam Interrupt An interrupt source tag compliant with the InterruptSoruce
 * concept.
 * @details Interrupts are used to awaken sleeping event-loops.
 */
template <InterruptSource Interrupt> struct interrupt : public Interrupt {
  /** @brief The underlying interrupt source type. */
  using interrupt_source_t = Interrupt;
  /** @brief Calls the underlying interrupt. */
  inline auto operator()() const noexcept -> void;
};

} // namespace net::timers

#include "impl/interrupt_impl.hpp" // IWYU pragma: export

#endif // CPPNET_INTERRUPT_HPP
