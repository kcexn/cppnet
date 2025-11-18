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
 * @file cppnet.hpp
 * @brief This file exports the public cppnet interface.
 */
#pragma once
#ifndef CPPNET_HPP
#define CPPNET_HPP
/** @brief This is the root namespace of cppnet. */
namespace net {}                         // namespace net
#include "service/async_context.hpp"     // IWYU pragma: export
#include "service/async_tcp_service.hpp" // IWYU pragma: export
#include "service/async_udp_service.hpp" // IWYU pragma: export
#include "service/context_thread.hpp"    // IWYU pragma: export
#include "timers/interrupt.hpp"          // IWYU pragma: export
#include "timers/timers.hpp"             // IWYU pragma: export
#endif                                   // CPPNET_HPP
