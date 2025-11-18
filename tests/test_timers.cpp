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
#include "net/timers/interrupt.hpp"
#include "net/timers/timers.hpp"

#include <gtest/gtest.h>

using namespace net::timers;

using interrupt_source = socketpair_interrupt_source_t;
using timers_type = timers<interrupt_source>;

TEST(TimersTests, MoveConstructor)
{
  auto timers0 = timers_type();
  auto timers1 = timers_type(std::move(timers0));
}

TEST(TimersTests, MoveAssignment)
{
  auto timers0 = timers_type();
  auto timers1 = timers_type();
  timers1 = std::move(timers0);
}

TEST(TimersTests, Swap)
{
  auto timers0 = timers_type();
  auto timers1 = timers_type();

  swap(timers0, timers1);
  swap(timers0, timers0);
  swap(timers1, timers1);
}

TEST(TimersTests, EventRefEquality)
{
  using event_ref = detail::event_ref;
  auto now = clock::now();
  auto ref0 = event_ref{.expires_at = now};
  auto ref1 = event_ref{.expires_at = now};
  EXPECT_EQ(ref0, ref1);
}

TEST(TimersTests, TimerAdd)
{
  auto timers = timers_type();
  auto timer = timers.add(100, [](timer_id) {});
  ASSERT_EQ(timer, 0);
}

TEST(TimersTests, ReuseTimerID)
{
  auto timers = timers_type();

  timers.remove(INVALID_TIMER); // Make sure this doesn't break.

  auto tmp = timers.remove(10);
  ASSERT_EQ(tmp, 10); // Invalid timers immediately return.

  auto timer0 = timers.add(100, [](timer_id) {});
  ASSERT_EQ(timer0, 0);
  tmp = timers.remove(timer0);
  ASSERT_EQ(tmp, INVALID_TIMER);

  timers.resolve();
  auto timer1 = timers.add(100, [](timer_id) {});
  EXPECT_EQ(timer0, timer1);
}

TEST(TimersTests, PeriodicTimer)
{
  using namespace std::chrono;

  auto timers = timers_type();
  auto timer0 = timers.add(100, [](timer_id) {}, 100);
  ASSERT_EQ(timer0, 0);
  std::this_thread::sleep_for(milliseconds(1));
  auto next = timers.resolve();
  EXPECT_NE(next.count(), -1);
}
// NOLINTEND
