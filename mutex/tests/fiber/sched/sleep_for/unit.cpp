#include <exe/runtime/sandbox.hpp>
#include <exe/runtime/multi_thread.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>
#include <exe/fiber/sched/sleep_for.hpp>

#include <exe/thread/wait_group.hpp>

#include <wheels/test/framework.hpp>

#include <vector>

using namespace exe;  // NOLINT

TEST_SUITE(SandboxedFibers) {
  SIMPLE_TEST(SleepFor) {
    runtime::Sandbox sandbox;

    bool done = false;

    fiber::Go(sandbox, [&] {
      fiber::SleepFor(1s);
      done = true;
    });

    {
      size_t count = sandbox.RunTasks();
      ASSERT_EQ(count, 1);
    }

    ASSERT_FALSE(done);

    {
      size_t count = sandbox.AdvanceClockBy(2s);
      ASSERT_EQ(count, 1);
    }

    {
      size_t count = sandbox.RunTasks();
      ASSERT_EQ(count, 1);
      ASSERT_TRUE(done);
    }
  }

  SIMPLE_TEST(SleepFor2) {
    runtime::Sandbox sandbox;

    bool done = false;

    fiber::Go(sandbox, [&] {
      fiber::SleepFor(2s);
      done = true;
    });

    {
      size_t count = sandbox.RunTasks();
      ASSERT_EQ(count, 1);
    }

    ASSERT_FALSE(done);

    sandbox.AdvanceClockBy(1s);

    {
      size_t count = sandbox.RunTasks();
      ASSERT_EQ(count, 0);
    }

    ASSERT_FALSE(done);

    sandbox.AdvanceClockBy(2s);

    {
      size_t count = sandbox.RunTasks();
      ASSERT_EQ(count, 1);
    }

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(SleepFor3) {
    runtime::Sandbox sandbox;

    bool done = false;

    fiber::Go(sandbox, [&done] {
      bool stop = false;

      fiber::Go([&stop] {
        fiber::SleepFor(3s);
        stop = true;
      });

      while (!stop) {
        fiber::Yield();
      }

      done = true;
    });

    {
      {
        size_t count = sandbox.RunAtMostTasks(7);
        ASSERT_EQ(count, 7);
      }

      ASSERT_FALSE(done);

      sandbox.AdvanceClockBy(5s);
      sandbox.RunTasks();

      ASSERT_TRUE(done);
    }
  }

  SIMPLE_TEST(MultiSleepFor) {
    runtime::Sandbox sandbox;

    fiber::Go(sandbox, [] {
      for (size_t i = 0; i < 3; ++i) {
        fiber::SleepFor(1s);
      }
    });

    ASSERT_EQ(sandbox.RunTasks(), 1);

    for (size_t i = 0; i < 3; ++i) {
      sandbox.AdvanceClockToNextDeadline();
      ASSERT_EQ(sandbox.RunTasks(), 1);
    }

    ASSERT_TRUE(sandbox.IsEmpty());
  }

  SIMPLE_TEST(SleepSort) {
    runtime::Sandbox sandbox;

    std::vector<int> in = {3, 5, 2, 1, 4, 6, 7};
    std::vector<int> out;

    for (int v : in) {
      fiber::Go(sandbox, [&out, v] {
        fiber::SleepFor(1s * v);
        out.push_back(v);
      });
    }

    do {
      sandbox.RunTasks();
      sandbox.AdvanceClockBy(1s);
    } while (!sandbox.IsEmpty());

    for (int i = 0; i < 7; ++i) {
      ASSERT_EQ(i+1, out[i]);
    }
  }
}

TEST_SUITE(MultiThreadedFibers) {
  SIMPLE_TEST(Sleepers) {
    runtime::MultiThread mt{2};
    mt.WithTimers()
        .Start();

    thread::WaitGroup wg;

    for (size_t i = 0; i < 128; ++i) {
      wg.Add(1);
      fiber::Go(mt, [&wg] {
        fiber::SleepFor(1s);
        wg.Done();
      });
    }

    wg.Wait();

    mt.Stop();
  }
}

RUN_ALL_TESTS()
