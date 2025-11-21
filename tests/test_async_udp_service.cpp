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
#include "test_udp_fixture.hpp"

TEST_F(AsyncUDPServiceTest, StartTest)
{
  service_v4->start(*ctx);
  service_v6->start(*ctx);
  ctx->signal(ctx->terminate);

  auto n = 0UL;
  while (ctx->poller.wait_for(100))
  {
    ASSERT_LE(n++, 3);
  }
  EXPECT_GT(n, 0);
}

TEST_F(AsyncUDPServiceTest, EchoTest)
{
  service_v4->start(*ctx);
  service_v6->start(*ctx);

  {
    using namespace io;
    using namespace io::socket;

    auto sock_v4 = socket_handle(AF_INET, SOCK_DGRAM, 0);
    auto sock_v6 = socket_handle(AF_INET6, SOCK_DGRAM, 0);

    auto buf = std::array<char, 1>{'x'};
    auto msg = socket_message{.buffers = buf};

    const char *alphabet = "abcdefghijklmnopqrstuvwxyz";
    auto *end = alphabet + 26;

    for (auto *it = alphabet; it != end; ++it)
    {
      auto len = sendmsg(sock_v4,
                         socket_message<sockaddr_in>{
                             .address = {addr_v4}, .buffers = std::span(it, 1)},
                         0);
      ASSERT_EQ(len, 1);
      len = sendmsg(sock_v6,
                    socket_message<sockaddr_in6>{.address = {addr_v6},
                                                 .buffers = std::span(it, 1)},
                    0);
      ASSERT_EQ(len, 1);

      auto n = ctx->poller.wait_for(50);
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
  while (ctx->poller.wait_for(100))
  {
    ASSERT_LE(n++, 2);
  }
  ASSERT_GT(n, 0);
}

TEST_F(AsyncUDPServiceTest, InitializeError)
{
  using namespace io::socket;
  service_v4->initialized = true;
  auto error = service_v4->start(*ctx);

  EXPECT_EQ(error, std::errc::invalid_argument);
}

TEST_F(AsyncUDPServiceTest, AsyncServerTest)
{
  using namespace io;
  using namespace io::socket;
  using enum async_context::signals;
  using enum async_context::context_states;

  server_v4->start(addr_v4);
  server_v6->start(addr_v6);
  server_v6->state.wait(PENDING);
  server_v4->state.wait(PENDING);
  ASSERT_EQ(server_v4->state, STARTED);
  ASSERT_EQ(server_v6->state, STARTED);

  {
    auto sock_v4 = socket_handle(AF_INET, SOCK_DGRAM, 0);
    auto sock_v6 = socket_handle(AF_INET6, SOCK_DGRAM, 0);

    auto buf = std::array<char, 1>{'x'};
    auto msg = socket_message{.buffers = buf};

    const char *alphabet = "abcdefghijklmnopqrstuvwxyz";
    auto *end = alphabet + 26;

    for (auto *it = alphabet; it != end; ++it)
    {
      auto len = sendmsg(sock_v4,
                         socket_message<sockaddr_in>{
                             .address = {addr_v4}, .buffers = std::span(it, 1)},
                         0);
      ASSERT_EQ(len, 1);
      len = sendmsg(sock_v6,
                    socket_message<sockaddr_in6>{.address = {addr_v6},
                                                 .buffers = std::span(it, 1)},
                    0);
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
// NOLINTEND
