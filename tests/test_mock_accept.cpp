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

static int error = 0;
int accept(int __fd, struct sockaddr *addr, socklen_t *len)
{
  errno = static_cast<int>(std::errc::bad_file_descriptor);
  error = errno;
  return -1;
}

TEST_F(AsyncTcpServiceTest, AcceptError)
{
  using namespace io::socket;
  service_v4->start(*ctx);
  service_v6->start(*ctx);
  ASSERT_EQ(error, static_cast<int>(std::errc::bad_file_descriptor));

  ctx->signal(ctx->terminate);
  auto n = 0UL;
  while (ctx->poller.wait_for(50))
  {
    ASSERT_LE(n++, 3);
  }
  EXPECT_GT(n, 0);
}
// NOLINTEND
