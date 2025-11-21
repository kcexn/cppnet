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
#include "test_udp_fixture.hpp"

static int error = 0;
int setsockopt(int __fd, int level, int optname, const void *optval,
               socklen_t optlen)
{
  errno = static_cast<int>(std::errc::interrupted);
  error = errno;
  return -1;
}

TEST_F(AsyncTcpServiceTest, SetSockOptError)
{
  using namespace io::socket;

  service_v4->start(*ctx);
  EXPECT_EQ(error, static_cast<int>(std::errc::interrupted));

  auto n = 0UL;
  ctx->signal(ctx->terminate);
  while (ctx->poller.wait_for(2000))
  {
    ASSERT_LE(n, 3);
  }
  ASSERT_EQ(n, 0);
}

TEST_F(AsyncTcpServiceTest, StartFailed)
{
  EXPECT_THROW(server_v4->start(addr_v4), std::system_error);
}

TEST_F(AsyncUDPServiceTest, SetSockOptError)
{
  service_v4->start(*ctx);
  EXPECT_EQ(error, static_cast<int>(std::errc::interrupted));
}
// NOLINTEND
