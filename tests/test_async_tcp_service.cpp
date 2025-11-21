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
#include "test_tcp_fixture.hpp"
#include <atomic>
using namespace net::service;

TEST_F(AsyncTcpServiceTest, StartTest)
{
  service_v4->start(*ctx);
  service_v6->start(*ctx);
  ctx->signal(ctx->terminate);

  auto n = 0UL;
  while (ctx->poller.wait_for(50))
  {
    ASSERT_LE(n++, 4);
  }
  EXPECT_GT(n, 0);
}

TEST_F(AsyncTcpServiceTest, EchoTest)
{
  using namespace stdexec;

  service_v4->start(*ctx);
  service_v6->start(*ctx);

  {
    using namespace io;
    using namespace io::socket;

    auto sock_v4 = socket_handle(AF_INET, SOCK_STREAM, 0);
    auto sock_v6 = socket_handle(AF_INET6, SOCK_STREAM, 0);

    ASSERT_EQ(io::connect(sock_v4, addr_v4), 0);
    ASSERT_EQ(io::connect(sock_v6, addr_v6), 0);
    auto n = ctx->poller.wait_for(2000);
    ASSERT_GT(n, 0);

    auto buf = std::array<char, 1>{'x'};
    auto msg = socket_message{.buffers = buf};

    const char *alphabet = "abcdefghijklmnopqrstuvwxyz";
    auto *end = alphabet + 26;

    for (auto *it = alphabet; it != end; ++it)
    {
      auto msg_ = socket_message<sockaddr_in>{.buffers = std::span(it, 1)};
      auto len = sendmsg(sock_v4, msg_, 0);
      ASSERT_EQ(len, 1);
      len = sendmsg(sock_v6, msg_, 0);
      ASSERT_EQ(len, 1);

      n = ctx->poller.wait_for(50);
      ASSERT_GT(n, 0);

      len = recvmsg(sock_v4, msg, 0);
      ASSERT_EQ(len, 1);
      EXPECT_EQ(buf[0], *it);

      len = recvmsg(sock_v6, msg, 0);
      ASSERT_EQ(len, 1);
      EXPECT_EQ(buf[0], *it);
    }
  }

  ctx->signal(ctx->terminate);
  auto n = 0UL;
  while (ctx->poller.wait_for(50))
  {
    ASSERT_LE(n++, 2);
  }
  ASSERT_GT(n, 0);
}

TEST_F(AsyncTcpServiceTest, InitializeError)
{
  using namespace io::socket;
  using namespace stdexec;

  service_v4->initialized = true;
  auto error = service_v4->start(*ctx);
  EXPECT_EQ(error, std::errc::invalid_argument);

  auto n = 0UL;
  ctx->signal(ctx->terminate);
  while (ctx->poller.wait_for(2000))
  {
    ASSERT_LE(n++, 2);
  }
  ASSERT_GT(n, 0);
}

TEST_F(AsyncTcpServiceTest, AsyncServerTest)
{
  using namespace io;
  using namespace io::socket;
  using enum async_context::signals;
  using enum async_context::context_states;

  server_v4->start(addr_v4);
  server_v6->start(addr_v6);
  ASSERT_EQ(server_v4->state, STARTED);
  ASSERT_EQ(server_v6->state, STARTED);

  {
    auto sock_v4 = socket_handle(AF_INET, SOCK_STREAM, 0);
    auto sock_v6 = socket_handle(AF_INET6, SOCK_STREAM, 0);

    ASSERT_EQ(connect(sock_v4, addr_v4), 0);
    ASSERT_EQ(connect(sock_v6, addr_v6), 0);

    auto buf = std::array<char, 1>{'x'};
    auto msg = socket_message{.buffers = buf};

    const char *alphabet = "abcdefghijklmnopqrstuvwxyz";
    auto *end = alphabet + 26;

    for (auto *it = alphabet; it != end; ++it)
    {
      auto msg_ = socket_message<sockaddr_in>{.buffers = std::span(it, 1)};
      auto len = sendmsg(sock_v4, msg_, 0);
      ASSERT_EQ(len, 1);
      len = sendmsg(sock_v6, msg_, 0);
      ASSERT_EQ(len, 1);

      len = recvmsg(sock_v4, msg, 0);
      ASSERT_EQ(len, 1);
      EXPECT_EQ(buf[0], *it);

      len = recvmsg(sock_v6, msg, 0);
      ASSERT_EQ(len, 1);
      EXPECT_EQ(buf[0], *it);
    }
  }
}

TEST_F(AsyncTcpServiceTest, ServerDrainTest)
{
  using namespace io;
  using namespace io::socket;
  using namespace std::chrono;
  using namespace net::timers;
  using enum async_context::signals;
  using enum async_context::context_states;

  server_v4->start(addr_v4);
  ASSERT_EQ(server_v4->state, STARTED);

  auto sock = std::make_shared<socket_handle>(AF_INET, SOCK_STREAM, 0);
  ASSERT_EQ(connect(*sock, addr_v4), 0);

  test_counter = 0;
  server_v4->timers.add(milliseconds(3500), [&](auto) { sock.reset(); });
  // The server must process the timer_add event before
  // it receives a terminate signal.
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  server_v4->signal(terminate);
  server_v4->state.wait(STARTED);
  EXPECT_GE(test_counter, 2);
}
// NOLINTEND
