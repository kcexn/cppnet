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
 * @file interrupt_impl.hpp
 * @brief This file defines the interrupt sources.
 */
#pragma once
#ifndef CPPNET_INTERRUPT_IMPL_HPP
#define CPPNET_INTERRUPT_IMPL_HPP
#include "net/timers/interrupt.hpp"
/** @brief This namespace is for timers and interrupts. */
namespace net::timers {

inline auto socketpair_interrupt_source_t::interrupt() const noexcept -> void
{
  using namespace io::socket;
  static constexpr auto buf = std::array<char, 1>{'x'};
  static const auto msg = socket_message<sockaddr_in>{.buffers = buf};

  ::io::sendmsg(sockets[1], msg, MSG_NOSIGNAL);
}

template <InterruptSource Interrupt>
inline auto interrupt<Interrupt>::operator()() const noexcept -> void
{
  Interrupt::interrupt();
}

} // namespace net::timers

#endif // CPPNET_INTERRUPT_IMPL_HPP
