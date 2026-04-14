#include <exe/runtime/sandbox.hpp>
#include <exe/runtime/submit_task.hpp>
#include <exe/runtime/set_timer.hpp>

#include <wheels/test/framework.hpp>

using namespace exe;  // NOLINT
using namespace std::chrono_literals;  // NOLINT

TEST_SUITE(SandboxTimers) {
  SIMPLE_TEST(SetTimer) {
    runtime::Sandbox sandbox;

    bool done = false;

    runtime::SetTimer(sandbox, 1s, [&] {
      done = true;
    });

    ASSERT_FALSE(sandbox.RunNextTask());

    ASSERT_FALSE(done);

    ASSERT_EQ(sandbox.AdvanceClockBy(1s), 1);
    ASSERT_FALSE(done);

    ASSERT_TRUE(sandbox.RunNextTask());
    ASSERT_TRUE(sandbox.IsEmpty());

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(SetTimer2) {
    runtime::Sandbox sandbox;

    bool done = false;

    runtime::SetTimer(sandbox, 2s, [&] {
      done = true;
    });

    // Clock: 0
    ASSERT_FALSE(sandbox.RunNextTask());

    ASSERT_FALSE(done);

    // Clock: 1
    ASSERT_EQ(sandbox.AdvanceClockBy(1s), 0);

    ASSERT_EQ(sandbox.RunNextTask(), 0);
    ASSERT_FALSE(done);

    // Clock: 3
    ASSERT_EQ(sandbox.AdvanceClockBy(2s), 1);
    ASSERT_FALSE(done);

    ASSERT_TRUE(sandbox.RunNextTask());
    ASSERT_TRUE(sandbox.IsEmpty());

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(SameDeadline) {
    runtime::Sandbox sandbox;

    size_t ready = 0;

    static const size_t kTimers = 4;

    for (size_t i = 0; i < kTimers; ++i) {
      runtime::SetTimer(sandbox, 5s, [&] {
        ++ready;
      });
    }

    ASSERT_EQ(ready, 0);

    ASSERT_EQ(sandbox.AdvanceClockBy(7s), kTimers);
    ASSERT_EQ(sandbox.RunTasks(), kTimers);

    ASSERT_EQ(ready, kTimers);
  }

  SIMPLE_TEST(SameDeadline2) {
    runtime::Sandbox sandbox;

    size_t ready = 0;

    static const size_t kTimers = 4;

    for (size_t i = 0; i < kTimers; ++i) {
      runtime::SetTimer(sandbox, 3s, [&] {
        ++ready;
      });
    }

    ASSERT_EQ(ready, 0);

    ASSERT_EQ(sandbox.AdvanceClockToNextDeadline(), kTimers);
    ASSERT_EQ(sandbox.RunTasks(), kTimers);

    ASSERT_EQ(ready, kTimers);
  }

  SIMPLE_TEST(AdvanceClockToNextDeadline) {
    runtime::Sandbox sandbox;

    size_t c = 0;

    static const size_t kTimers = 4;

    for (size_t t = 1; t <= kTimers; ++t) {
      runtime::SetTimer(sandbox, 1s * t, [&, t] {
        c = t;
      });
    }

    ASSERT_EQ(c, 0);

    for (size_t t = 1; t <= kTimers; ++t) {
      sandbox.AdvanceClockToNextDeadline();
      sandbox.RunTasks();

      ASSERT_EQ(c, t);
    }

    ASSERT_TRUE(sandbox.IsEmpty());
  }

  SIMPLE_TEST(ManyTimers) {
    runtime::Sandbox sandbox;

    size_t ready = 0;

    static const size_t kTimers = 4;

    for (size_t t = 1; t <= kTimers; ++t) {
      runtime::SetTimer(sandbox, 1s * t, [&] {
        ++ready;
      });
    }

    ASSERT_EQ(ready, 0);

    ASSERT_EQ(sandbox.AdvanceClockBy(10s), kTimers);
    ASSERT_EQ(sandbox.RunTasks(), kTimers);

    ASSERT_EQ(ready, kTimers);
  }

  SIMPLE_TEST(PriorityQueue) {
    runtime::Sandbox sandbox;

    int c = 0;

    for (int t = 3; t >= 1; --t) {
      runtime::SetTimer(sandbox, 1s * t, [&, t] {
        c = t;
      });
    }

    ASSERT_EQ(c, 0);

    for (int t = 1; t <= 3; ++t) {
      sandbox.AdvanceClockBy(1s);  // Tick

      ASSERT_EQ(sandbox.RunTasks(), 1);
      ASSERT_EQ(c, t);
    }
  }

  SIMPLE_TEST(Ticks) {
    runtime::Sandbox sandbox;

    for (size_t k = 1; k < 10; ++k) {
      bool alarm = false;

      runtime::SetTimer(sandbox, 1s * k, [&] {
        alarm = true;
      });

      size_t c = 0;

      while (!alarm) {
        c += 1;
        sandbox.AdvanceClockBy(1s);
        sandbox.RunTasks();
      }

      ASSERT_EQ(c, k);
      ASSERT_TRUE(sandbox.IsEmpty());
    }
  }

  SIMPLE_TEST(LongTimer) {
    runtime::Sandbox sandbox;

    int c = 0;

    runtime::SetTimer(sandbox, 10s, [&] {
      c = 10;
    });

    for (int k = 1; k < 10; ++k) {
      runtime::SetTimer(sandbox, 1s, [&c, k] {
        c = k;
      });

      ASSERT_EQ(sandbox.AdvanceClockBy(1s), 1);
      sandbox.RunTasks();
      ASSERT_EQ(c, k);
    }

    ASSERT_FALSE(sandbox.IsEmpty());
    sandbox.AdvanceClockToNextDeadline();
    sandbox.RunTasks();

    ASSERT_EQ(c, 10);
  }
}

RUN_ALL_TESTS()
