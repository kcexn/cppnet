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

// NOLINTBEGIN
#include "net/service/context_thread.hpp"

#include <gtest/gtest.h>

#include <condition_variable>
#include <mutex>

using namespace net::service;

int socketpair(int domain, int type, int __protocol, int __fds[2])
{
  return -1;
}

class AsyncServiceTest : public ::testing::Test {};

std::mutex test_mtx;
std::condition_variable test_cv;
static int test_signal = 0;
static int test_started = 0;
struct test_service {
  auto signal_handler(int signum) noexcept -> void
  {
    std::lock_guard lock{test_mtx};
    test_signal = signum;
    test_cv.notify_all();
  }
  auto start(async_context &ctx) noexcept -> std::error_code
  {
    std::lock_guard lock{test_mtx};
    test_started = 1;
    test_cv.notify_all();
    return {};
  }
};

TEST_F(AsyncServiceTest, StartTest)
{
  using enum async_context::context_states;

  auto service = basic_context_thread<test_service>();

  EXPECT_THROW(service.start(), std::system_error);
}
// NOLINTEND
