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

class AsyncContextTest : public ::testing::Test {};

TEST_F(AsyncContextTest, SignalTest)
{
  auto ctx = async_context{};

  int err = ::socketpair(AF_UNIX, SOCK_STREAM, 0, ctx.timers.sockets.data());
  ASSERT_EQ(err, 0);

  ctx.signal(ctx.terminate);

  auto buf = std::array<char, 5>();
  auto msg = io::socket::socket_message<sockaddr_in>{.buffers = buf};
  auto len = io::recvmsg(ctx.timers.sockets[0], msg, 0);
  EXPECT_EQ(len, 1);
}

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

TEST_F(AsyncContextTest, AsyncServiceTest)
{
  using enum async_context::context_states;

  auto service = basic_context_thread<test_service>();
  service.start();
  ASSERT_EQ(service.state, STARTED);

  service.signal(service.terminate);
  service.state.wait(STARTED);
  ASSERT_EQ(service.state, STOPPED);
}

TEST_F(AsyncContextTest, StartTwiceTest)
{
  using enum async_context::context_states;

  auto service = basic_context_thread<test_service>{};

  service.start();
  EXPECT_THROW(service.start(), std::invalid_argument);
  ASSERT_EQ(service.state, STARTED);

  service.signal(service.terminate);
  service.state.wait(STARTED);
  ASSERT_EQ(service.state, service.STOPPED);
}

TEST_F(AsyncContextTest, TestUser1Signal)
{
  using enum async_context::context_states;

  auto service = basic_context_thread<test_service>();

  service.start();
  ASSERT_EQ(service.state, STARTED);

  service.signal(service.user1);
  {
    auto lock = std::unique_lock{test_mtx};
    test_cv.wait(lock, [&] { return test_signal == service.user1; });
  }
  EXPECT_EQ(test_signal, service.user1);
}
// NOLINTEND
